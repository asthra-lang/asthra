/**
 * Asthra Programming Language v1.2 Concurrency Tests - Error Handling
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for error handling in concurrent contexts, including Result type 
 * propagation and stress testing of error scenarios.
 */

#include "concurrency_test_utils.h"

// =============================================================================
// ERROR HANDLING IN CONCURRENT CONTEXT
// =============================================================================

static void* result_propagation_function(void *arg) {
    int input = *(int*)arg;
    
    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) return NULL;
    
    if (input < 0) {
        *result = concurrent_result_err("Negative input not allowed");
    } else if (input > 1000) {
        *result = concurrent_result_err("Input too large");
    } else {
        int *value = malloc(sizeof(int));
        if (!value) {
            *result = concurrent_result_err("Memory allocation failed");
        } else {
            *value = input * 3;
            *result = concurrent_result_ok(value);
        }
    }
    
    return result;
}

AsthraTestResult test_concurrency_result_propagation(AsthraV12TestContext *ctx) {
    // Test Result<T,E> propagation in concurrent contexts
    
    // Test successful case
    int success_input = 10;
    TestTask *success_task = test_spawn(result_propagation_function, &success_input);
    
    if (!ASTHRA_TEST_ASSERT(ctx, success_task != NULL,
                           "Success task creation should succeed", "")) {
        return ASTHRA_TEST_FAIL;
    }
    
    test_task_join(success_task);
    
    ConcurrentResult *success_result = (ConcurrentResult*)success_task->result;
    if (!ASTHRA_TEST_ASSERT(ctx, success_result != NULL,
                           "Success result should not be NULL", "")) {
        test_task_destroy(success_task);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(ctx, success_result->is_ok,
                           "Success result should be Ok", "")) {
        free(success_result);
        test_task_destroy(success_task);
        return ASTHRA_TEST_FAIL;
    }
    
    int *success_value = (int*)success_result->data.ok_value;
    if (!ASTHRA_TEST_ASSERT(ctx, *success_value == 30,
                           "Success value should be 30, got %d", *success_value)) {
        free(success_value);
        free(success_result);
        test_task_destroy(success_task);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test error case
    int error_input = -5;
    TestTask *error_task = test_spawn(result_propagation_function, &error_input);
    
    if (!ASTHRA_TEST_ASSERT(ctx, error_task != NULL,
                           "Error task creation should succeed", "")) {
        free(success_value);
        free(success_result);
        test_task_destroy(success_task);
        return ASTHRA_TEST_FAIL;
    }
    
    test_task_join(error_task);
    
    ConcurrentResult *error_result = (ConcurrentResult*)error_task->result;
    if (!ASTHRA_TEST_ASSERT(ctx, error_result != NULL,
                           "Error result should not be NULL", "")) {
        free(success_value);
        free(success_result);
        test_task_destroy(success_task);
        test_task_destroy(error_task);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(ctx, !error_result->is_ok,
                           "Error result should be Err", "")) {
        free(success_value);
        free(success_result);
        free(error_result);
        test_task_destroy(success_task);
        test_task_destroy(error_task);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(ctx, strcmp(error_result->data.error_message, "Negative input not allowed") == 0,
                           "Error message should match expected", "")) {
        free(success_value);
        free(success_result);
        free(error_result);
        test_task_destroy(success_task);
        test_task_destroy(error_task);
        return ASTHRA_TEST_FAIL;
    }
    
    free(success_value);
    free(success_result);
    free(error_result);
    test_task_destroy(success_task);
    test_task_destroy(error_task);
    return ASTHRA_TEST_PASS;
}

// Shared data for cross-task error testing
typedef struct {
    _Atomic(int) task_count;
    _Atomic(int) error_count;
    _Atomic(int) success_count;
    ConcurrentResult results[10];
} CrossTaskData;

static void* cross_task_function(void *arg) {
    CrossTaskData *data = (CrossTaskData*)arg;
    int task_id = atomic_fetch_add(&data->task_count, 1);
    
    // Simulate different outcomes based on task ID
    if (task_id % 3 == 0) {
        // Error case
        atomic_fetch_add(&data->error_count, 1);
        data->results[task_id] = concurrent_result_err("Simulated error");
    } else {
        // Success case
        atomic_fetch_add(&data->success_count, 1);
        int *value = malloc(sizeof(int));
        *value = task_id * 100;
        data->results[task_id] = concurrent_result_ok(value);
    }
    
    return &data->results[task_id];
}

AsthraTestResult test_concurrency_error_across_tasks(AsthraV12TestContext *ctx) {
    // Test error handling across multiple concurrent tasks
    
    CrossTaskData shared_data;
    atomic_store(&shared_data.task_count, 0);
    atomic_store(&shared_data.error_count, 0);
    atomic_store(&shared_data.success_count, 0);
    
    const int num_tasks = 9;
    TestTask *tasks[num_tasks];
    
    // Create multiple tasks
    for (int i = 0; i < num_tasks; i++) {
        tasks[i] = test_spawn(cross_task_function, &shared_data);
        
        if (!ASTHRA_TEST_ASSERT(ctx, tasks[i] != NULL,
                               "Task %d creation should succeed", i)) {
            // Cleanup created tasks
            for (int j = 0; j < i; j++) {
                test_task_join(tasks[j]);
                test_task_destroy(tasks[j]);
            }
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Wait for all tasks to complete
    for (int i = 0; i < num_tasks; i++) {
        test_task_join(tasks[i]);
    }
    
    // Verify results
    int final_task_count = atomic_load(&shared_data.task_count);
    int final_error_count = atomic_load(&shared_data.error_count);
    int final_success_count = atomic_load(&shared_data.success_count);
    
    if (!ASTHRA_TEST_ASSERT(ctx, final_task_count == num_tasks,
                           "All tasks should have executed: %d/%d", final_task_count, num_tasks)) {
        // Cleanup
        for (int i = 0; i < num_tasks; i++) {
            test_task_destroy(tasks[i]);
        }
        return ASTHRA_TEST_FAIL;
    }
    
    // Expected: tasks 0, 3, 6 should error (3 errors), others should succeed (6 successes)
    if (!ASTHRA_TEST_ASSERT(ctx, final_error_count == 3,
                           "Should have 3 errors, got %d", final_error_count)) {
        // Cleanup
        for (int i = 0; i < num_tasks; i++) {
            test_task_destroy(tasks[i]);
        }
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(ctx, final_success_count == 6,
                           "Should have 6 successes, got %d", final_success_count)) {
        // Cleanup
        for (int i = 0; i < num_tasks; i++) {
            test_task_destroy(tasks[i]);
        }
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify individual results
    for (int i = 0; i < num_tasks; i++) {
        if (i % 3 == 0) {
            // Should be error
            if (!ASTHRA_TEST_ASSERT(ctx, !shared_data.results[i].is_ok,
                                   "Task %d should have error result", i)) {
                // Cleanup
                for (int j = 0; j < num_tasks; j++) {
                    test_task_destroy(tasks[j]);
                }
                return ASTHRA_TEST_FAIL;
            }
        } else {
            // Should be success
            if (!ASTHRA_TEST_ASSERT(ctx, shared_data.results[i].is_ok,
                                   "Task %d should have success result", i)) {
                // Cleanup
                for (int j = 0; j < num_tasks; j++) {
                    test_task_destroy(tasks[j]);
                }
                return ASTHRA_TEST_FAIL;
            }
            
            // Cleanup allocated values
            free(shared_data.results[i].data.ok_value);
        }
    }
    
    // Cleanup tasks
    for (int i = 0; i < num_tasks; i++) {
        test_task_destroy(tasks[i]);
    }
    
    return ASTHRA_TEST_PASS;
}

static void* stress_test_function(void *arg) {
    int *counter = (int*)arg;
    
    // Simulate varying workloads
    int work_amount = (*counter) % 10;
    for (int i = 0; i < work_amount; i++) {
        usleep(1000); // 1ms per unit of work
    }
    
    // Randomly succeed or fail
    if ((*counter) % 7 == 0) {
        return NULL; // Simulate error
    }
    
    int *result = malloc(sizeof(int));
    *result = *counter;
    return result;
}

AsthraTestResult test_concurrency_error_handling_stress(AsthraV12TestContext *ctx) {
    // Stress test error handling with many concurrent tasks
    
            asthra_benchmark_start(ctx);
    
    const int stress_task_count = 100;
    TestTask *stress_tasks[stress_task_count];
    int task_inputs[stress_task_count];
    
    uint64_t start_time = asthra_test_get_time_ns();
    
    // Create many tasks
    for (int i = 0; i < stress_task_count; i++) {
        task_inputs[i] = i;
        stress_tasks[i] = test_spawn(stress_test_function, &task_inputs[i]);
        
        if (!ASTHRA_TEST_ASSERT(ctx, stress_tasks[i] != NULL,
                               "Stress task %d creation should succeed", i)) {
            // Cleanup created tasks
            for (int j = 0; j < i; j++) {
                test_task_join(stress_tasks[j]);
                test_task_destroy(stress_tasks[j]);
            }
            return ASTHRA_TEST_FAIL;
        }
        
        asthra_benchmark_iteration(ctx);
    }
    
    // Wait for all tasks to complete
    int success_count = 0;
    int error_count = 0;
    
    for (int i = 0; i < stress_task_count; i++) {
        test_task_join(stress_tasks[i]);
        
        if (stress_tasks[i]->result != NULL) {
            success_count++;
            free(stress_tasks[i]->result);
        } else {
            error_count++;
        }
        
        test_task_destroy(stress_tasks[i]);
    }
    
    uint64_t end_time = asthra_test_get_time_ns();
    asthra_benchmark_end(ctx);
    
    // Verify stress test results
    if (!ASTHRA_TEST_ASSERT(ctx, success_count + error_count == stress_task_count,
                           "All tasks should be accounted for: %d + %d = %d", 
                           success_count, error_count, stress_task_count)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Should have some errors (every 7th task)
    int expected_errors = stress_task_count / 7 + (stress_task_count % 7 > 0 ? 1 : 0);
    if (!ASTHRA_TEST_ASSERT(ctx, error_count >= expected_errors - 2 && error_count <= expected_errors + 2,
                           "Error count should be approximately %d, got %d", expected_errors, error_count)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Performance should be reasonable
    uint64_t total_duration = end_time - start_time;
    double duration_seconds = (double)total_duration / 1e9;
    
    if (!ASTHRA_TEST_ASSERT(ctx, duration_seconds < 5.0,
                           "Stress test should complete in reasonable time: %.2f seconds", duration_seconds)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Record performance metrics
    ctx->benchmark.throughput_ops_per_sec = (double)stress_task_count / duration_seconds;
    
    return ASTHRA_TEST_PASS;
} 
