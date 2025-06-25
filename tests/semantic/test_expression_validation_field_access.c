/**
 * Expression Validation Tests - Field Access
 *
 * Tests for struct field access validation and error checking.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_expression_validation_common.h"

// =============================================================================
// FIELD ACCESS VALIDATION TESTS
// =============================================================================

AsthraTestResult test_field_access_validation(AsthraTestContext *context) {
    // Valid field access
    ASTHRA_TEST_ASSERT(context,
                       test_expression_success("package test;\n"
                                               "pub struct Point {\n"
                                               "    pub x: i32,\n"
                                               "    pub y: i32\n"
                                               "}\n"
                                               "pub fn test(none) -> void {\n"
                                               "    let p: Point = Point { x: 1, y: 2 };\n"
                                               "    let x_val: i32 = p.x;\n"
                                               "    let y_val: i32 = p.y;\n"
                                               "}\n",
                                               "valid_field_access"),
                       "Valid field access");

    // Invalid field access - no such field
    ASTHRA_TEST_ASSERT(context,
                       test_expression_error("package test;\n"
                                             "pub struct Point {\n"
                                             "    pub x: i32,\n"
                                             "    pub y: i32\n"
                                             "}\n"
                                             "pub fn test(none) -> void {\n"
                                             "    let p: Point = Point { x: 1, y: 2 };\n"
                                             "    let z_val: i32 = p.z;\n"
                                             "}\n",
                                             SEMANTIC_ERROR_INVALID_STRUCT_FIELD, "no_such_field"),
                       "Access to non-existent field should fail");

    // Invalid field access - field access on non-struct
    ASTHRA_TEST_ASSERT(context,
                       test_expression_error("package test;\n"
                                             "pub fn test(none) -> void {\n"
                                             "    let x: int = 5;\n"
                                             "    let y: int = x.field;\n"
                                             "}\n",
                                             SEMANTIC_ERROR_INVALID_OPERATION,
                                             "field_access_on_non_struct"),
                       "Field access on non-struct should fail");

    // Nested field access
    ASTHRA_TEST_ASSERT(
        context,
        test_expression_success("package test;\n"
                                "pub struct Inner {\n"
                                "    pub value: i32\n"
                                "}\n"
                                "pub struct Outer {\n"
                                "    pub inner: Inner\n"
                                "}\n"
                                "pub fn test(none) -> void {\n"
                                "    let o: Outer = Outer { inner: Inner { value: 42 } };\n"
                                "    let v: i32 = o.inner.value;\n"
                                "}\n",
                                "valid_nested_field_access"),
        "Valid nested field access");

    // Invalid nested field access
    ASTHRA_TEST_ASSERT(context,
                       test_expression_error("package test;\n"
                                             "pub struct Point {\n"
                                             "    pub x: i32,\n"
                                             "    pub y: i32\n"
                                             "}\n"
                                             "pub fn test(none) -> void {\n"
                                             "    let p: Point = Point { x: 1, y: 2 };\n"
                                             "    let bad: int = p.x.y;\n"
                                             "}\n",
                                             SEMANTIC_ERROR_INVALID_OPERATION,
                                             "field_access_on_int"),
                       "Field access on int should fail");

    return ASTHRA_TEST_PASS;
}