/**
 * Asthra Programming Language v1.2 Integration Tests
 * Comprehensive Performance Integration Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_integration_common.h"

// =============================================================================
// PERFORMANCE COMPREHENSIVE INTEGRATION TEST
// =============================================================================

// External function declaration (assumed to be from test framework)
extern int test_c_add(int a, int b);

// Scenario 5: Performance integration test
AsthraTestResult test_integration_performance_comprehensive(AsthraV12TestContext *ctx) {
    // Integration test: All features combined for performance testing

    asthra_benchmark_start(ctx);

    const int iterations = 1000;
    const int concurrent_tasks = 8;

    uint64_t start_time = asthra_test_get_time_ns();

    // Combined workload: string processing, slice operations, FFI calls, pattern matching
    typedef struct {
        int task_id;
        int operations_completed;
        uint64_t task_duration;
    } PerformanceTask;

    PerformanceTask tasks[concurrent_tasks];
    pthread_t task_threads[concurrent_tasks];

    void *performance_worker(void *arg) {
        PerformanceTask *task = (PerformanceTask *)arg;
        uint64_t task_start = asthra_test_get_time_ns();

        for (int i = 0; i < iterations / concurrent_tasks; i++) {
            // String operation
            char buffer[100];
            snprintf(buffer, sizeof(buffer), "Task %d iteration %d", task->task_id, i);

            // Mock pattern matching
            int pattern_result = 0;
            if (strlen(buffer) > 10) {
                pattern_result = 1;
            } else if (strlen(buffer) > 5) {
                pattern_result = 2;
            } else {
                pattern_result = 3;
            }

            // Mock FFI call
            int ffi_result = test_c_add(i, pattern_result);

            // Slice-like operation
            int *data = malloc(sizeof(int));
            *data = ffi_result;

            // Simulate processing
            if (*data > 0) {
                task->operations_completed++;
            }

            free(data);

            asthra_benchmark_iteration(ctx);
        }

        task->task_duration = asthra_test_get_time_ns() - task_start;
        return task;
    }

    // Start performance tasks
    for (int i = 0; i < concurrent_tasks; i++) {
        tasks[i].task_id = i;
        tasks[i].operations_completed = 0;
        tasks[i].task_duration = 0;

        int result = pthread_create(&task_threads[i], NULL, performance_worker, &tasks[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, result == 0,
                                "Performance task %d creation should succeed", i)) {
            // Cleanup
            for (int j = 0; j < i; j++) {
                pthread_join(task_threads[j], NULL);
            }
            return ASTHRA_TEST_FAIL;
        }
    }

    // Wait for completion
    for (int i = 0; i < concurrent_tasks; i++) {
        pthread_join(task_threads[i], NULL);
    }

    uint64_t end_time = asthra_test_get_time_ns();
    uint64_t total_duration = end_time - start_time;

    asthra_benchmark_end(ctx);

    // Verify all tasks completed
    int total_operations = 0;
    for (int i = 0; i < concurrent_tasks; i++) {
        total_operations += tasks[i].operations_completed;

        if (!ASTHRA_TEST_ASSERT(&ctx->base, tasks[i].operations_completed > 0,
                                "Task %d should complete some operations", i)) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Performance assertions
    double duration_seconds = (double)total_duration / 1e9;
    double throughput = (double)total_operations / duration_seconds;

    if (!ASTHRA_TEST_ASSERT(&ctx->base, duration_seconds < 10.0,
                            "Performance test should complete in reasonable time: %.2f seconds",
                            duration_seconds)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, throughput > 100.0,
                            "Throughput should be reasonable: %.0f ops/sec", throughput)) {
        return ASTHRA_TEST_FAIL;
    }

    // Record performance metrics
    ctx->benchmark.throughput_ops_per_sec = throughput;
    ctx->benchmark.min_duration_ns = total_duration;
    ctx->benchmark.max_duration_ns = total_duration;

    return ASTHRA_TEST_PASS;
}
