/**
 * Asthra Programming Language v1.2 Pattern Matching Error Propagation Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for error propagation across FFI boundaries, task boundaries,
 * and deep call stacks.
 */

#include "test_pattern_matching_helpers.h"

// =============================================================================
// ERROR PROPAGATION TESTS
// =============================================================================

AsthraTestResult test_error_propagation_ffi_boundaries(AsthraV12TestContext *ctx) {
    // Test error propagation across FFI boundaries

    // Test successful FFI call
    TestResult success_result = mock_ffi_function(50);
    if (!ASTHRA_TEST_ASSERT(&ctx->base, success_result.is_ok,
                            "FFI call with valid input should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, success_result.data.ok_value == 100,
                            "FFI call should return doubled value: expected 100, got %d",
                            success_result.data.ok_value)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test FFI call with negative input
    TestResult negative_result = mock_ffi_function(-10);
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !negative_result.is_ok,
                            "FFI call with negative input should fail")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(
            &ctx->base,
            strcmp(negative_result.data.error_message, "Negative input not allowed") == 0,
            "FFI call should return correct error message")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test FFI call with too large input
    TestResult large_result = mock_ffi_function(2000);
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !large_result.is_ok,
                            "FFI call with large input should fail")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base,
                            strcmp(large_result.data.error_message, "Input too large") == 0,
                            "FFI call should return correct error message for large input")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_error_propagation_task_boundaries(AsthraV12TestContext *ctx) {
    // Test error propagation across task boundaries (simulated)

    // Simulate task that returns Result
    TaskResult task_results[] = {{1, test_result_ok(42)},
                                 {2, test_result_err("Task 2 failed")},
                                 {3, test_result_ok(100)},
                                 {4, test_result_err("Task 4 timeout")},
                                 {5, test_result_ok(0)}};

    size_t task_count = sizeof(task_results) / sizeof(task_results[0]);
    int successful_tasks = 0;
    int failed_tasks = 0;

    for (size_t i = 0; i < task_count; i++) {
        TaskResult task = task_results[i];

        if (task.result.is_ok) {
            successful_tasks++;
            // Verify task result
            if (!ASTHRA_TEST_ASSERT(&ctx->base, task.result.data.ok_value >= 0,
                                    "Task %d should have non-negative result", task.task_id)) {
                return ASTHRA_TEST_FAIL;
            }
        } else {
            failed_tasks++;
            // Verify error message is not empty
            if (!ASTHRA_TEST_ASSERT(&ctx->base, strlen(task.result.data.error_message) > 0,
                                    "Task %d should have non-empty error message", task.task_id)) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, successful_tasks == 3,
                            "Expected 3 successful tasks, got %d", successful_tasks)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, failed_tasks == 2, "Expected 2 failed tasks, got %d",
                            failed_tasks)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_error_propagation_deep_call_stack(AsthraV12TestContext *ctx) {
    // Test error propagation through deep call stacks

    // Function pointer for recursive testing
    TestResult (*recursive_function)(int depth, int max_depth);

    recursive_function = recursive_impl;

    // Test successful deep call
    TestResult deep_ok = recursive_function(0, 5);
    if (!ASTHRA_TEST_ASSERT(&ctx->base, deep_ok.is_ok,
                            "Deep call stack should succeed for even max depth")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, deep_ok.data.ok_value == 10, // 5 + 5 transformations
                            "Deep call result should be 10, got %d", deep_ok.data.ok_value)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test error propagation through deep call
    TestResult deep_err = recursive_function(0, 7); // Odd number should cause error
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !deep_err.is_ok,
                            "Deep call stack should fail for odd max depth")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base,
                            strcmp(deep_err.data.error_message, "Max depth reached") == 0,
                            "Error message should be propagated correctly")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}
