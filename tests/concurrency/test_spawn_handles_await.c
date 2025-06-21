/**
 * Asthra Programming Language v1.2 Concurrency Tests - Handle Operations and Await
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for spawn_with_handle operations and await expressions including
 * timeout handling and task coordination.
 */

#include "test_spawn_common.h"

// =============================================================================
// HANDLE OPERATIONS AND AWAIT EXPRESSION TESTS
// =============================================================================

// Test spawn_with_handle operations
AsthraTestResult test_concurrency_spawn_with_handle_operations(AsthraV12TestContext *ctx) {
    const int num_tasks = 4;
    TestTask *tasks[num_tasks];
    HandleTaskData inputs[num_tasks];
    
    // Create multiple tasks with handles
    for (int i = 0; i < num_tasks; i++) {
        inputs[i].task_id = i + 1;
        inputs[i].data = 10 * (i + 1);
        
        tasks[i] = test_spawn(handle_task_function, &inputs[i]);
        if (!ASTHRA_TEST_ASSERT(ctx, tasks[i] != NULL,
                               "Failed to spawn task %d with handle", i)) {
            // Cleanup any created tasks
            cleanup_tasks(tasks, i);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Wait for all tasks and verify results
    for (int i = 0; i < num_tasks; i++) {
        test_task_join(tasks[i]);
        
        HandleTaskData *result = (HandleTaskData*)tasks[i]->result;
        if (!ASTHRA_TEST_ASSERT(ctx, result != NULL,
                               "Task %d result should not be NULL", i)) {
            cleanup_tasks(tasks, num_tasks);
            return ASTHRA_TEST_FAIL;
        }
        
        int expected = inputs[i].data * inputs[i].task_id;
        if (!ASTHRA_TEST_ASSERT(ctx, result->data == expected,
                               "Task %d result should be %d, got %d", 
                               i, expected, result->data)) {
            cleanup_tasks(tasks, num_tasks);
            return ASTHRA_TEST_FAIL;
        }
        
        free(result);
        test_task_destroy(tasks[i]);
    }
    
    return ASTHRA_TEST_PASS;
}

// Test await expressions with timeouts
AsthraTestResult test_concurrency_await_expressions(AsthraV12TestContext *ctx) {
    // Test various await patterns
    int delays[] = {10, 20, 5, 30}; // milliseconds
    const int num_tasks = 4;
    TestTask *tasks[num_tasks];
    
    // Spawn tasks with different delays
    for (int i = 0; i < num_tasks; i++) {
        tasks[i] = test_spawn(timeout_task_function, &delays[i]);
        if (!ASTHRA_TEST_ASSERT(ctx, tasks[i] != NULL,
                               "Failed to spawn timeout task %d", i)) {
            cleanup_tasks(tasks, i);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Await tasks in different order than spawned
    int await_order[] = {2, 0, 3, 1}; // Await shortest delay first
    
    for (int i = 0; i < num_tasks; i++) {
        int task_idx = await_order[i];
        test_task_join(tasks[task_idx]);
        
        int *result = (int*)tasks[task_idx]->result;
        if (!ASTHRA_TEST_ASSERT(ctx, result != NULL && *result == delays[task_idx],
                               "Await task %d should return %d, got %d",
                               task_idx, delays[task_idx], result ? *result : -1)) {
            cleanup_tasks(tasks, num_tasks);
            return ASTHRA_TEST_FAIL;
        }
        
        free(result);
    }
    
    // Cleanup
    for (int i = 0; i < num_tasks; i++) {
        test_task_destroy(tasks[i]);
    }
    
    return ASTHRA_TEST_PASS;
} 
