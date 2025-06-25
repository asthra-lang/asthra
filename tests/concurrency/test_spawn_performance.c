/**
 * Asthra Programming Language v1.2 Concurrency Tests - Performance and Behavior
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for performance, scalability, and deterministic behavior validation
 * including timing, resource usage, and consistency checks.
 */

#include "test_spawn_common.h"

// =============================================================================
// PERFORMANCE AND BEHAVIOR TESTS
// =============================================================================

// Test deterministic behavior
ConcurrencyTestResult test_concurrency_deterministic_behavior(AsthraV12TestContext *ctx) {
    const int test_input = 42;
    const int num_runs = 10;
    int expected_result = (test_input * test_input) + (test_input * 2) + 1; // 1849

    for (int run = 0; run < num_runs; run++) {
        TestTask *task = test_spawn(deterministic_task, (void *)&test_input);

        if (!CONCURRENCY_TEST_ASSERT(ctx, task != NULL, "Failed to spawn deterministic task run %d",
                                     run)) {
            return CONCURRENCY_TEST_FAIL;
        }

        test_task_join(task);

        int *result = (int *)task->result;
        if (!CONCURRENCY_TEST_ASSERT(ctx, result != NULL && *result == expected_result,
                                     "Deterministic task run %d should return %d, got %d", run,
                                     expected_result, result ? *result : -1)) {
            if (result)
                free(result);
            test_task_destroy(task);
            return CONCURRENCY_TEST_FAIL;
        }

        free(result);
        test_task_destroy(task);
    }

    return CONCURRENCY_TEST_PASS;
}

// Test performance and scalability
ConcurrencyTestResult test_concurrency_performance_scalability(AsthraV12TestContext *ctx) {
    const int num_tasks = 8;
    const int work_amount = 10000;
    TestTask *tasks[num_tasks];

    // Measure time for concurrent execution
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Spawn multiple tasks
    for (int i = 0; i < num_tasks; i++) {
        tasks[i] = test_spawn(performance_task, (void *)&work_amount);
        if (!CONCURRENCY_TEST_ASSERT(ctx, tasks[i] != NULL, "Failed to spawn performance task %d",
                                     i)) {
            cleanup_tasks(tasks, i);
            return CONCURRENCY_TEST_FAIL;
        }
    }

    // Wait for all tasks
    for (int i = 0; i < num_tasks; i++) {
        test_task_join(tasks[i]);

        if (!CONCURRENCY_TEST_ASSERT(ctx, tasks[i]->result != NULL,
                                     "Performance task %d should have result", i)) {
            cleanup_tasks(tasks, num_tasks);
            return CONCURRENCY_TEST_FAIL;
        }

        free(tasks[i]->result);
        test_task_destroy(tasks[i]);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate execution time
    double time_ms = timespec_to_ms(&start, &end);

    // Should complete within reasonable time (less than 1 second for this workload)
    if (!CONCURRENCY_TEST_ASSERT(ctx, time_ms < 1000.0, "Performance test took too long: %.2f ms",
                                 time_ms)) {
        return CONCURRENCY_TEST_FAIL;
    }

    printf("  Performance test completed in %.2f ms\n", time_ms);
    return CONCURRENCY_TEST_PASS;
}
