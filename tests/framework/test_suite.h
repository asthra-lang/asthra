/**
 * Asthra Programming Language
 * Test Framework - Test Suite Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test suite configuration and management
 * Enhanced for Testing Framework Standardization Plan Phase 1
 */

#ifndef ASTHRA_TEST_SUITE_H
#define ASTHRA_TEST_SUITE_H

#include "test_statistics.h"
#include "test_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPES AND STRUCTURES
// =============================================================================

// Forward declaration for suite structure
typedef struct AsthraTestSuite AsthraTestSuite;

// Test reporting levels for different framework modes
typedef enum {
    ASTHRA_TEST_REPORTING_MINIMAL,  // Just pass/fail counts
    ASTHRA_TEST_REPORTING_STANDARD, // Standard test output
    ASTHRA_TEST_REPORTING_DETAILED, // Detailed with timing and stats
    ASTHRA_TEST_REPORTING_JSON      // Machine-readable JSON
} AsthraTestReportingLevel;

// Enhanced test suite configuration with new Phase 1 options
typedef struct {
    const char *name;
    const char *description;
    bool lightweight_mode;                    // NEW: Minimal overhead for simple tests
    bool custom_main;                         // NEW: Allow custom main() orchestration
    bool statistics_tracking;                 // NEW: Enable detailed statistics
    AsthraTestReportingLevel reporting_level; // NEW: Reporting level control
    uint64_t default_timeout_ns;              // Enhanced timeout configuration
    bool parallel_execution;                  // Enhanced parallel execution support
    bool stop_on_failure;
    bool verbose_output;
    bool json_output; // Preserved for backward compatibility
    size_t max_parallel_tests;
    AsthraTestStatistics *statistics;
} AsthraTestSuiteConfig;

// Analysis callback type for custom test analysis
typedef void (*AsthraAnalysisCallback)(AsthraTestSuite *suite, AsthraTestStatistics *stats);

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Enhanced test suite management with new modes
AsthraTestSuite *asthra_test_suite_create(const char *name, const char *description);
AsthraTestSuite *asthra_test_suite_create_lightweight(const char *name); // NEW: Lightweight mode
AsthraTestSuite *asthra_test_suite_create_detailed(const char *name,
                                                   const char *description); // NEW: Detailed mode

void asthra_test_suite_destroy(AsthraTestSuite *suite);
AsthraTestResult asthra_test_suite_run(AsthraTestSuite *suite);
int asthra_test_suite_run_and_exit(AsthraTestSuite *suite); // NEW: Convenience function

// Enhanced configuration
void asthra_test_suite_set_config(AsthraTestSuite *suite,
                                  const AsthraTestSuiteConfig *config); // NEW
void asthra_test_suite_set_analysis_callback(AsthraTestSuite *suite,
                                             AsthraAnalysisCallback callback); // NEW

// Test suite configuration
void asthra_test_suite_set_setup(AsthraTestSuite *suite, AsthraTestFunction setup_func);
void asthra_test_suite_set_teardown(AsthraTestSuite *suite, AsthraTestFunction teardown_func);

// Enhanced test management
void asthra_test_suite_add_test(AsthraTestSuite *suite, const char *test_name,
                                const char *description, AsthraTestFunction test_func);
void asthra_test_suite_add_test_with_data(AsthraTestSuite *suite, const char *test_name,
                                          const char *description, AsthraTestFunction test_func,
                                          void *user_data); // NEW: For adapter pattern

// Test suite configuration helpers
AsthraTestSuiteConfig asthra_test_suite_config_default(void);
AsthraTestSuiteConfig asthra_test_suite_config_create(const char *name, const char *description);
AsthraTestSuiteConfig asthra_test_suite_config_lightweight(const char *name); // NEW
AsthraTestSuiteConfig asthra_test_suite_config_detailed(const char *name,
                                                        const char *description); // NEW

// Test execution
AsthraTestResult asthra_test_run_single(AsthraTestFunction test_func,
                                        const AsthraTestMetadata *metadata,
                                        AsthraTestStatistics *stats);
AsthraTestResult asthra_test_run_suite(AsthraTestFunction *tests,
                                       const AsthraTestMetadata *metadata_array, size_t test_count,
                                       const AsthraTestSuiteConfig *config);

// Test execution and management
AsthraTestResult
asthra_test_run_suite_wrapper(AsthraTestSuite *suite); // Convenience wrapper for suite objects

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_SUITE_H
