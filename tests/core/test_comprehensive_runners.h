/**
 * Asthra Programming Language v1.2 Comprehensive Test Suite - Runners Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test runner function declarations.
 */

#ifndef ASTHRA_TEST_COMPREHENSIVE_RUNNERS_H
#define ASTHRA_TEST_COMPREHENSIVE_RUNNERS_H

#include "test_comprehensive_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST RUNNER FUNCTION DECLARATIONS
// =============================================================================

// Main test runners
AsthraTestResult run_v1_2_comprehensive_test_suite(void);
AsthraTestResult run_v1_2_category_tests(AsthraV12TestCategory category);
AsthraTestResult run_v1_2_performance_benchmarks(void);
AsthraTestResult run_v1_2_security_validation(void);

// =============================================================================
// INDIVIDUAL TEST FUNCTION DECLARATIONS
// =============================================================================
// Note: These would normally be declared in their respective test files,
// but are needed here for the runners

// Grammar tests
AsthraTestResult test_grammar_postfix_expr_disambiguation(AsthraV12TestContext *ctx);
AsthraTestResult test_grammar_precedence_rules(AsthraV12TestContext *ctx);
AsthraTestResult test_grammar_zero_parsing_conflicts(AsthraV12TestContext *ctx);

// Pattern matching tests
AsthraTestResult test_result_exhaustive_matching(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_matching_zero_cost(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_matching_benchmark(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_matching_optimization(AsthraV12TestContext *ctx);

// Security tests
AsthraTestResult test_security_constant_time_verification(AsthraV12TestContext *ctx);
AsthraTestResult test_security_memory_zeroing(AsthraV12TestContext *ctx);
AsthraTestResult test_security_csprng_functionality(AsthraV12TestContext *ctx);
AsthraTestResult test_security_side_channel_resistance(AsthraV12TestContext *ctx);
AsthraTestResult test_security_entropy_testing(AsthraV12TestContext *ctx);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_COMPREHENSIVE_RUNNERS_H 
