/**
 * Asthra Programming Language
 * Profile-Guided Optimization (PGO) Performance Test Suite
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Main test suite for PGO performance validation with comprehensive benchmarks
 */

#include "test_pgo_benchmarks.h"

// =============================================================================
// TEST SUITE MAIN FUNCTION
// =============================================================================

int main(void) {
    // Initialize test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return EXIT_FAILURE;
    }
    
    // Configure test suite
    AsthraTestSuiteConfig config = asthra_test_suite_config_default();
    config.name = "PGO Performance Test Suite";
    config.description = "Validates Profile-Guided Optimization performance improvements";
    config.verbose_output = true;
    config.stop_on_failure = false;
    config.default_timeout_ns = 120 * 1000000000ULL; // 2 minutes
    config.statistics = stats;
    
    printf("=============================================================================\n");
    printf("Asthra PGO Performance Test Suite\n");
    printf("Testing: Profile-Guided Optimization performance improvements\n");
    printf("Performance threshold: %.1f%% improvement\n", PGO_PERFORMANCE_THRESHOLD_PERCENT);
    printf("Benchmark iterations: %d per test\n", PGO_BENCHMARK_ITERATIONS);
    printf("=============================================================================\n\n");
    
    // Define test cases
    AsthraTestFunction tests[] = {
        test_pgo_fibonacci_benchmark,
        test_pgo_matrix_benchmark,
        test_pgo_sorting_benchmark,
        test_pgo_compilation_performance,
        test_pgo_memory_usage,
        test_pgo_profile_data_quality
    };
    
    AsthraTestMetadata metadata[] = {
        {"PGO Fibonacci Benchmark", __FILE__, __LINE__, "test_pgo_fibonacci_benchmark", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
        {"PGO Matrix Benchmark", __FILE__, __LINE__, "test_pgo_matrix_benchmark", ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
        {"PGO Sorting Benchmark", __FILE__, __LINE__, "test_pgo_sorting_benchmark", ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
        {"PGO Compilation Performance", __FILE__, __LINE__, "test_pgo_compilation_performance", ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
        {"PGO Memory Usage", __FILE__, __LINE__, "test_pgo_memory_usage", ASTHRA_TEST_SEVERITY_LOW, 0, false, NULL},
        {"PGO Profile Data Quality", __FILE__, __LINE__, "test_pgo_profile_data_quality", ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    // Verify test environment before running
    printf("Checking test environment...\n");
    if (!pgo_validate_binaries("bin/asthra", "bin/asthra-pgo-optimized")) {
        printf("⚠ Warning: Required binaries not found. Some tests may be skipped.\n");
        printf("   To run all tests, ensure 'make pgo-optimize' has been executed.\n\n");
    } else {
        printf("✓ Test environment ready\n\n");
    }
    
    // Run the test suite
    AsthraTestResult suite_result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    // Print final results
    printf("\n=============================================================================\n");
    printf("PGO Performance Test Results Summary\n");
    printf("=============================================================================\n");
    asthra_test_statistics_print(stats, false);
    
    // Additional performance summary
    printf("\nPerformance Testing Notes:\n");
    printf("• Performance improvements are workload-dependent and may vary\n");
    printf("• Tests focus on correctness; performance gains are beneficial but not required\n");
    printf("• Skipped tests indicate missing PGO binaries - run 'make pgo-optimize' first\n");
    printf("• Statistical significance requires multiple benchmark iterations\n");
    
    // Cleanup
    asthra_test_statistics_destroy(stats);
    
    return (suite_result == ASTHRA_TEST_PASS) ? EXIT_SUCCESS : EXIT_FAILURE;
} 
