/**
 * Expression Validation Tests - Unary Operators
 * 
 * Tests for unary operator type checking and validation.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_expression_validation_common.h"

// =============================================================================
// UNARY OPERATOR VALIDATION TESTS
// =============================================================================

AsthraTestResult test_unary_operators(AsthraTestContext* context) {
    // Valid unary operations
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: int = -5;\n"
        "    let b: f64 = -5.0;\n"
        "    let c: int = +10;\n"
        "}\n",
        "valid_numeric_unary"
    ), "Valid numeric unary operations");
    
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: bool = !true;\n"
        "    let b: bool = !false;\n"
        "    let c: bool = !(5 > 3);\n"
        "}\n",
        "valid_logical_not"
    ), "Valid logical not operations");
    
    // Invalid unary operations
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: string = -\"hello\";\n"
        "}\n",
        SEMANTIC_ERROR_INVALID_OPERATION,
        "negate_string_error"
    ), "Negate string should fail");
    
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: bool = !0;\n"
        "}\n",
        SEMANTIC_ERROR_INVALID_OPERATION,
        "not_int_error"
    ), "Logical not on int should fail");
    
    return ASTHRA_TEST_PASS;
}