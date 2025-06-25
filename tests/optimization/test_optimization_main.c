/**
 * Asthra Programming Language
 * Optimization Tools Integration Test Suite - Main Runner
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test runner that orchestrates all optimization test suites:
 * - PGO (Profile-Guided Optimization)
 * - Clang Sanitizers (ASan, TSan, UBSan, MSan)
 * - Enhanced Gperf integration
 * - Automation scripts
 * - Makefile targets
 * - CI/CD compatibility
 */

#include "test_optimization_common.h"

// Include test suite declarations
extern AsthraTestFunction pgo_test_functions[];
extern AsthraTestMetadata pgo_test_metadata[];
extern const size_t pgo_test_count;

extern AsthraTestFunction sanitizer_test_functions[];
extern AsthraTestMetadata sanitizer_test_metadata[];
extern const size_t sanitizer_test_count;

extern AsthraTestFunction gperf_test_functions[];
extern AsthraTestMetadata gperf_test_metadata[];
extern const size_t gperf_test_count;

extern AsthraTestFunction automation_test_functions[];
extern AsthraTestMetadata automation_test_metadata[];
extern const size_t automation_test_count;

extern AsthraTestFunction makefile_test_functions[];
extern AsthraTestMetadata makefile_test_metadata[];
extern const size_t makefile_test_count;

extern AsthraTestFunction cicd_test_functions[];
extern AsthraTestMetadata cicd_test_metadata[];
extern const size_t cicd_test_count;

// New: Advanced Optimization Pass Tests
extern AsthraTestFunction advanced_optimization_pass_test_functions[];
extern AsthraTestMetadata advanced_optimization_pass_test_metadata[];
extern const size_t advanced_optimization_pass_test_count;

// Test suite runner function
static AsthraTestResult run_test_suite(const char *suite_name, AsthraTestFunction *functions,
                                       AsthraTestMetadata *metadata, size_t test_count,
                                       AsthraTestSuiteConfig *config) {
    printf("\n--- %s ---\n", suite_name);

    AsthraTestResult suite_result = asthra_test_run_suite(functions, metadata, test_count, config);

    printf("Suite '%s' result: %s\n", suite_name,
           (suite_result == ASTHRA_TEST_PASS)   ? "PASS"
           : (suite_result == ASTHRA_TEST_FAIL) ? "FAIL"
                                                : "SKIP");

    return suite_result;
}

int main(void) {
    // Initialize test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return EXIT_FAILURE;
    }

    // Configure test suite
    AsthraTestSuiteConfig config = asthra_test_suite_config_default();
    config.name = "Optimization Tools Integration Test Suite";
    config.description = "Validates Clang-native optimization tools migration";
    config.verbose_output = true;
    config.stop_on_failure = false;
    config.default_timeout_ns = TEST_TIMEOUT_SECONDS * 1000000000ULL;
    config.statistics = stats;

    printf("=============================================================================\n");
    printf("Asthra Optimization Tools Integration Test Suite\n");
    printf("Testing: PGO, Sanitizers, Gperf, Makefile targets, and automation scripts\n");
    printf("=============================================================================\n");

    // Track overall results
    int total_suites = 0;
    int passed_suites = 0;
    int failed_suites = 0;
    int skipped_suites = 0;

    // Run PGO test suite
    total_suites++;
    AsthraTestResult pgo_result = run_test_suite("PGO Test Suite", pgo_test_functions,
                                                 pgo_test_metadata, pgo_test_count, &config);
    if (pgo_result == ASTHRA_TEST_PASS)
        passed_suites++;
    else if (pgo_result == ASTHRA_TEST_FAIL)
        failed_suites++;
    else
        skipped_suites++;

    // Run Sanitizer test suite
    total_suites++;
    AsthraTestResult sanitizer_result =
        run_test_suite("Sanitizer Test Suite", sanitizer_test_functions, sanitizer_test_metadata,
                       sanitizer_test_count, &config);
    if (sanitizer_result == ASTHRA_TEST_PASS)
        passed_suites++;
    else if (sanitizer_result == ASTHRA_TEST_FAIL)
        failed_suites++;
    else
        skipped_suites++;

    // Run Gperf test suite
    total_suites++;
    AsthraTestResult gperf_result =
        run_test_suite("Gperf Integration Test Suite", gperf_test_functions, gperf_test_metadata,
                       gperf_test_count, &config);
    if (gperf_result == ASTHRA_TEST_PASS)
        passed_suites++;
    else if (gperf_result == ASTHRA_TEST_FAIL)
        failed_suites++;
    else
        skipped_suites++;

    // Run Automation Scripts test suite
    total_suites++;
    AsthraTestResult automation_result =
        run_test_suite("Automation Scripts Test Suite", automation_test_functions,
                       automation_test_metadata, automation_test_count, &config);
    if (automation_result == ASTHRA_TEST_PASS)
        passed_suites++;
    else if (automation_result == ASTHRA_TEST_FAIL)
        failed_suites++;
    else
        skipped_suites++;

    // Run Makefile Integration test suite
    total_suites++;
    AsthraTestResult makefile_result =
        run_test_suite("Makefile Integration Test Suite", makefile_test_functions,
                       makefile_test_metadata, makefile_test_count, &config);
    if (makefile_result == ASTHRA_TEST_PASS)
        passed_suites++;
    else if (makefile_result == ASTHRA_TEST_FAIL)
        failed_suites++;
    else
        skipped_suites++;

    // Run CI/CD Compatibility test suite
    total_suites++;
    AsthraTestResult cicd_result =
        run_test_suite("CI/CD Compatibility Test Suite", cicd_test_functions, cicd_test_metadata,
                       cicd_test_count, &config);
    if (cicd_result == ASTHRA_TEST_PASS)
        passed_suites++;
    else if (cicd_result == ASTHRA_TEST_FAIL)
        failed_suites++;
    else
        skipped_suites++;

    // New: Run Advanced Optimization Pass test suite
    total_suites++;
    AsthraTestResult advanced_opt_result = run_test_suite(
        "Advanced Optimization Pass Test Suite", advanced_optimization_pass_test_functions,
        advanced_optimization_pass_test_metadata, advanced_optimization_pass_test_count, &config);
    if (advanced_opt_result == ASTHRA_TEST_PASS)
        passed_suites++;
    else if (advanced_opt_result == ASTHRA_TEST_FAIL)
        failed_suites++;
    else
        skipped_suites++;

    // Print final summary
    printf("\n=============================================================================\n");
    printf("OVERALL TEST SUITE SUMMARY\n");
    printf("=============================================================================\n");
    printf("Total Test Suites: %d\n", total_suites);
    printf("Passed Suites:     %d\n", passed_suites);
    printf("Failed Suites:     %d\n", failed_suites);
    printf("Skipped Suites:    %d\n", skipped_suites);
    printf("\nIndividual Test Statistics:\n");
    asthra_test_statistics_print(stats, false);

    // Determine overall result
    bool overall_success = (failed_suites == 0);

    printf("\n=============================================================================\n");
    printf("OVERALL RESULT: %s\n", overall_success ? "✓ PASS" : "✗ FAIL");
    printf("=============================================================================\n");

    // Cleanup
    asthra_test_statistics_destroy(stats);

    return overall_success ? EXIT_SUCCESS : EXIT_FAILURE;
}
