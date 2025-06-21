/**
 * Asthra Programming Language v1.2 Concurrency Tests - Error Handling
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for error handling, edge cases, and exceptional conditions
 * in spawn functionality.
 */

#include "test_spawn_common.h"

// =============================================================================
// ERROR HANDLING AND EDGE CASE TESTS
// =============================================================================

// Test error cases
AsthraTestResult test_concurrency_error_cases(AsthraV12TestContext *ctx) {
    // Test NULL return
    int dummy = 0;
    TestTask *null_task = test_spawn(error_task_null_return, &dummy);
    
    if (!ASTHRA_TEST_ASSERT(ctx, null_task != NULL,
                           "Failed to spawn null return task", "")) {
        return ASTHRA_TEST_FAIL;
    }
    
    test_task_join(null_task);
    
    if (!ASTHRA_TEST_ASSERT(ctx, null_task->result == NULL,
                           "Null return task should have NULL result", "")) {
        test_task_destroy(null_task);
        return ASTHRA_TEST_FAIL;
    }
    
    test_task_destroy(null_task);
    
    // Test error condition
    int negative_input = -5;
    TestTask *error_task = test_spawn(error_task_exception, &negative_input);
    
    if (!ASTHRA_TEST_ASSERT(ctx, error_task != NULL,
                           "Failed to spawn error condition task", "")) {
        return ASTHRA_TEST_FAIL;
    }
    
    test_task_join(error_task);
    
    if (!ASTHRA_TEST_ASSERT(ctx, error_task->result == NULL,
                           "Error condition task should have NULL result", "")) {
        test_task_destroy(error_task);
        return ASTHRA_TEST_FAIL;
    }
    
    test_task_destroy(error_task);
    
    // Test positive case to ensure error detection is working
    int positive_input = 10;
    TestTask *success_task = test_spawn(error_task_exception, &positive_input);
    
    if (!ASTHRA_TEST_ASSERT(ctx, success_task != NULL,
                           "Failed to spawn success case task", "")) {
        return ASTHRA_TEST_FAIL;
    }
    
    test_task_join(success_task);
    
    int *result = (int*)success_task->result;
    if (!ASTHRA_TEST_ASSERT(ctx, result != NULL && *result == 20,
                           "Success case should return 20, got %d", 
                           result ? *result : -1)) {
        if (result) free(result);
        test_task_destroy(success_task);
        return ASTHRA_TEST_FAIL;
    }
    
    free(result);
    test_task_destroy(success_task);
    
    return ASTHRA_TEST_PASS;
} 
