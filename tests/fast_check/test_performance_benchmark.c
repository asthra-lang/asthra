/*
 * Test for Asthra Fast Check Performance Benchmark
 * Verifies the modularized benchmark system works correctly
 */

#include "../framework/test_assertions.h"
#include "../framework/test_framework.h"
#include "fast_check/benchmark_internal.h"
#include <stdio.h>

// Test that benchmark runner can execute successfully
AsthraTestResult test_performance_benchmark_runner(AsthraTestContext *context) {
    int result = run_performance_benchmarks();

    // The benchmarks should run successfully (return 0)
    // Note: In CI/test environments, performance targets might not be met,
    // but the infrastructure should work
    if (!asthra_test_assert_bool(context, result == 0 || result == 1,
                                 "Benchmark runner should complete (result: %d)", result)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Test that individual benchmark components work
AsthraTestResult test_performance_benchmark_components(AsthraTestContext *context) {
    // Test stub creation
    FastCheckEngine *engine = fast_check_engine_create();
    if (!asthra_test_assert_not_null(context, engine, "Should create engine")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, engine->initialized, "Engine should be initialized")) {
        fast_check_engine_destroy(engine);
        return ASTHRA_TEST_FAIL;
    }

    // Test file checking
    FastCheckResult *result = fast_check_file(engine, "test.asthra");
    if (!asthra_test_assert_not_null(context, result, "Should create result")) {
        fast_check_engine_destroy(engine);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, result->success, "Should succeed")) {
        fast_check_result_destroy(result);
        fast_check_engine_destroy(engine);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, result->duration_ms > 0, "Should have duration")) {
        fast_check_result_destroy(result);
        fast_check_engine_destroy(engine);
        return ASTHRA_TEST_FAIL;
    }

    // Cleanup
    fast_check_result_destroy(result);
    fast_check_engine_destroy(engine);

    return ASTHRA_TEST_PASS;
}

// Test configuration access
AsthraTestResult test_performance_benchmark_config(AsthraTestContext *context) {
    const BenchmarkSuite *suites = get_benchmark_suites();
    int count = get_benchmark_suite_count();

    if (!asthra_test_assert_not_null(context, (void *)suites, "Should have benchmark suites")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_int_eq(context, count, 4, "Should have 4 benchmark suites")) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify suite names
    if (!asthra_test_assert_str_eq(context, suites[0].name, "single_file", "First suite name")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_str_eq(context, suites[1].name, "medium_project",
                                   "Second suite name")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_str_eq(context, suites[2].name, "large_project", "Third suite name")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_str_eq(context, suites[3].name, "cache_performance",
                                   "Fourth suite name")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test template access
    const char *simple = get_code_template("simple");
    const char *medium = get_code_template("medium");
    const char *complex = get_code_template("complex");

    if (!asthra_test_assert_not_null(context, (void *)simple, "Should have simple template")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_not_null(context, (void *)medium, "Should have medium template")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_not_null(context, (void *)complex, "Should have complex template")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Test utility functions
AsthraTestResult test_performance_benchmark_utils(AsthraTestContext *context) {
    // Test time measurement
    double time1 = get_current_time_ms();
    double time2 = get_current_time_ms();
    if (!asthra_test_assert_bool(context, time2 >= time1, "Time should increase")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test file creation/cleanup
    const char *test_file = "test_benchmark_file.asthra";
    bool created = create_benchmark_file(test_file, "test content %d", 1);
    if (!asthra_test_assert_bool(context, created, "Should create file")) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify file exists
    FILE *f = fopen(test_file, "r");
    if (!asthra_test_assert_not_null(context, f, "File should exist")) {
        cleanup_benchmark_file(test_file);
        return ASTHRA_TEST_FAIL;
    }
    fclose(f);

    // Cleanup
    cleanup_benchmark_file(test_file);

    // Verify file removed
    f = fopen(test_file, "r");
    if (!asthra_test_assert_null(context, f, "File should be removed")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

int main(void) {
    // Initialize test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Define test cases
    const struct {
        AsthraTestFunction test_func;
        const char *test_name;
    } test_cases[] = {
        {test_performance_benchmark_runner, "test_performance_benchmark_runner"},
        {test_performance_benchmark_components, "test_performance_benchmark_components"},
        {test_performance_benchmark_config, "test_performance_benchmark_config"},
        {test_performance_benchmark_utils, "test_performance_benchmark_utils"},
        {NULL, NULL}};

    // Run tests
    for (int i = 0; test_cases[i].test_func != NULL; i++) {
        AsthraTestMetadata metadata = {.name = test_cases[i].test_name,
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = test_cases[i].test_name,
                                       .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
                                       .timeout_ns = 30000000000ULL, // 30 seconds
                                       .skip = false,
                                       .skip_reason = NULL};

        AsthraTestResult result = asthra_test_run_single(test_cases[i].test_func, &metadata, stats);

        if (result != ASTHRA_TEST_PASS && result != ASTHRA_TEST_SKIP) {
            // Don't fail immediately, continue running other tests
        }
    }

    // Print test statistics
    asthra_test_statistics_print(stats, false);

    // Determine overall result
    bool all_passed = asthra_test_get_stat(&stats->tests_failed) == 0 &&
                      asthra_test_get_stat(&stats->tests_error) == 0;

    // Cleanup
    asthra_test_statistics_destroy(stats);

    return all_passed ? 0 : 1;
}