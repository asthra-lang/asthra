/**
 * Expression Validation Tests - Method Calls
 *
 * Tests for method call validation and mutability checking.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_expression_validation_common.h"

// =============================================================================
// METHOD CALL VALIDATION TESTS
// =============================================================================

AsthraTestResult test_method_call_validation(AsthraTestContext *context) {
    // Valid method calls
    ASTHRA_TEST_ASSERT(context,
                       test_expression_success("package test;\n"
                                               "pub struct Point {\n"
                                               "    pub x: i32,\n"
                                               "    pub y: i32\n"
                                               "}\n"
                                               "impl Point {\n"
                                               "    pub fn distance(self) -> f64 {\n"
                                               "        return 0.0;\n"
                                               "    }\n"
                                               "}\n"
                                               "pub fn test(none) -> void {\n"
                                               "    let p: Point = Point { x: 0, y: 0 };\n"
                                               "    let d: f64 = p.distance();\n"
                                               "}\n",
                                               "valid_method_call"),
                       "Valid method call");

    // Invalid method call - mutability check
    ASTHRA_TEST_ASSERT(
        context,
        test_expression_error("package test;\n"
                              "pub struct Point {\n"
                              "    pub x: i32,\n"
                              "    pub y: i32\n"
                              "}\n"
                              "impl Point {\n"
                              "    pub fn translate(mut self, dx: i32, dy: i32) -> void {\n"
                              "        self.x = self.x + dx;\n"
                              "        self.y = self.y + dy;\n"
                              "    }\n"
                              "}\n"
                              "pub fn test(none) -> void {\n"
                              "    let p: Point = Point { x: 0, y: 0 };\n"
                              "    p.translate(1, 1);\n"
                              "}\n",
                              SEMANTIC_ERROR_IMMUTABLE_MODIFICATION, "mut_method_on_immutable"),
        "Mutable method on immutable variable should fail");

    return ASTHRA_TEST_PASS;
}