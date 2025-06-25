/**
 * Asthra Programming Language v1.2 Concurrency Tests - Basic Spawn Functionality
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for basic spawn functionality including spawn, deterministic behavior,
 * multiple tasks, arguments, and multiple statements.
 */

#include "test_spawn_common.h"

// =============================================================================
// BASIC SPAWN FUNCTIONALITY TESTS
// =============================================================================

ConcurrencyTestResult test_concurrency_spawn_basic(AsthraV12TestContext *ctx) {
    // Test basic spawn functionality - Phase 8 Enhanced

    int input_value = 42;
    TestTask *task = test_spawn(simple_task_function, &input_value);

    if (!CONCURRENCY_TEST_ASSERT(ctx, task != NULL, "Failed to spawn basic task", "")) {
        return CONCURRENCY_TEST_FAIL;
    }

    // Wait for task completion
    test_task_join(task);

    // Verify task completed successfully
    int status = atomic_load(&task->status);
    if (!CONCURRENCY_TEST_ASSERT(ctx, status == TASK_STATUS_COMPLETED,
                                 "Task should have completed, status: %d", status)) {
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    // Verify result
    int *result = (int *)task->result;
    if (!CONCURRENCY_TEST_ASSERT(ctx, result != NULL, "Task result should not be NULL", "")) {
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    if (!CONCURRENCY_TEST_ASSERT(ctx, *result == 84, "Task result should be 84, got %d", *result)) {
        free(result);
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    free(result);
    test_task_destroy(task);
    return CONCURRENCY_TEST_PASS;
}

// Enhanced test for deterministic behavior validation
ConcurrencyTestResult test_concurrency_spawn_deterministic(AsthraV12TestContext *ctx) {
    // Phase 8: Test that Tier 1 spawn is deterministic for same inputs

    const int test_input = 15;
    const int num_runs = 5;
    int first_result = -1;

    for (int run = 0; run < num_runs; run++) {
        TestTask *task = test_spawn(simple_task_function, (void *)&test_input);

        if (!CONCURRENCY_TEST_ASSERT(ctx, task != NULL,
                                     "Failed to spawn deterministic test task run %d", run)) {
            return CONCURRENCY_TEST_FAIL;
        }

        test_task_join(task);

        int *result = (int *)task->result;
        if (!CONCURRENCY_TEST_ASSERT(ctx, result != NULL,
                                     "Deterministic test result should not be NULL on run %d",
                                     run)) {
            test_task_destroy(task);
            return CONCURRENCY_TEST_FAIL;
        }

        if (run == 0) {
            first_result = *result;
        } else {
            if (!CONCURRENCY_TEST_ASSERT(ctx, *result == first_result,
                                         "Deterministic test: run %d result %d != first result %d",
                                         run, *result, first_result)) {
                free(result);
                test_task_destroy(task);
                return CONCURRENCY_TEST_FAIL;
            }
        }

        free(result);
        test_task_destroy(task);
    }

    return CONCURRENCY_TEST_PASS;
}

// Test multiple concurrent spawns (Tier 1 scalability)
ConcurrencyTestResult test_concurrency_spawn_multiple(AsthraV12TestContext *ctx) {
    // Phase 8: Test spawning multiple tasks concurrently

    const int num_tasks = 6;
    TestTask *tasks[num_tasks];
    int inputs[num_tasks];

    // Create multiple tasks
    for (int i = 0; i < num_tasks; i++) {
        inputs[i] = (i + 1) * 10;
        tasks[i] = test_spawn(simple_task_function, &inputs[i]);

        if (!CONCURRENCY_TEST_ASSERT(ctx, tasks[i] != NULL, "Failed to spawn task %d", i)) {
            // Cleanup previously spawned tasks
            cleanup_tasks(tasks, i);
            return CONCURRENCY_TEST_FAIL;
        }
    }

    // Wait for all tasks and verify results
    for (int i = 0; i < num_tasks; i++) {
        test_task_join(tasks[i]);

        int *result = (int *)tasks[i]->result;
        int expected = inputs[i] * 2;

        if (!CONCURRENCY_TEST_ASSERT(ctx, result != NULL && *result == expected,
                                     "Task %d result should be %d, got %d", i, expected,
                                     result ? *result : -1)) {
            // Cleanup all tasks
            cleanup_tasks(tasks, num_tasks);
            return CONCURRENCY_TEST_FAIL;
        }

        free(result);
        test_task_destroy(tasks[i]);
    }

    return CONCURRENCY_TEST_PASS;
}

ConcurrencyTestResult test_concurrency_spawn_with_arguments(AsthraV12TestContext *ctx) {
    struct TaskArgs args = {15, 7, '+'};

    TestTask *task = test_spawn(task_with_arguments, &args);

    if (!CONCURRENCY_TEST_ASSERT(ctx, task != NULL, "Failed to spawn task with arguments", "")) {
        return CONCURRENCY_TEST_FAIL;
    }

    test_task_join(task);

    int *result = (int *)task->result;
    if (!CONCURRENCY_TEST_ASSERT(ctx, result != NULL && *result == 22,
                                 "Task with arguments should compute 15+7=22, got %d",
                                 result ? *result : -1)) {
        if (result)
            free(result);
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    free(result);
    test_task_destroy(task);
    return CONCURRENCY_TEST_PASS;
}

ConcurrencyTestResult test_concurrency_spawn_multiple_statements(AsthraV12TestContext *ctx) {
    int input_value = 21;
    TestTask *task = test_spawn(multi_statement_task, &input_value);

    if (!CONCURRENCY_TEST_ASSERT(ctx, task != NULL, "Failed to spawn multi-statement task", "")) {
        return CONCURRENCY_TEST_FAIL;
    }

    test_task_join(task);

    int *result = (int *)task->result;
    if (!CONCURRENCY_TEST_ASSERT(ctx, result != NULL,
                                 "Multi-statement task result should not be NULL", "")) {
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    // Expected: (21*2+10)/3 = 52/3 = 17, plus computational work
    if (!CONCURRENCY_TEST_ASSERT(ctx, *result >= 17,
                                 "Multi-statement task result should be >= 17, got %d", *result)) {
        free(result);
        test_task_destroy(task);
        return CONCURRENCY_TEST_FAIL;
    }

    free(result);
    test_task_destroy(task);
    return CONCURRENCY_TEST_PASS;
}
