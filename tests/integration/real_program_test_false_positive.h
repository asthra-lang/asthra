/**
 * Real Program Test False Positive Detection Header
 *
 * Contains functions for detecting false positives, validating feature
 * functionality, and auditing test suites.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef REAL_PROGRAM_TEST_FALSE_POSITIVE_H
#define REAL_PROGRAM_TEST_FALSE_POSITIVE_H

#include "real_program_test_suite.h"
#include "real_program_test_utils.h"
#include <stdbool.h>
#include <stddef.h>

// =============================================================================
// TEST AUDIT STRUCTURES
// =============================================================================

typedef enum {
    TEST_TYPE_UNIT,
    TEST_TYPE_INTEGRATION,
    TEST_TYPE_END_TO_END,
    TEST_TYPE_MIXED
} TestType;

typedef struct {
    const char *category_name;
    size_t total_tests;
    size_t unit_tests;
    size_t integration_tests;
    size_t end_to_end_tests;
    size_t false_positive_risks;
    double coverage_score;
    const char *primary_gaps;
    TestType primary_type;
} TestAuditReport;

// =============================================================================
// FALSE POSITIVE DETECTION FUNCTIONS
// =============================================================================

/**
 * Detect false positive test results
 * @param feature_name The feature being tested
 * @param individual_result Result from individual test
 * @param integration_result Result from integration test
 * @return true if false positive detected
 */
bool detect_false_positive(const char *feature_name, RealProgramTestResult individual_result,
                           RealProgramTestResult integration_result);

/**
 * Validate feature functionality with real programs
 * @param feature_name The feature to validate
 * @param test_programs Array of test programs
 * @param program_count Number of programs
 * @param config Test configuration
 * @return true if feature functionality is validated
 */
bool validate_feature_functionality(const char *feature_name, const char **test_programs,
                                    size_t program_count, const TestSuiteConfig *config);

/**
 * Audit existing test suite for false positive risks
 * @return Audit report with analysis results
 */
TestAuditReport *audit_existing_test_suite(void);

#endif // REAL_PROGRAM_TEST_FALSE_POSITIVE_H