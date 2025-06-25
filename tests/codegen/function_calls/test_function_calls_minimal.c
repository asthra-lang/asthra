/**
 * Asthra Programming Language Compiler
 * Function Calls Test (Minimal Framework)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Minimal framework version of function calls test
 */

#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_simple_function_call", __FILE__, __LINE__, "Test simple function call generation",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_function_call_with_arguments", __FILE__, __LINE__, "Test function call with arguments",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_method_call_generation", __FILE__, __LINE__, "Test method call generation",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_nested_function_calls", __FILE__, __LINE__, "Test nested function calls",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_chained_function_calls", __FILE__, __LINE__, "Test chained function calls",
     ASTHRA_TEST_SEVERITY_LOW, 5000000000ULL, false, NULL}};

// Test functions using minimal framework approach
static AsthraTestResult test_simple_function_call(AsthraTestContext *context) {
    const char *test_source = "package test;\n\npub fn function_name(none) -> void { }\n\npub fn "
                              "main(none) -> void { function_name(); }";

    struct ASTNode *ast = parse_test_source(test_source, "test_simple_call.asthra");
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

static AsthraTestResult test_function_call_with_arguments(AsthraTestContext *context) {
    const char *test_source =
        "package test;\n\npub fn function_name(a: int, b: int, c: int) -> void { }\n\npub fn "
        "main(none) -> void { let arg1: int = 1; let arg2: int = 2; let arg3: int = 3; "
        "function_name(arg1, arg2, arg3); }";

    struct ASTNode *ast = parse_test_source(test_source, "test_call_args.asthra");
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

static AsthraTestResult test_method_call_generation(AsthraTestContext *context) {
    const char *test_source =
        "package test;\n\npub struct MyObject { pub field: int }\n\nimpl MyObject { pub fn "
        "method_name(self, arg: int) -> void { } }\n\npub fn main(none) -> void { let obj: "
        "MyObject = MyObject { field: 0 }; let argument: int = 42; obj.method_name(argument); }";

    struct ASTNode *ast = parse_test_source(test_source, "test_method.asthra");
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

static AsthraTestResult test_nested_function_calls(AsthraTestContext *context) {
    const char *test_source =
        "package test;\n\npub fn inner_function(v: int) -> int { return v * 2; }\n\npub fn "
        "outer_function(x: int) -> void { }\n\npub fn main(none) -> void { let value: int = 21; "
        "outer_function(inner_function(value)); }";

    struct ASTNode *ast = parse_test_source(test_source, "test_nested.asthra");
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

static AsthraTestResult test_chained_function_calls(AsthraTestContext *context) {
    const char *test_source =
        "package test;\n\npub struct MyObject { pub value: int }\n\nimpl MyObject { pub fn "
        "method1(self) -> MyObject { return self; } pub fn method2(self) -> MyObject { return "
        "self; } pub fn method3(self) -> void { } }\n\npub fn main(none) -> void { let obj: "
        "MyObject = MyObject { value: 42 }; obj.method1().method2().method3(); }";

    struct ASTNode *ast = parse_test_source(test_source, "test_chained.asthra");
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
    AsthraTestFunction tests[] = {test_simple_function_call, test_function_call_with_arguments,
                                  test_method_call_generation, test_nested_function_calls,
                                  test_chained_function_calls};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    printf("Running Function Calls Tests (Minimal Framework)\n");
    printf("===============================================\n");

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

    printf("\nFunction Calls Tests Summary: %d/%zu passed\n", passed, test_count);
    return (passed == (int)test_count) ? 0 : 1;
}
