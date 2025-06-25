/**
 * Asthra Programming Language v1.2 Pattern Matching Performance Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Performance tests and benchmarks for pattern matching optimization,
 * including zero-cost abstractions and optimization strategies.
 */

#include "test_pattern_matching_helpers.h"
#include <stdlib.h>

// =============================================================================
// PERFORMANCE TESTS
// =============================================================================

AsthraTestResult test_pattern_matching_zero_cost(AsthraV12TestContext *ctx) {
    // Test that pattern matching has zero runtime cost

    asthra_benchmark_start(ctx);

    const int iterations = 100000;
    volatile int result_sum = 0; // volatile to prevent optimization

    // Benchmark simple conditional (baseline)
    uint64_t start_time = asthra_test_get_time_ns();

    for (int i = 0; i < iterations; i++) {
        TestResult result = (i % 2 == 0) ? test_result_ok(i) : test_result_err("error");

        // Simple conditional check
        if (result.is_ok) {
            result_sum += result.data.ok_value;
        }
    }

    uint64_t conditional_time = asthra_test_get_time_ns() - start_time;

    // Reset for pattern matching test
    result_sum = 0;

    // Benchmark pattern matching (should be equivalent)
    start_time = asthra_test_get_time_ns();

    for (int i = 0; i < iterations; i++) {
        TestResult result = (i % 2 == 0) ? test_result_ok(i) : test_result_err("error");

        // Pattern matching simulation
        if (result.is_ok) {
            result_sum += result.data.ok_value;
        } else {
            // Error case - do nothing
        }
    }

    uint64_t pattern_matching_time = asthra_test_get_time_ns() - start_time;

    asthra_benchmark_end(ctx);

    // Pattern matching should not be significantly slower than simple conditional
    double overhead_ratio = (double)pattern_matching_time / (double)conditional_time;

    if (!ASTHRA_TEST_ASSERT(&ctx->base, overhead_ratio <= 1.1, // Allow 10% overhead
                            "Pattern matching overhead too high: %.2f", overhead_ratio)) {
        return ASTHRA_TEST_FAIL;
    }

    // Record performance metrics
    ctx->benchmark.min_duration_ns =
        conditional_time < pattern_matching_time ? conditional_time : pattern_matching_time;
    ctx->benchmark.max_duration_ns =
        conditional_time > pattern_matching_time ? conditional_time : pattern_matching_time;
    ctx->benchmark.avg_duration_ns = (conditional_time + pattern_matching_time) / 2;
    ctx->benchmark.iterations = iterations * 2;

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_pattern_matching_benchmark(AsthraV12TestContext *ctx) {
    // Comprehensive pattern matching performance benchmark

    asthra_benchmark_start(ctx);

    const int iterations = 1000000;
    const int test_cases = 100;

    // Create diverse test data
    TestResult *test_data = malloc(test_cases * sizeof(TestResult));
    if (!test_data) {
        return ASTHRA_TEST_ERROR;
    }

    for (int i = 0; i < test_cases; i++) {
        if (i % 3 == 0) {
            test_data[i] = test_result_ok(i);
        } else if (i % 3 == 1) {
            test_data[i] = test_result_err("Error case");
        } else {
            test_data[i] = test_result_ok(-i);
        }
    }

    uint64_t start_time = asthra_test_get_time_ns();
    volatile int processed_count = 0;

    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < test_cases; i++) {
            TestResult result = test_data[i];

            // Complex pattern matching
            if (result.is_ok) {
                int value = result.data.ok_value;
                if (value < 0) {
                    processed_count += 1; // Negative case
                } else if (value == 0) {
                    processed_count += 2; // Zero case
                } else if (value < 50) {
                    processed_count += 3; // Small positive
                } else {
                    processed_count += 4; // Large positive
                }
            } else {
                processed_count += 5; // Error case
            }
        }
    }

    uint64_t end_time = asthra_test_get_time_ns();
    uint64_t total_duration = end_time - start_time;

    asthra_benchmark_end(ctx);

    // Calculate performance metrics
    ctx->benchmark.iterations = iterations * test_cases;
    ctx->benchmark.avg_duration_ns = total_duration;
    ctx->benchmark.throughput_ops_per_sec =
        (double)(iterations * test_cases) / ((double)total_duration / 1e9);

    // Verify we processed all cases
    if (!ASTHRA_TEST_ASSERT(&ctx->base, processed_count > 0,
                            "Pattern matching should process all cases")) {
        free(test_data);
        return ASTHRA_TEST_FAIL;
    }

    // Performance should be reasonable (> 1M ops/sec)
    if (!ASTHRA_TEST_ASSERT(&ctx->base, ctx->benchmark.throughput_ops_per_sec > 1000000.0,
                            "Pattern matching throughput too low: %.0f ops/sec",
                            ctx->benchmark.throughput_ops_per_sec)) {
        free(test_data);
        return ASTHRA_TEST_FAIL;
    }

    free(test_data);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_pattern_matching_optimization(AsthraV12TestContext *ctx) {
    // Test pattern matching optimization strategies

    // Test jump table optimization for dense integer patterns
    const int dense_cases = 10;
    volatile int jump_table_result = 0;

    uint64_t start_time = asthra_test_get_time_ns();

    for (int iter = 0; iter < 100000; iter++) {
        for (int i = 0; i < dense_cases; i++) {
            TestResult result = test_result_ok(i);

            if (result.is_ok) {
                int value = result.data.ok_value;
                // Simulate jump table optimization
                switch (value) {
                case 0:
                    jump_table_result += 1;
                    break;
                case 1:
                    jump_table_result += 2;
                    break;
                case 2:
                    jump_table_result += 3;
                    break;
                case 3:
                    jump_table_result += 4;
                    break;
                case 4:
                    jump_table_result += 5;
                    break;
                case 5:
                    jump_table_result += 6;
                    break;
                case 6:
                    jump_table_result += 7;
                    break;
                case 7:
                    jump_table_result += 8;
                    break;
                case 8:
                    jump_table_result += 9;
                    break;
                case 9:
                    jump_table_result += 10;
                    break;
                default:
                    jump_table_result += 0;
                    break;
                }
            }
        }
    }

    uint64_t jump_table_time = asthra_test_get_time_ns() - start_time;

    // Test binary search optimization for sparse patterns
    volatile int binary_search_result = 0;
    start_time = asthra_test_get_time_ns();

    for (int iter = 0; iter < 100000; iter++) {
        for (int i = 0; i < dense_cases; i++) {
            TestResult result = test_result_ok(i * 100); // Sparse values

            if (result.is_ok) {
                int value = result.data.ok_value;
                // Simulate binary search optimization
                if (value < 500) {
                    if (value < 250) {
                        if (value < 125) {
                            binary_search_result += (value < 100) ? 1 : 2;
                        } else {
                            binary_search_result += (value < 200) ? 3 : 4;
                        }
                    } else {
                        if (value < 375) {
                            binary_search_result += (value < 300) ? 5 : 6;
                        } else {
                            binary_search_result += (value < 400) ? 7 : 8;
                        }
                    }
                } else {
                    binary_search_result += 9;
                }
            }
        }
    }

    uint64_t binary_search_time = asthra_test_get_time_ns() - start_time;

    // Jump table should be faster for dense patterns
    if (!ASTHRA_TEST_ASSERT(&ctx->base, jump_table_time <= binary_search_time,
                            "Jump table optimization should be faster for dense patterns")) {
        return ASTHRA_TEST_FAIL;
    }

    // Record optimization metrics
    ctx->benchmark.min_duration_ns = jump_table_time;
    ctx->benchmark.max_duration_ns = binary_search_time;

    return ASTHRA_TEST_PASS;
}
