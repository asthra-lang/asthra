/**
 * Asthra Programming Language v1.2 Test Registry
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test registry and data structures for the comprehensive test runner
 */

#ifndef ASTHRA_TEST_REGISTRY_H
#define ASTHRA_TEST_REGISTRY_H

#include "../core/test_comprehensive.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST REGISTRY STRUCTURES
// =============================================================================

// Test function registry entry
typedef struct {
    const char *name;
    const char *category;
    AsthraTestResult (*test_func)(AsthraV12TestContext *ctx);
    bool is_performance_test;
    bool is_security_test;
} TestRegistryEntry;

// Test execution summary
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
    int error_tests;
    int skipped_tests;
    int performance_tests;
    int security_tests;
    double total_duration_seconds;
    double total_throughput;
    char **failure_messages;
    int failure_count;
} TestSummary;

// =============================================================================
// TEST REGISTRY FUNCTIONS
// =============================================================================

/**
 * Get the test registry array
 * @return Pointer to the test registry array
 */
const TestRegistryEntry* get_test_registry(void);

/**
 * Get the size of the test registry
 * @return Number of tests in the registry
 */
size_t get_test_registry_size(void);

/**
 * Initialize a test summary structure
 * @param summary Pointer to summary to initialize
 */
void test_summary_init(TestSummary *summary);

/**
 * Cleanup a test summary structure
 * @param summary Pointer to summary to cleanup
 */
void test_summary_cleanup(TestSummary *summary);

/**
 * Add a failure message to the test summary
 * @param summary Test summary to update
 * @param test_name Name of the failed test
 * @param message Failure message
 */
void test_summary_add_failure(TestSummary *summary, const char *test_name, const char *message);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_REGISTRY_H 
