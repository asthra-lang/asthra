#include "benchmark.h"
#include "benchmark_test_cases.h"
#include "benchmark_test_functions.h"
#include "benchmark_test_helpers.h"
#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>

// =============================================================================
// MAIN TEST EXECUTION
// =============================================================================

int main(void) {
    printf("=== Asthra Benchmark Framework Test Suite ===\n\n");

    // Create test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Create test metadata for context
    AsthraTestMetadata metadata = {.name = "benchmark_framework_tests",
                                   .file = __FILE__,
                                   .line = __LINE__,
                                   .function = "main",
                                   .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                   .timeout_ns = 30000000000ULL, // 30 seconds
                                   .skip = false,
                                   .skip_reason = NULL};

    // Create test context
    AsthraTestContext *context = asthra_test_context_create(&metadata, stats);
    if (!context) {
        fprintf(stderr, "Failed to create test context\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }

    // Run all tests
    AsthraTestResult results[] = {
        test_benchmark_timer_accuracy(context),   test_benchmark_configuration(context),
        test_simple_benchmark_execution(context), test_benchmark_with_setup_teardown(context),
        test_benchmark_suite_execution(context),  test_statistical_analysis(context)};

    // Print results
    const char *test_names[] = {"Timer Accuracy", "Configuration",   "Simple Execution",
                                "Setup/Teardown", "Suite Execution", "Statistical Analysis"};

    size_t test_count = sizeof(results) / sizeof(results[0]);
    size_t passed = 0;

    printf("\n=== Test Results ===\n");
    for (size_t i = 0; i < test_count; i++) {
        printf("[%s] %s\n", results[i] == ASTHRA_TEST_PASS ? "PASS" : "FAIL", test_names[i]);
        if (results[i] == ASTHRA_TEST_PASS) {
            passed++;
        }
    }

    printf("\n=== Summary ===\n");
    printf("Tests run: %zu\n", test_count);
    printf("Passed: %zu\n", passed);
    printf("Failed: %zu\n", test_count - passed);
    printf("Pass rate: %.1f%%\n", (double)passed / (double)test_count * 100.0);

    // Demonstrate benchmark framework with actual performance measurements
    printf("\n=== Performance Demonstration ===\n");

    // Create and run a demonstration benchmark suite
    AsthraBenchmarkSuite *demo_suite = asthra_benchmark_suite_create("Asthra C17 Performance Demo");

    AsthraBenchmarkDefinition demo_benchmarks[] = {
        {.config = asthra_benchmark_config_create("String Operations", 1000,
                                                  ASTHRA_BENCHMARK_MODE_SINGLE_THREADED),
         .benchmark_func = benchmark_string_operations,
         .setup_func = NULL,
         .teardown_func = NULL,
         .user_data = NULL},
        {.config = asthra_benchmark_config_create("Buffer Operations", 1000,
                                                  ASTHRA_BENCHMARK_MODE_SINGLE_THREADED),
         .benchmark_func = benchmark_buffer_operations,
         .setup_func = NULL,
         .teardown_func = NULL,
         .user_data = NULL},
        {.config = asthra_benchmark_config_create("Atomic Operations", 10000,
                                                  ASTHRA_BENCHMARK_MODE_SINGLE_THREADED),
         .benchmark_func = benchmark_atomic_operations,
         .setup_func = NULL,
         .teardown_func = NULL,
         .user_data = NULL}};

    for (size_t i = 0; i < sizeof(demo_benchmarks) / sizeof(demo_benchmarks[0]); i++) {
        asthra_benchmark_suite_add(demo_suite, &demo_benchmarks[i]);
    }

    AsthraBenchmarkResult demo_results[10];
    size_t demo_result_count = 0;

    asthra_benchmark_execute_suite(demo_suite, demo_results, &demo_result_count);
    asthra_benchmark_print_suite_results(demo_results, demo_result_count);

    asthra_benchmark_suite_destroy(demo_suite);

    // Cleanup
    asthra_test_context_destroy(context);
    asthra_test_statistics_destroy(stats);

    return (passed == test_count) ? 0 : 1;
}
