/**
 * Asthra Enhanced Concurrency Bridge Test Suite - Task Management Tests
 * Tests for task spawning, execution, and lifecycle management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_test_common.h"

// =============================================================================
// TASK MANAGEMENT TESTS
// =============================================================================

bool test_task_spawn_and_completion(void) {
    printf("[TEST DEBUG] Starting task spawn and completion test\n");
    fflush(stdout);
    
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");
    
    int test_value = 5;
    printf("[TEST DEBUG] Test value: %d\n", test_value);
    fflush(stdout);
    
    // Spawn a simple task
    AsthraConcurrencyTaskHandle *handle = Asthra_spawn_task(simple_task_function, &test_value, 
                                                           sizeof(int), NULL);
    printf("[TEST DEBUG] Task spawned, handle: %p\n", (void*)handle);
    fflush(stdout);
    ASSERT(handle != NULL, "Task spawn should succeed");
    
    // Check initial state
    bool initially_complete = Asthra_task_is_complete(handle);
    printf("[TEST DEBUG] Initially complete: %s\n", initially_complete ? "true" : "false");
    fflush(stdout);
    ASSERT(!initially_complete, "Task should not be complete initially");
    
    // Wait for completion
    printf("[TEST DEBUG] Waiting for task result...\n");
    fflush(stdout);
    AsthraResult task_result = Asthra_task_get_result(handle);
    printf("[TEST DEBUG] Got task result, tag: %d, is_ok: %s\n", 
           task_result.tag, asthra_result_is_ok(task_result) ? "true" : "false");
    fflush(stdout);
    ASSERT(asthra_result_is_ok(task_result), "Task should complete successfully");
    
    // Check completion state
    bool finally_complete = Asthra_task_is_complete(handle);
    printf("[TEST DEBUG] Finally complete: %s\n", finally_complete ? "true" : "false");
    fflush(stdout);
    ASSERT(finally_complete, "Task should be complete after waiting");
    
    // Verify result
    int *result_value = (int*)asthra_result_unwrap_ok(task_result);
    printf("[TEST DEBUG] Unwrapped result pointer: %p\n", (void*)result_value);
    fflush(stdout);
    ASSERT(result_value != NULL, "Task result should not be NULL");
    
    if (result_value) {
        printf("[TEST DEBUG] Result value: %d (expected: 6)\n", *result_value);
        fflush(stdout);
        ASSERT(*result_value == 6, "Task should have incremented the value");
    }
    
    // Cleanup
    Asthra_task_handle_free(handle);
    Asthra_concurrency_bridge_cleanup();
    
    printf("[TEST DEBUG] Test completed\n");
    fflush(stdout);
    return true;
}

bool test_task_failure_handling(void) {
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");
    
    // Spawn a failing task
    AsthraConcurrencyTaskHandle *handle = Asthra_spawn_task(failing_task_function, NULL, 0, NULL);
    ASSERT(handle != NULL, "Task spawn should succeed");
    
    // Wait for completion
    AsthraResult task_result = Asthra_task_get_result(handle);
    ASSERT(asthra_result_is_err(task_result), "Task should fail");
    
    // Verify error
    int *error_code = (int*)asthra_result_unwrap_err(task_result);
    ASSERT(error_code != NULL, "Error code should not be NULL");
    ASSERT(*error_code == 42, "Error code should match expected value");
    
    // Cleanup
    Asthra_task_handle_free(handle);
    Asthra_concurrency_bridge_cleanup();
    
    return true;
}

bool test_task_timeout(void) {
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");
    
    // Spawn a long-running task
    AsthraConcurrencyTaskHandle *handle = Asthra_spawn_task(long_running_task_function, NULL, 0, NULL);
    ASSERT(handle != NULL, "Task spawn should succeed");
    
    // Wait with short timeout
    AsthraResult timeout_result = Asthra_task_wait_timeout(handle, 100); // 100ms timeout
    ASSERT(asthra_result_is_err(timeout_result), "Task wait should timeout");
    
    // Task should still be running
    ASSERT(!Asthra_task_is_complete(handle), "Task should still be running after timeout");
    
    // Wait for actual completion (with longer timeout)
    AsthraResult completion_result = Asthra_task_wait_timeout(handle, 2000); // 2s timeout
    ASSERT(asthra_result_is_ok(completion_result), "Task should eventually complete");
    
    // Cleanup
    Asthra_task_handle_free(handle);
    Asthra_concurrency_bridge_cleanup();
    
    return true;
}

bool test_task_spawn_options(void) {
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");
    
    // Test with custom options
    AsthraConcurrencyTaskSpawnOptions options = {
        .stack_size = 1024 * 1024, // 1MB stack
        .priority = 0,
        .detached = false,
        .name = "test_task",
        .affinity_mask = NULL
    };
    
    int test_value = 10;
    AsthraConcurrencyTaskHandle *handle = Asthra_spawn_task(simple_task_function, &test_value, 
                                                           sizeof(int), &options);
    ASSERT(handle != NULL, "Task spawn with options should succeed");
    
    // Wait for completion
    AsthraResult task_result = Asthra_task_get_result(handle);
    ASSERT(asthra_result_is_ok(task_result), "Task should complete successfully");
    
    // Cleanup
    Asthra_task_handle_free(handle);
    Asthra_concurrency_bridge_cleanup();
    
    return true;
} 
