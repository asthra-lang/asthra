/**
 * Asthra Programming Language Compiler
 * Static Analysis Integration Test Suite - Main Test Runner
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 5.2: Static Analysis Integration Tests - Main Runner
 * - Coordinates execution of all static analysis test modules
 * - Provides comprehensive test statistics and reporting
 * - Manages test suite configuration and execution
 */

#include "test_static_analysis_common.h"

// =============================================================================
// MAIN TEST SUITE
// =============================================================================

int main(void) {
    printf("=== Asthra Static Analysis Integration Test Suite ===\n\n");

    // Create test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Test suite configuration
    AsthraTestSuiteConfig config = {.name = "Static Analysis Integration Tests",
                                    .description =
                                        "Comprehensive tests for C17 static analysis integration",
                                    .parallel_execution = false,
                                    .stop_on_failure = false,
                                    .verbose_output = true,
                                    .json_output = false,
                                    .default_timeout_ns = 30000000000ULL, // 30 seconds
                                    .max_parallel_tests = 1,
                                    .statistics = stats};

    // Test functions and metadata
    AsthraTestFunction tests[] = {test_static_analysis_attributes,
                                  test_const_correct_string_operations,
                                  test_const_correct_memory_operations,
                                  test_restrict_pointer_operations,
                                  test_buffer_operations,
                                  test_string_formatting,
                                  test_static_analysis_configuration};

    AsthraTestMetadata metadata[] = {{.name = "test_static_analysis_attributes",
                                      .file = __FILE__,
                                      .line = __LINE__,
                                      .function = "test_static_analysis_attributes",
                                      .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                      .timeout_ns = 5000000000ULL,
                                      .skip = false,
                                      .skip_reason = NULL},
                                     {.name = "test_const_correct_string_operations",
                                      .file = __FILE__,
                                      .line = __LINE__,
                                      .function = "test_const_correct_string_operations",
                                      .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                      .timeout_ns = 5000000000ULL,
                                      .skip = false,
                                      .skip_reason = NULL},
                                     {.name = "test_const_correct_memory_operations",
                                      .file = __FILE__,
                                      .line = __LINE__,
                                      .function = "test_const_correct_memory_operations",
                                      .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                      .timeout_ns = 5000000000ULL,
                                      .skip = false,
                                      .skip_reason = NULL},
                                     {.name = "test_restrict_pointer_operations",
                                      .file = __FILE__,
                                      .line = __LINE__,
                                      .function = "test_restrict_pointer_operations",
                                      .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                      .timeout_ns = 5000000000ULL,
                                      .skip = false,
                                      .skip_reason = NULL},
                                     {.name = "test_buffer_operations",
                                      .file = __FILE__,
                                      .line = __LINE__,
                                      .function = "test_buffer_operations",
                                      .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
                                      .timeout_ns = 5000000000ULL,
                                      .skip = false,
                                      .skip_reason = NULL},
                                     {.name = "test_string_formatting",
                                      .file = __FILE__,
                                      .line = __LINE__,
                                      .function = "test_string_formatting",
                                      .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
                                      .timeout_ns = 5000000000ULL,
                                      .skip = false,
                                      .skip_reason = NULL},
                                     {.name = "test_static_analysis_configuration",
                                      .file = __FILE__,
                                      .line = __LINE__,
                                      .function = "test_static_analysis_configuration",
                                      .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                      .timeout_ns = 5000000000ULL,
                                      .skip = false,
                                      .skip_reason = NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    // Run test suite
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);

    // Print final statistics
    printf("\n=== Final Test Results ===\n");
    asthra_test_statistics_print(stats, false);

    // Cleanup
    asthra_test_statistics_destroy(stats);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
