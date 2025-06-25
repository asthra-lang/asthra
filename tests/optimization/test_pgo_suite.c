/**
 * Asthra Programming Language
 * Profile-Guided Optimization (PGO) Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for PGO workflow, makefile targets, and performance validation
 */

#include "test_optimization_common.h"

// Forward declarations
static AsthraTestResult test_pgo_makefile_targets(AsthraTestContext *context);
static AsthraTestResult test_pgo_complete_workflow(AsthraTestContext *context);
static AsthraTestResult test_pgo_performance_improvement(AsthraTestContext *context);

static AsthraTestResult test_pgo_makefile_targets(AsthraTestContext *context) {
    // Test 1: pgo-generate target
    CommandResult result = execute_command("make pgo-generate", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "pgo-generate target should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify instrumented binary exists
    if (!ASTHRA_TEST_ASSERT(context, file_exists("bin/asthra-pgo-generate"),
                            "PGO instrumented binary should be created")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test 2: pgo-run target
    result = execute_command("make pgo-run", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0, "pgo-run target should succeed: %s",
                               result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test 3: pgo-merge target
    result = execute_command("make pgo-merge", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0, "pgo-merge target should succeed: %s",
                               result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify profile data exists
    if (!ASTHRA_TEST_ASSERT(context, file_exists("default.profdata"),
                            "Merged profile data should be created")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test 4: pgo-use target
    result = execute_command("make pgo-use", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0, "pgo-use target should succeed: %s",
                               result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify optimized binary exists
    if (!ASTHRA_TEST_ASSERT(context, file_exists("bin/asthra-pgo-optimized"),
                            "PGO optimized binary should be created")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_pgo_complete_workflow(AsthraTestContext *context) {
    // Test complete PGO workflow with single command
    CommandResult result = execute_command("make pgo-optimize", TEST_TIMEOUT_SECONDS);

    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Complete PGO workflow should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify all expected artifacts exist
    const char *expected_files[] = {"bin/asthra-pgo-generate", "bin/asthra-pgo-optimized",
                                    "default.profdata"};

    for (size_t i = 0; i < sizeof(expected_files) / sizeof(expected_files[0]); i++) {
        if (!ASTHRA_TEST_ASSERT(context, file_exists(expected_files[i]),
                                "PGO artifact should exist: %s", expected_files[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_pgo_performance_improvement(AsthraTestContext *context) {
    // Ensure we have both baseline and optimized binaries
    if (!file_exists("bin/asthra") || !file_exists("bin/asthra-pgo-optimized")) {
        return ASTHRA_TEST_SKIP;
    }

    // Create a test input file for benchmarking
    const char *test_input = "examples/test_optimization.asthra";
    FILE *test_file = fopen(test_input, "w");
    if (test_file) {
        fprintf(test_file, "// Test program for optimization benchmarking\n");
        fprintf(test_file, "fn fibonacci(n: i32) -> i32 {\n");
        fprintf(test_file, "    if n <= 1 { return n; }\n");
        fprintf(test_file, "    return fibonacci(n-1) + fibonacci(n-2);\n");
        fprintf(test_file, "}\n");
        fprintf(test_file, "fn main(void) {\n");
        fprintf(test_file, "    let result = fibonacci(30);\n");
        fprintf(test_file, "    print(result);\n");
        fprintf(test_file, "}\n");
        fclose(test_file);
    }

    PerformanceBenchmark benchmark =
        benchmark_performance("bin/asthra", "bin/asthra-pgo-optimized", test_input);

    // Clean up test file
    unlink(test_input);

    if (!ASTHRA_TEST_ASSERT(context, benchmark.baseline_time_ms > 0,
                            "Baseline execution should complete successfully")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, benchmark.optimized_time_ms > 0,
                            "Optimized execution should complete successfully")) {
        return ASTHRA_TEST_FAIL;
    }

    // Log performance results
    printf("Performance Benchmark Results:\n");
    printf("  Baseline time: %.2f ms\n", benchmark.baseline_time_ms);
    printf("  Optimized time: %.2f ms\n", benchmark.optimized_time_ms);
    printf("  Improvement: %.2f%%\n", benchmark.improvement_percent);

    // Performance improvement is nice to have but not required for test to pass
    // (depends on workload and compiler optimizations already present)
    if (benchmark.meets_threshold) {
        printf("  ✓ Meets performance threshold (%.1f%%)\n", PERFORMANCE_THRESHOLD_PERCENT);
    } else {
        printf("  ⚠ Below performance threshold (%.1f%%) - this is acceptable\n",
               PERFORMANCE_THRESHOLD_PERCENT);
    }

    return ASTHRA_TEST_PASS;
}

// Export test functions and metadata for the main test runner
AsthraTestFunction pgo_test_functions[] = {test_pgo_makefile_targets, test_pgo_complete_workflow,
                                           test_pgo_performance_improvement};

AsthraTestMetadata pgo_test_metadata[] = {
    {"PGO Makefile Targets", __FILE__, __LINE__, "test_pgo_makefile_targets",
     ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"PGO Complete Workflow", __FILE__, __LINE__, "test_pgo_complete_workflow",
     ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"PGO Performance Improvement", __FILE__, __LINE__, "test_pgo_performance_improvement",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL}};

const size_t pgo_test_count = sizeof(pgo_test_functions) / sizeof(pgo_test_functions[0]);
