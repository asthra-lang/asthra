/**
 * Asthra Programming Language - Enhanced Test Suite Runner
 * Header file for comprehensive test suite execution
 *
 * Phase 4: Test Framework Enhancement
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_TEST_SUITE_RUNNER_H
#define ASTHRA_TEST_SUITE_RUNNER_H

#include "test_formatters.h"
#include "test_framework.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct AsthraTestRunner AsthraTestRunner;

// =============================================================================
// TEST RUNNER CREATION AND MANAGEMENT
// =============================================================================

/**
 * Create a new test runner instance
 */
AsthraTestRunner *asthra_test_runner_create(void);

/**
 * Destroy test runner and free all resources
 */
void asthra_test_runner_destroy(AsthraTestRunner *runner);

/**
 * Add a test suite to the runner
 */
int asthra_test_runner_add_suite(AsthraTestRunner *runner, const char *suite_name,
                                 AsthraTestFunction *tests,
                                 const AsthraTestMetadata *metadata_array, size_t test_count);

/**
 * Execute all registered test suites
 */
int asthra_test_runner_execute(AsthraTestRunner *runner);

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

/**
 * Main entry point for test suite runner with command line parsing
 */
int test_suite_runner_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_SUITE_RUNNER_H