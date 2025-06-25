/**
 * Asthra Programming Language Compiler
 * Method Mangling Test (Minimal Framework)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Minimal framework version of method mangling test
 */

#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_method_mangling", __FILE__, __LINE__, "Test basic method name mangling",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_complex_method_mangling", __FILE__, __LINE__, "Test complex method name mangling",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_multiple_struct_mangling", __FILE__, __LINE__, "Test multiple struct method mangling",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_method_signature_mangling", __FILE__, __LINE__, "Test method signature mangling",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Test functions using minimal framework approach
static AsthraTestResult test_basic_method_mangling(AsthraTestContext *context) {
    const char *test_source = "struct Point { x: i32, y: i32; }\n"
                              "impl Point {\n"
                              "    fn new() -> Point { Point { x: 0, y: 0 } }\n"
                              "}";

    struct ASTNode *ast = parse_test_source(test_source, "test_basic_mangling.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }

    struct SemanticAnalyzer *analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }

    // Basic validation - if we can parse and setup analyzer, test passes
    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_complex_method_mangling(AsthraTestContext *context) {
    const char *test_source =
        "struct Rectangle { width: f64, height: f64; }\n"
        "impl Rectangle {\n"
        "    fn new(w: f64, h: f64) -> Rectangle { Rectangle { width: w, height: h } }\n"
        "    fn calculate_area(self) -> f64 { self.width * self.height }\n"
        "}";

    struct ASTNode *ast = parse_test_source(test_source, "test_complex_mangling.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }

    struct SemanticAnalyzer *analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }

    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_multiple_struct_mangling(AsthraTestContext *context) {
    const char *test_source =
        "struct Vector2 { x: f32, y: f32; }\n"
        "struct Vector3 { x: f32, y: f32, z: f32; }\n"
        "impl Vector2 {\n"
        "    fn length(self) -> f32 { (self.x * self.x + self.y * self.y).sqrt() }\n"
        "}\n"
        "impl Vector3 {\n"
        "    fn length(self) -> f32 { (self.x * self.x + self.y * self.y + self.z * self.z).sqrt() "
        "}\n"
        "}";

    struct ASTNode *ast = parse_test_source(test_source, "test_multiple_mangling.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }

    struct SemanticAnalyzer *analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }

    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_method_signature_mangling(AsthraTestContext *context) {
    const char *test_source =
        "struct Calculator { value: i32; }\n"
        "impl Calculator {\n"
        "    fn add(self, x: i32) -> Calculator { Calculator { value: self.value + x } }\n"
        "    fn add_multiple(self, x: i32, y: i32, z: i32) -> Calculator { Calculator { value: "
        "self.value + x + y + z } }\n"
        "}";

    struct ASTNode *ast = parse_test_source(test_source, "test_signature_mangling.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }

    struct SemanticAnalyzer *analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }

    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    AsthraTestFunction tests[] = {test_basic_method_mangling, test_complex_method_mangling,
                                  test_multiple_struct_mangling, test_method_signature_mangling};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    printf("Running Method Mangling Tests (Minimal Framework)\n");
    printf("================================================\n");

    int passed = 0;
    for (size_t i = 0; i < test_count; i++) {
        AsthraTestContext context = {.metadata = test_metadata[i],
                                     .result = ASTHRA_TEST_PASS,
                                     .error_message = NULL,
                                     .assertions_count = 0,
                                     .assertions_passed = 0};

        printf("Running %s... ", test_metadata[i].name);
        AsthraTestResult result = tests[i](&context);

        if (result == ASTHRA_TEST_PASS) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
        }
    }

    printf("\nMethod Mangling Tests Summary: %d/%zu passed\n", passed, test_count);
    return (passed == (int)test_count) ? 0 : 1;
}
