#include "benchmark_test_cases.h"
#include "benchmark.h"
#include "benchmark_test_functions.h"
#include "benchmark_test_helpers.h"
#include "test_framework.h"
#include <stdio.h>
#include <unistd.h>

// =============================================================================
// TEST CASE IMPLEMENTATIONS
// =============================================================================

AsthraTestResult test_benchmark_timer_accuracy(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Test timer basic functionality
    AsthraBenchmarkTimer timer = asthra_benchmark_timer_start();

    if (!asthra_test_assert_bool(context, timer.is_valid, "Timer should be valid after start")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, timer.start_ns > 0,
                                 "Timer start time should be positive")) {
        return ASTHRA_TEST_FAIL;
    }

    // Sleep for a known duration
    usleep(10000); // 10ms

    asthra_benchmark_timer_end(&timer);

    if (!asthra_test_assert_bool(context, timer.duration_ns > 0,
                                 "Timer duration should be positive")) {
        return ASTHRA_TEST_FAIL;
    }

    // Check that duration is reasonable (between 8ms and 20ms)
    double duration_ms = asthra_benchmark_ns_to_milliseconds(timer.duration_ns);
    if (!asthra_test_assert_bool(context, duration_ms >= 8.0 && duration_ms <= 20.0,
                                 "Timer duration should be approximately 10ms")) {
        return ASTHRA_TEST_FAIL;
    }

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_benchmark_configuration(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Test default configuration
    AsthraBenchmarkConfig config = asthra_benchmark_config_default("test_benchmark");

    if (!asthra_test_assert_bool(context, asthra_benchmark_config_validate(&config),
                                 "Default config should be valid")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_string_eq(context, config.name, "test_benchmark",
                                      "Config name should match")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test custom configuration
    AsthraBenchmarkConfig custom_config =
        asthra_benchmark_config_create("custom_test", 5000, ASTHRA_BENCHMARK_MODE_SINGLE_THREADED);

    if (!asthra_test_assert_bool(context, asthra_benchmark_config_validate(&custom_config),
                                 "Custom config should be valid")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_size_eq(context, custom_config.iterations, 5000,
                                    "Custom iterations should match")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test invalid configuration
    AsthraBenchmarkConfig invalid_config = config;
    invalid_config.iterations = 0; // Invalid

    if (!asthra_test_assert_bool(context, !asthra_benchmark_config_validate(&invalid_config),
                                 "Invalid config should not validate")) {
        return ASTHRA_TEST_FAIL;
    }

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_simple_benchmark_execution(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Create a simple benchmark
    AsthraBenchmarkConfig config =
        asthra_benchmark_config_create("cpu_test", 100, ASTHRA_BENCHMARK_MODE_SINGLE_THREADED);

    AsthraBenchmarkDefinition benchmark = {.config = config,
                                           .benchmark_func = benchmark_cpu_intensive,
                                           .setup_func = NULL,
                                           .teardown_func = NULL,
                                           .user_data = NULL};

    AsthraBenchmarkResult result;
    AsthraBenchmarkStatus status = asthra_benchmark_execute(&benchmark, &result);

    if (!asthra_test_assert_bool(context, status == ASTHRA_BENCHMARK_STATUS_SUCCESS,
                                 "Benchmark execution should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, result.status == ASTHRA_BENCHMARK_STATUS_SUCCESS,
                                 "Benchmark result status should be success")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_size_eq(context, result.stats.iterations, 100,
                                    "Result should show correct iteration count")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, result.stats.mean_ns > 0,
                                 "Mean execution time should be positive")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, result.stats.throughput_ops_per_sec > 0,
                                 "Throughput should be positive")) {
        return ASTHRA_TEST_FAIL;
    }

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_benchmark_with_setup_teardown(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Create benchmark with setup/teardown
    AsthraBenchmarkConfig config = asthra_benchmark_config_create(
        "large_memory_test", 50, ASTHRA_BENCHMARK_MODE_SINGLE_THREADED);

    AsthraBenchmarkDefinition benchmark = {.config = config,
                                           .benchmark_func = benchmark_large_memory_copy,
                                           .setup_func = setup_large_buffer,
                                           .teardown_func = teardown_large_buffer,
                                           .user_data = NULL};

    AsthraBenchmarkResult result;
    AsthraBenchmarkStatus status = asthra_benchmark_execute(&benchmark, &result);

    if (!asthra_test_assert_bool(context, status == ASTHRA_BENCHMARK_STATUS_SUCCESS,
                                 "Benchmark with setup/teardown should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, result.stats.mean_ns > 0,
                                 "Large memory copy should take measurable time")) {
        return ASTHRA_TEST_FAIL;
    }

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_benchmark_suite_execution(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Create benchmark suite
    AsthraBenchmarkSuite *suite = asthra_benchmark_suite_create("Asthra Performance Suite");

    if (!asthra_test_assert_pointer(context, suite, "Suite creation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Add multiple benchmarks
    AsthraBenchmarkDefinition benchmarks[] = {
        {.config = asthra_benchmark_config_create("string_ops", 200,
                                                  ASTHRA_BENCHMARK_MODE_SINGLE_THREADED),
         .benchmark_func = benchmark_string_operations,
         .setup_func = NULL,
         .teardown_func = NULL,
         .user_data = NULL},
        {.config = asthra_benchmark_config_create("buffer_ops", 200,
                                                  ASTHRA_BENCHMARK_MODE_SINGLE_THREADED),
         .benchmark_func = benchmark_buffer_operations,
         .setup_func = NULL,
         .teardown_func = NULL,
         .user_data = NULL},
        {.config = asthra_benchmark_config_create("memory_ops", 200,
                                                  ASTHRA_BENCHMARK_MODE_SINGLE_THREADED),
         .benchmark_func = benchmark_memory_operations,
         .setup_func = NULL,
         .teardown_func = NULL,
         .user_data = NULL},
        {.config = asthra_benchmark_config_create("atomic_ops", 1000,
                                                  ASTHRA_BENCHMARK_MODE_SINGLE_THREADED),
         .benchmark_func = benchmark_atomic_operations,
         .setup_func = NULL,
         .teardown_func = NULL,
         .user_data = NULL}};

    for (size_t i = 0; i < sizeof(benchmarks) / sizeof(benchmarks[0]); i++) {
        AsthraBenchmarkStatus add_status = asthra_benchmark_suite_add(suite, &benchmarks[i]);
        if (!asthra_test_assert_bool(context, add_status == ASTHRA_BENCHMARK_STATUS_SUCCESS,
                                     "Adding benchmark to suite should succeed")) {
            asthra_benchmark_suite_destroy(suite);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Execute suite
    AsthraBenchmarkResult results[10];
    size_t result_count = 0;

    AsthraBenchmarkStatus suite_status =
        asthra_benchmark_execute_suite(suite, results, &result_count);

    if (!asthra_test_assert_bool(context, suite_status == ASTHRA_BENCHMARK_STATUS_SUCCESS,
                                 "Suite execution should succeed")) {
        asthra_benchmark_suite_destroy(suite);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_size_eq(context, result_count, 4, "Should execute all 4 benchmarks")) {
        asthra_benchmark_suite_destroy(suite);
        return ASTHRA_TEST_FAIL;
    }

    // Verify all benchmarks succeeded
    for (size_t i = 0; i < result_count; i++) {
        if (!asthra_test_assert_bool(context, results[i].status == ASTHRA_BENCHMARK_STATUS_SUCCESS,
                                     "All benchmarks should succeed")) {
            asthra_benchmark_suite_destroy(suite);
            return ASTHRA_TEST_FAIL;
        }
    }

    asthra_benchmark_suite_destroy(suite);
    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_statistical_analysis(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Create test data with known statistical properties
    uint64_t durations[] = {1000, 1100, 1200, 1050, 1150, 1300, 1000, 1100, 1250, 1080};
    size_t count = sizeof(durations) / sizeof(durations[0]);

    AsthraBenchmarkStatistics stats;
    asthra_benchmark_calculate_statistics(durations, count, &stats);

    if (!asthra_test_assert_size_eq(context, stats.iterations, count,
                                    "Statistics should show correct iteration count")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_size_eq(context, stats.min_ns, 1000, "Minimum should be correct")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_size_eq(context, stats.max_ns, 1300, "Maximum should be correct")) {
        return ASTHRA_TEST_FAIL;
    }

    // Mean should be 1123 (sum = 11230, count = 10)
    if (!asthra_test_assert_size_eq(context, stats.mean_ns, 1123, "Mean should be correct")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, stats.throughput_ops_per_sec > 0,
                                 "Throughput should be calculated")) {
        return ASTHRA_TEST_FAIL;
    }

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}
