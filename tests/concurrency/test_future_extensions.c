/**
 * Asthra Programming Language v1.2 Concurrency Tests - Future Extensions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for foundation features that support future async/await extensions,
 * including await patterns, task handles, and future compatibility.
 */

#include "concurrency_test_utils.h"

// =============================================================================
// FUTURE EXTENSIONS FOUNDATION
// =============================================================================

// Forward declaration for simple_task_function (defined in test_spawn.c)
extern void* simple_task_function(void *arg);

// Future completing task function
typedef struct {
    TestFuture *future;
    int value;
} FutureTaskArg;

static void* future_completing_task(void *task_arg) {
    FutureTaskArg *farg = (FutureTaskArg*)task_arg;
    
    // Simulate some work
    usleep(10000); // 10ms
    
    int *result = malloc(sizeof(int));
    *result = farg->value * 2;
    
    test_future_complete(farg->future, result);
    return result;
}

AsthraTestResult test_concurrency_await_foundation(AsthraV12TestContext *ctx) {
    // Test foundation for future await functionality
    
    TestFuture *future = test_future_create();
    
    if (!ASTHRA_TEST_ASSERT(ctx, future != NULL,
                           "Future creation should succeed", "")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test initial state
    if (!ASTHRA_TEST_ASSERT(ctx, !atomic_load(&future->is_ready),
                           "Future should not be ready initially", "")) {
        test_future_destroy(future);
        return ASTHRA_TEST_FAIL;
    }
    
    // Create a task that will complete the future
    FutureTaskArg arg = {future, 123};
    
    TestTask *task = test_spawn(future_completing_task, &arg);
    
    if (!ASTHRA_TEST_ASSERT(ctx, task != NULL,
                           "Future completing task should be created", "")) {
        test_future_destroy(future);
        return ASTHRA_TEST_FAIL;
    }
    
    // Await the future (this should block until task completes)
    void *result = test_future_await(future);
    
    if (!ASTHRA_TEST_ASSERT(ctx, result != NULL,
                           "Future await should return result", "")) {
        test_task_join(task);
        test_task_destroy(task);
        test_future_destroy(future);
        return ASTHRA_TEST_FAIL;
    }
    
    int *int_result = (int*)result;
    if (!ASTHRA_TEST_ASSERT(ctx, *int_result == 246,
                           "Future result should be 246, got %d", *int_result)) {
        free(result);
        test_task_join(task);
        test_task_destroy(task);
        test_future_destroy(future);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify future is now ready
    if (!ASTHRA_TEST_ASSERT(ctx, atomic_load(&future->is_ready),
                           "Future should be ready after completion", "")) {
        free(result);
        test_task_join(task);
        test_task_destroy(task);
        test_future_destroy(future);
        return ASTHRA_TEST_FAIL;
    }
    
    free(result);
    test_task_join(task);
    test_task_destroy(task);
    test_future_destroy(future);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_concurrency_task_handles(AsthraV12TestContext *ctx) {
    // Test task handle management for future async/await
    
    int input_value = 55;
    TestTask *task = test_spawn(simple_task_function, &input_value);
    
    if (!ASTHRA_TEST_ASSERT(ctx, task != NULL,
                           "Task creation should succeed", "")) {
        return ASTHRA_TEST_FAIL;
    }
    
    TestTaskHandle *handle = test_task_handle_create(task);
    
    if (!ASTHRA_TEST_ASSERT(ctx, handle != NULL,
                           "Task handle creation should succeed", "")) {
        test_task_join(task);
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify handle properties
    if (!ASTHRA_TEST_ASSERT(ctx, handle->task == task,
                           "Handle should reference correct task", "")) {
        test_task_join(task);
        test_task_destroy(task);
        test_task_handle_destroy(handle);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(ctx, handle->handle_id > 0,
                           "Handle should have valid ID", "")) {
        test_task_join(task);
        test_task_destroy(task);
        test_task_handle_destroy(handle);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(ctx, handle->future != NULL,
                           "Handle should have associated future", "")) {
        test_task_join(task);
        test_task_destroy(task);
        test_task_handle_destroy(handle);
        return ASTHRA_TEST_FAIL;
    }
    
    // Wait for task completion and complete the future
    test_task_join(task);
    test_future_complete(handle->future, task->result);
    
    // Test awaiting through handle
    void *result = test_future_await(handle->future);
    
    if (!ASTHRA_TEST_ASSERT(ctx, result != NULL,
                           "Handle await should return result", "")) {
        test_task_destroy(task);
        test_task_handle_destroy(handle);
        return ASTHRA_TEST_FAIL;
    }
    
    int *int_result = (int*)result;
    if (!ASTHRA_TEST_ASSERT(ctx, *int_result == 110,
                           "Handle result should be 110, got %d", *int_result)) {
        free(result);
        test_task_destroy(task);
        test_task_handle_destroy(handle);
        return ASTHRA_TEST_FAIL;
    }
    
    free(result);
    test_task_destroy(task);
    test_task_handle_destroy(handle);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_concurrency_future_compatibility(AsthraV12TestContext *ctx) {
    // Test compatibility foundation for future async/await extensions
    
    // Test multiple futures
    const int num_futures = 5;
    TestFuture *futures[num_futures];
    TestTask *tasks[num_futures];
    int inputs[num_futures];
    
    // Create multiple futures and tasks
    for (int i = 0; i < num_futures; i++) {
        futures[i] = test_future_create();
        inputs[i] = i * 10;
        
        if (!ASTHRA_TEST_ASSERT(ctx, futures[i] != NULL,
                               "Future %d creation should succeed", i)) {
            // Cleanup
            for (int j = 0; j < i; j++) {
                test_future_destroy(futures[j]);
            }
            return ASTHRA_TEST_FAIL;
        }
        
        tasks[i] = test_spawn(simple_task_function, &inputs[i]);
        
        if (!ASTHRA_TEST_ASSERT(ctx, tasks[i] != NULL,
                               "Task %d creation should succeed", i)) {
            // Cleanup
            for (int j = 0; j <= i; j++) {
                test_future_destroy(futures[j]);
            }
            for (int j = 0; j < i; j++) {
                test_task_join(tasks[j]);
                test_task_destroy(tasks[j]);
            }
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Complete all futures as tasks finish
    for (int i = 0; i < num_futures; i++) {
        test_task_join(tasks[i]);
        test_future_complete(futures[i], tasks[i]->result);
    }
    
    // Await all futures
    for (int i = 0; i < num_futures; i++) {
        void *result = test_future_await(futures[i]);
        
        if (!ASTHRA_TEST_ASSERT(ctx, result != NULL,
                               "Future %d await should return result", i)) {
            // Cleanup
            for (int j = 0; j < num_futures; j++) {
                if (j != i && tasks[j]->result) free(tasks[j]->result);
                test_task_destroy(tasks[j]);
                test_future_destroy(futures[j]);
            }
            return ASTHRA_TEST_FAIL;
        }
        
        int *int_result = (int*)result;
        int expected = inputs[i] * 2;
        
        if (!ASTHRA_TEST_ASSERT(ctx, *int_result == expected,
                               "Future %d result should be %d, got %d", i, expected, *int_result)) {
            // Cleanup
            for (int j = 0; j < num_futures; j++) {
                if (tasks[j]->result) free(tasks[j]->result);
                test_task_destroy(tasks[j]);
                test_future_destroy(futures[j]);
            }
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Cleanup
    for (int i = 0; i < num_futures; i++) {
        if (tasks[i]->result) free(tasks[i]->result);
        test_task_destroy(tasks[i]);
        test_future_destroy(futures[i]);
    }
    
    return ASTHRA_TEST_PASS;
} 
