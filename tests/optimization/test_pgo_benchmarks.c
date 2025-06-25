/**
 * Asthra Programming Language
 * Profile-Guided Optimization (PGO) Individual Benchmark Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Individual benchmark test implementations for PGO performance validation
 */

#include "test_pgo_performance_common.h"

// =============================================================================
// INDIVIDUAL BENCHMARK TESTS
// =============================================================================

AsthraTestResult test_pgo_fibonacci_benchmark(AsthraTestContext *context) {
    const char *test_program = "examples/benchmark_fibonacci.asthra";
    const char *baseline_binary = "bin/asthra";
    const char *optimized_binary = "bin/asthra-pgo-optimized";

    // Create test program
    if (!pgo_create_test_program(test_program, PGO_BENCHMARK_FIBONACCI)) {
        return ASTHRA_TEST_SKIP;
    }

    // Check if both binaries exist
    if (!pgo_validate_binaries(baseline_binary, optimized_binary)) {
        unlink(test_program);
        return ASTHRA_TEST_SKIP;
    }

    PGOPerformanceStats baseline_stats, optimized_stats;

    // Benchmark baseline binary
    char baseline_args[PGO_MAX_COMMAND_LENGTH];
    snprintf(baseline_args, sizeof(baseline_args), "%s", test_program);

    if (!pgo_run_binary_benchmark(baseline_binary, baseline_args, &baseline_stats)) {
        unlink(test_program);
        return ASTHRA_TEST_FAIL;
    }

    // Benchmark optimized binary
    char optimized_args[PGO_MAX_COMMAND_LENGTH];
    snprintf(optimized_args, sizeof(optimized_args), "%s", test_program);

    if (!pgo_run_binary_benchmark(optimized_binary, optimized_args, &optimized_stats)) {
        unlink(test_program);
        return ASTHRA_TEST_FAIL;
    }

    // Compare performance
    PGOBenchmarkComparison comparison = pgo_compare_performance(baseline_stats, optimized_stats);

    // Print detailed results
    pgo_print_benchmark_comparison("Fibonacci", &comparison);

    // Cleanup
    unlink(test_program);

    // Test assertions
    if (!ASTHRA_TEST_ASSERT(context, baseline_stats.avg_time_ms > 0,
                            "Baseline benchmark should complete successfully")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, optimized_stats.avg_time_ms > 0,
                            "Optimized benchmark should complete successfully")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_pgo_matrix_benchmark(AsthraTestContext *context) {
    const char *test_program = "examples/benchmark_matrix.asthra";
    const char *baseline_binary = "bin/asthra";
    const char *optimized_binary = "bin/asthra-pgo-optimized";

    // Create test program
    if (!pgo_create_test_program(test_program, PGO_BENCHMARK_MATRIX)) {
        return ASTHRA_TEST_SKIP;
    }

    // Check if both binaries exist
    if (!pgo_validate_binaries(baseline_binary, optimized_binary)) {
        unlink(test_program);
        return ASTHRA_TEST_SKIP;
    }

    PGOPerformanceStats baseline_stats, optimized_stats;

    // Benchmark baseline binary
    char baseline_args[PGO_MAX_COMMAND_LENGTH];
    snprintf(baseline_args, sizeof(baseline_args), "%s", test_program);

    if (!pgo_run_binary_benchmark(baseline_binary, baseline_args, &baseline_stats)) {
        unlink(test_program);
        return ASTHRA_TEST_FAIL;
    }

    // Benchmark optimized binary
    char optimized_args[PGO_MAX_COMMAND_LENGTH];
    snprintf(optimized_args, sizeof(optimized_args), "%s", test_program);

    if (!pgo_run_binary_benchmark(optimized_binary, optimized_args, &optimized_stats)) {
        unlink(test_program);
        return ASTHRA_TEST_FAIL;
    }

    // Compare performance
    PGOBenchmarkComparison comparison = pgo_compare_performance(baseline_stats, optimized_stats);

    // Print results
    pgo_print_benchmark_comparison("Matrix Multiplication", &comparison);

    // Cleanup
    unlink(test_program);

    // Test assertions
    if (!ASTHRA_TEST_ASSERT(context, baseline_stats.avg_time_ms > 0,
                            "Baseline benchmark should complete successfully")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, optimized_stats.avg_time_ms > 0,
                            "Optimized benchmark should complete successfully")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_pgo_sorting_benchmark(AsthraTestContext *context) {
    const char *test_program = "examples/benchmark_sorting.asthra";
    const char *baseline_binary = "bin/asthra";
    const char *optimized_binary = "bin/asthra-pgo-optimized";

    // Create test program
    if (!pgo_create_test_program(test_program, PGO_BENCHMARK_SORTING)) {
        return ASTHRA_TEST_SKIP;
    }

    // Check if both binaries exist
    if (!pgo_validate_binaries(baseline_binary, optimized_binary)) {
        unlink(test_program);
        return ASTHRA_TEST_SKIP;
    }

    PGOPerformanceStats baseline_stats, optimized_stats;

    // Benchmark baseline binary
    char baseline_args[PGO_MAX_COMMAND_LENGTH];
    snprintf(baseline_args, sizeof(baseline_args), "%s", test_program);

    if (!pgo_run_binary_benchmark(baseline_binary, baseline_args, &baseline_stats)) {
        unlink(test_program);
        return ASTHRA_TEST_FAIL;
    }

    // Benchmark optimized binary
    char optimized_args[PGO_MAX_COMMAND_LENGTH];
    snprintf(optimized_args, sizeof(optimized_args), "%s", test_program);

    if (!pgo_run_binary_benchmark(optimized_binary, optimized_args, &optimized_stats)) {
        unlink(test_program);
        return ASTHRA_TEST_FAIL;
    }

    // Compare performance
    PGOBenchmarkComparison comparison = pgo_compare_performance(baseline_stats, optimized_stats);

    // Print results
    pgo_print_benchmark_comparison("Sorting", &comparison);

    // Cleanup
    unlink(test_program);

    // Test assertions
    if (!ASTHRA_TEST_ASSERT(context, baseline_stats.avg_time_ms > 0,
                            "Baseline benchmark should complete successfully")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, optimized_stats.avg_time_ms > 0,
                            "Optimized benchmark should complete successfully")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_pgo_compilation_performance(AsthraTestContext *context) {
    // Test that PGO doesn't significantly slow down compilation itself

    const char *test_program = "examples/benchmark_compile.asthra";

    // Create a moderately complex test program
    if (!pgo_create_test_program(test_program, PGO_BENCHMARK_SORTING)) {
        return ASTHRA_TEST_SKIP;
    }

    PGOPerformanceStats baseline_stats, optimized_stats;

    // Benchmark baseline compilation
    char baseline_cmd[PGO_MAX_COMMAND_LENGTH];
    snprintf(baseline_cmd, sizeof(baseline_cmd), "bin/asthra --compile-only %s", test_program);

    if (!pgo_run_binary_benchmark("sh", baseline_cmd, &baseline_stats)) {
        unlink(test_program);
        return ASTHRA_TEST_FAIL;
    }

    // Benchmark PGO compilation (if available)
    char optimized_cmd[PGO_MAX_COMMAND_LENGTH];
    snprintf(optimized_cmd, sizeof(optimized_cmd), "bin/asthra-pgo-optimized --compile-only %s",
             test_program);

    if (file_exists("bin/asthra-pgo-optimized")) {
        if (!pgo_run_binary_benchmark("sh", optimized_cmd, &optimized_stats)) {
            unlink(test_program);
            return ASTHRA_TEST_FAIL;
        }

        PGOBenchmarkComparison comparison =
            pgo_compare_performance(baseline_stats, optimized_stats);

        printf("\nCompilation Performance Results:\n");
        printf("Baseline compilation: %.2f ms average\n", baseline_stats.avg_time_ms);
        printf("PGO compilation: %.2f ms average\n", optimized_stats.avg_time_ms);
        printf("Compilation overhead: %.2f%%\n", -comparison.improvement_percent);

        // PGO compilation should not be significantly slower
        if (!ASTHRA_TEST_ASSERT(context, comparison.improvement_percent > -50.0,
                                "PGO compilation should not be more than 50%% slower")) {
            unlink(test_program);
            return ASTHRA_TEST_FAIL;
        }
    } else {
        printf("PGO binary not available, skipping compilation performance test\n");
    }

    // Cleanup
    unlink(test_program);

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_pgo_memory_usage(AsthraTestContext *context) {
    // Test that PGO doesn't significantly increase memory usage

    const char *baseline_binary = "bin/asthra";
    const char *optimized_binary = "bin/asthra-pgo-optimized";

    struct stat baseline_st, optimized_st;

    if (stat(baseline_binary, &baseline_st) != 0 || stat(optimized_binary, &optimized_st) != 0) {
        return ASTHRA_TEST_SKIP;
    }

    // Compare binary sizes
    double size_increase_percent =
        ((double)(optimized_st.st_size - baseline_st.st_size) / baseline_st.st_size) * 100.0;

    printf("\nBinary Size Comparison:\n");
    printf("Baseline binary: %lld bytes\n", (long long)baseline_st.st_size);
    printf("PGO binary: %lld bytes\n", (long long)optimized_st.st_size);
    printf("Size increase: %.2f%%\n", size_increase_percent);

    // PGO binary should not be excessively larger
    if (!ASTHRA_TEST_ASSERT(context, size_increase_percent < 25.0,
                            "PGO binary should not be more than 25%% larger")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_pgo_profile_data_quality(AsthraTestContext *context) {
    // Test that profile data is generated and has reasonable quality

    const char *profile_data = "default.profdata";
    struct stat st;

    if (stat(profile_data, &st) != 0) {
        return ASTHRA_TEST_SKIP;
    }

    printf("\nProfile Data Quality:\n");
    printf("Profile data size: %lld bytes\n", (long long)st.st_size);

    // Profile data should exist and have reasonable size
    if (!ASTHRA_TEST_ASSERT(context, st.st_size > 1000,
                            "Profile data should be substantial (>1KB)")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, st.st_size < 100 * 1024 * 1024,
                            "Profile data should be reasonable size (<100MB)")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}
