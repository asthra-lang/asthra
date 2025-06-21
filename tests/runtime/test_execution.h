/**
 * Asthra Programming Language v1.2 Test Execution
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test execution logic and output formatting
 */

#ifndef ASTHRA_TEST_EXECUTION_H
#define ASTHRA_TEST_EXECUTION_H

#include "test_registry.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST EXECUTION FUNCTIONS
// =============================================================================

/**
 * Print the test suite header
 */
void print_test_header(void);

/**
 * Print a category header
 * @param category Category name to print
 */
void print_category_header(const char *category);

/**
 * Print the result of a single test
 * @param test_name Name of the test
 * @param result Test result
 * @param duration_ms Test duration in milliseconds
 */
void print_test_result(const char *test_name, AsthraTestResult result, double duration_ms);

/**
 * Print the comprehensive test summary
 * @param summary Test summary to print
 */
void print_test_summary(const TestSummary *summary);

/**
 * Execute a single test with timing and error handling
 * @param entry Test registry entry to execute
 * @param summary Test summary to update
 * @return Test result
 */
AsthraTestResult execute_single_test(const TestRegistryEntry *entry, TestSummary *summary);

/**
 * Check if a test should be skipped based on filters
 * @param entry Test registry entry
 * @param run_performance_tests Whether to run performance tests
 * @param run_security_tests Whether to run security tests
 * @param filter_category Category filter (NULL for no filter)
 * @return true if test should be skipped
 */
bool should_skip_test(const TestRegistryEntry *entry, 
                     bool run_performance_tests, 
                     bool run_security_tests, 
                     const char *filter_category);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_EXECUTION_H 
