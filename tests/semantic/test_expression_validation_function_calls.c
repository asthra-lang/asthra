/**
 * Expression Validation Tests - Function Calls
 *
 * Tests for function call validation and argument checking.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_expression_validation_common.h"

// =============================================================================
// FUNCTION CALL VALIDATION TESTS
// =============================================================================

AsthraTestResult test_function_call_validation(AsthraTestContext *context) {
    // Valid function calls
    ASTHRA_TEST_ASSERT(
        context,
        test_expression_success("package test;\n"
                                "pub fn add(a: i32, b: i32) -> i32 { return a + b; }\n"
                                "pub fn test(none) -> void {\n"
                                "    let x: int = add(1, 2);\n"
                                "}\n",
                                "valid_function_call"),
        "Valid function call");

    // Invalid function calls - wrong number of arguments
    ASTHRA_TEST_ASSERT(context,
                       test_expression_error("package test;\n"
                                             "pub fn add(a: i32, b: i32) -> i32 { return a + b; }\n"
                                             "pub fn test(none) -> void {\n"
                                             "    let x: int = add(1);\n"
                                             "}\n",
                                             SEMANTIC_ERROR_INVALID_ARGUMENTS, "too_few_arguments"),
                       "Too few arguments should fail");

    ASTHRA_TEST_ASSERT(context,
                       test_expression_error("package test;\n"
                                             "pub fn add(a: i32, b: i32) -> i32 { return a + b; }\n"
                                             "pub fn test(none) -> void {\n"
                                             "    let x: int = add(1, 2, 3);\n"
                                             "}\n",
                                             SEMANTIC_ERROR_INVALID_ARGUMENTS,
                                             "too_many_arguments"),
                       "Too many arguments should fail");

    // Invalid function calls - type mismatch
    ASTHRA_TEST_ASSERT(context,
                       test_expression_error("package test;\n"
                                             "pub fn add(a: i32, b: i32) -> i32 { return a + b; }\n"
                                             "pub fn test(none) -> void {\n"
                                             "    let x: int = add(\"1\", \"2\");\n"
                                             "}\n",
                                             SEMANTIC_ERROR_INVALID_ARGUMENTS,
                                             "argument_type_mismatch"),
                       "Type mismatch in arguments should fail");

    // Calling non-function
    ASTHRA_TEST_ASSERT(context,
                       test_expression_error("package test;\n"
                                             "pub fn test(none) -> void {\n"
                                             "    let x: int = 5;\n"
                                             "    let y: int = x(1, 2);\n"
                                             "}\n",
                                             SEMANTIC_ERROR_NOT_CALLABLE, "call_non_function"),
                       "Calling non-function should fail");

    return ASTHRA_TEST_PASS;
}