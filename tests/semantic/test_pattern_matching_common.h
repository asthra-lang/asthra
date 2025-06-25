/**
 * Asthra Programming Language
 * Pattern Matching Tests - Common Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef PATTERN_MATCHING_TEST_COMMON_H
#define PATTERN_MATCHING_TEST_COMMON_H

#include "../framework/semantic_test_utils.h"
#include "../framework/test_framework.h"
#include "ast_operations.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "semantic_errors.h"
#include "semantic_types.h"
#include "test_type_system_common.h"
#include <string.h>

// =============================================================================
// TEST FUNCTION DECLARATIONS
// =============================================================================

// Test helper functions
bool test_pattern_success(const char *source, const char *test_name);
bool test_pattern_error(const char *source, SemanticErrorCode expected_error,
                        const char *test_name);

// Test suite functions
AsthraTestResult test_basic_enum_pattern_matching(AsthraTestContext *context);
AsthraTestResult test_struct_pattern_destructuring(AsthraTestContext *context);
AsthraTestResult test_nested_pattern_matching(AsthraTestContext *context);
AsthraTestResult test_guard_conditions(AsthraTestContext *context);
AsthraTestResult test_if_let_statements(AsthraTestContext *context);
AsthraTestResult test_pattern_variable_binding(AsthraTestContext *context);
AsthraTestResult test_wildcard_and_literal_patterns(AsthraTestContext *context);
AsthraTestResult test_exhaustiveness_complex_cases(AsthraTestContext *context);
AsthraTestResult test_type_compatibility_in_patterns(AsthraTestContext *context);
AsthraTestResult test_match_expression_vs_statement(AsthraTestContext *context);

#endif // PATTERN_MATCHING_TEST_COMMON_H