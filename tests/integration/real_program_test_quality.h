/**
 * Real Program Test Quality Gates Header
 *
 * Contains functions for validating quality gates, assessing feature quality,
 * and ensuring comprehensive testing standards.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef REAL_PROGRAM_TEST_QUALITY_H
#define REAL_PROGRAM_TEST_QUALITY_H

#include "real_program_test_suite.h"
#include <stdbool.h>
#include <stddef.h>

// =============================================================================
// QUALITY GATE STRUCTURES
// =============================================================================

typedef struct {
    bool grammar_compliant;
    bool end_to_end_functional;
    bool performance_acceptable;
    bool error_handling_correct;
    bool feature_coverage_complete;
    double overall_score;
    const char *failure_reasons[10];
    size_t failure_count;
} QualityGateResult;

// =============================================================================
// QUALITY GATE FUNCTIONS
// =============================================================================

/**
 * Validate quality gates for a set of test programs
 * @param feature_name The name of the feature being tested
 * @param test_programs Array of test program source code
 * @param program_count Number of test programs
 * @return Quality gate validation results
 */
QualityGateResult validate_quality_gates(const char *feature_name, const char **test_programs,
                                         size_t program_count);

/**
 * Assess the quality of a specific feature implementation
 * @param feature_name The name of the feature to assess
 * @return Quality assessment results
 */
QualityGateResult assess_feature_quality(const char *feature_name);

/**
 * Validate that real functionality is working correctly
 * @param feature_name The feature to validate
 * @param real_world_programs Array of real-world test programs
 * @param program_count Number of programs
 * @return true if real functionality is validated
 */
bool validate_real_functionality(const char *feature_name, const char **real_world_programs,
                                 size_t program_count);

/**
 * Generate coverage analysis for a test suite
 * @param suite The test suite to analyze
 * @param config Test configuration
 */
void generate_coverage_analysis(const RealProgramTestSuite *suite, const TestSuiteConfig *config);

#endif // REAL_PROGRAM_TEST_QUALITY_H