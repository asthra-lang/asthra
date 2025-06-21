/**
 * Expression Validation Tests - Common Header
 * 
 * Shared definitions and function declarations for expression validation test modules.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef EXPRESSION_VALIDATION_COMMON_H
#define EXPRESSION_VALIDATION_COMMON_H

#include "test_type_system_common.h"
#include "semantic_analyzer.h"
#include "semantic_errors.h"
#include "semantic_types.h"
#include "parser.h"
#include "ast_operations.h"
#include "../framework/test_framework.h"
#include "../framework/semantic_test_utils.h"
#include <string.h>

// =============================================================================
// SHARED TEST HELPER FUNCTIONS
// =============================================================================

/**
 * Test that an expression validates successfully
 */
bool test_expression_success(const char* source, const char* test_name);

/**
 * Test that an expression produces a specific semantic error
 */
bool test_expression_error(const char* source, SemanticErrorCode expected_error, const char* test_name);

// =============================================================================
// TEST FUNCTION DECLARATIONS
// =============================================================================

// Binary operator tests
AsthraTestResult test_binary_arithmetic_operators(AsthraTestContext* context);
AsthraTestResult test_binary_comparison_operators(AsthraTestContext* context);
AsthraTestResult test_binary_logical_operators(AsthraTestContext* context);

// Unary operator tests
AsthraTestResult test_unary_operators(AsthraTestContext* context);

// Function call tests
AsthraTestResult test_function_call_validation(AsthraTestContext* context);

// Field access tests
AsthraTestResult test_field_access_validation(AsthraTestContext* context);

// Array/slice operation tests
AsthraTestResult test_array_slice_operations(AsthraTestContext* context);

// Method call tests
AsthraTestResult test_method_call_validation(AsthraTestContext* context);

// Type cast tests
AsthraTestResult test_type_cast_validation(AsthraTestContext* context);

// Complex expression tests
AsthraTestResult test_complex_expression_trees(AsthraTestContext* context);

#endif // EXPRESSION_VALIDATION_COMMON_H