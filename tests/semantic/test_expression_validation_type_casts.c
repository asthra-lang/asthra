/**
 * Expression Validation Tests - Type Casts
 * 
 * Tests for type casting validation and conversion checking.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_expression_validation_common.h"

// =============================================================================
// TYPE CAST VALIDATION TESTS
// =============================================================================

AsthraTestResult test_type_cast_validation(AsthraTestContext* context) {
    // Valid type casts
    ASTHRA_TEST_ASSERT(context, test_expression_success(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let a: f64 = f64(5);\n"
        "    let b: i32 = i32(3.14);\n"
        "}\n",
        "valid_numeric_casts"
    ), "Valid numeric type casts");
    
    // Invalid type casts
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let c: i32 = i32(\"hello\");\n"
        "}\n",
        SEMANTIC_ERROR_INVALID_OPERATION,
        "string_to_int_cast"
    ), "String to int cast should fail");
    
    ASTHRA_TEST_ASSERT(context, test_expression_error(
        "package test;\n"
        "pub fn test(none) -> void {\n"
        "    let d: string = string(true);\n"
        "}\n",
        SEMANTIC_ERROR_INVALID_OPERATION,
        "bool_to_string_cast"
    ), "Bool to string cast should fail");
    
    return ASTHRA_TEST_PASS;
}