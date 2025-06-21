/**
 * Expression Validation Tests - Binary Operators
 * 
 * Tests for binary operator type checking and validation.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_expression_validation_common.h"

// =============================================================================
// BINARY OPERATOR TYPE CHECKING TESTS
// =============================================================================

AsthraTestResult test_binary_arithmetic_operators(AsthraTestContext* context) {
    // Valid arithmetic operations
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: int = 5 + 3;\n"
        "    let b: int = 10 - 2;\n"
        "    let c: int = 4 * 3;\n"
        "    let d: int = 12 / 3;\n"
        "    let e: int = 10 % 3;\n"
        "}\n",
        "valid_int_arithmetic"
    ), "Valid integer arithmetic operations");
    
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: float = 5.0 + 3.0;\n"
        "    let b: float = 10.0 - 2.5;\n"
        "    let c: float = 4.0 * 3.0;\n"
        "    let d: float = 12.0 / 3.0;\n"
        "}\n",
        "valid_float_arithmetic"
    ), "Valid float arithmetic operations");
    
    // Invalid arithmetic operations
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: string = \"hello\" + 5;\n"
        "}\n",
        SEMANTIC_ERROR_INCOMPATIBLE_TYPES,
        "string_plus_int_error"
    ), "String + int should fail");
    
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: bool = true + false;\n"
        "}\n",
        SEMANTIC_ERROR_INCOMPATIBLE_TYPES,
        "bool_arithmetic_error"
    ), "Bool arithmetic should fail");
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_binary_comparison_operators(AsthraTestContext* context) {
    // Valid comparisons
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: bool = 5 < 10;\n"
        "    let b: bool = 10 > 5;\n"
        "    let c: bool = 5 <= 5;\n"
        "    let d: bool = 10 >= 10;\n"
        "    let e: bool = 5 == 5;\n"
        "    let f: bool = 5 != 10;\n"
        "}\n",
        "valid_int_comparison"
    ), "Valid integer comparisons");
    
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: bool = \"apple\" < \"banana\";\n"
        "    let b: bool = \"hello\" == \"hello\";\n"
        "}\n",
        "valid_string_comparison"
    ), "Valid string comparisons");
    
    // Invalid comparisons
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: bool = 5 < \"10\";\n"
        "}\n",
        SEMANTIC_ERROR_INCOMPATIBLE_TYPES,
        "int_string_comparison_error"
    ), "Int < string should fail");
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_binary_logical_operators(AsthraTestContext* context) {
    // Valid logical operations
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: bool = true && false;\n"
        "    let b: bool = true || false;\n"
        "    let c: bool = (5 > 3) && (10 < 20);\n"
        "}\n",
        "valid_logical_operations"
    ), "Valid logical operations");
    
    // Invalid logical operations
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: bool = 5 && true;\n"
        "}\n",
        SEMANTIC_ERROR_INCOMPATIBLE_TYPES,
        "int_logical_and_error"
    ), "Int && bool should fail");
    
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: bool = \"hello\" || false;\n"
        "}\n",
        SEMANTIC_ERROR_INCOMPATIBLE_TYPES,
        "string_logical_or_error"
    ), "String || bool should fail");
    
    return ASTHRA_TEST_PASS;
}