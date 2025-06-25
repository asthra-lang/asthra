/**
 * Asthra Programming Language v1.2 Concurrency Tests - C Integration
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for C function integration and bidirectional communication
 * between Asthra and C threads.
 */

#include "test_spawn_common.h"

// =============================================================================
// C FUNCTION INTEGRATION TESTS
// =============================================================================

ConcurrencyTestResult test_concurrency_spawn_with_c_functions(AsthraV12TestContext *ctx) {
    // Test spawning tasks that call C functions

    char input_string[] = "Hello from C";
    TestTask *task = test_spawn(c_function_for_asthra, input_string);

    if (!CONCURRENCY_TEST_ASSERT(ctx, task != NULL, "Failed to spawn task with C function", "")) {
        return CONCURRENCY_TEST_FAIL;
    }

    test_task_join(task);

    int status = atomic_load(&task->status);
    if (!CONCURRENCY_TEST_ASSERT(ctx, status == TASK_STATUS_COMPLETED,
                                 "C function task should have completed", "")) {
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    char *result = (char *)task->result;
    if (!CONCURRENCY_TEST_ASSERT(ctx, result != NULL, "C function result should not be NULL", "")) {
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    if (!CONCURRENCY_TEST_ASSERT(ctx, strstr(result, "Processed: Hello from C") != NULL,
                                 "C function result incorrect: %s", result)) {
        free(result);
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    free(result);
    test_task_destroy(task);
    return CONCURRENCY_TEST_PASS;
}

ConcurrencyTestResult test_concurrency_c_threads_calling_asthra(AsthraV12TestContext *ctx) {
    // Test C threads calling Asthra functions

    int input_value = 7;
    TestTask *task = test_spawn(c_thread_calling_asthra, &input_value);

    if (!CONCURRENCY_TEST_ASSERT(ctx, task != NULL, "Failed to spawn C thread calling Asthra",
                                 "")) {
        return CONCURRENCY_TEST_FAIL;
    }

    test_task_join(task);

    int status = atomic_load(&task->status);
    if (!CONCURRENCY_TEST_ASSERT(ctx, status == TASK_STATUS_COMPLETED,
                                 "C thread calling Asthra should have completed", "")) {
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    int *result = (int *)task->result;
    if (!CONCURRENCY_TEST_ASSERT(ctx, result != NULL, "Asthra function result should not be NULL",
                                 "")) {
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    if (!CONCURRENCY_TEST_ASSERT(ctx, *result == 49, "Asthra function result should be 49, got %d",
                                 *result)) {
        free(result);
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    // Test error case
    int negative_input = -5;
    TestTask *error_task = test_spawn(c_thread_calling_asthra, &negative_input);

    if (!CONCURRENCY_TEST_ASSERT(ctx, error_task != NULL, "Failed to spawn error test task", "")) {
        free(result);
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    test_task_join(error_task);

    // Should complete but with NULL result (error case)
    if (!CONCURRENCY_TEST_ASSERT(ctx, error_task->result == NULL,
                                 "Error case should return NULL result", "")) {
        free(result);
        test_task_destroy(task);
        test_task_destroy(error_task);
        return CONCURRENCY_TEST_FAIL;
    }

    free(result);
    test_task_destroy(task);
    test_task_destroy(error_task);
    return CONCURRENCY_TEST_PASS;
}
