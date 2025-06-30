/**
 * Asthra Programming Language
 * Option Type Integration Tests
 *
 * Tests for complete Option<T> usage pipeline from parsing to code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../../src/analysis/semantic_analyzer.h"
#include "../../src/compiler/pipeline_orchestrator.h"
#include "../../src/parser/parser.h"
#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST HELPERS
// =============================================================================

static bool compile_option_program(const char *source, char *assembly_output, size_t output_size) {
    // Create parser
    Parser *parser = create_test_parser(source);
    if (!parser)
        return false;

    // Parse program
    ASTNode *ast = parser_parse_program(parser);
    if (!ast) {
        destroy_test_parser(parser);
        return false;
    }

    // Semantic analysis
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return false;
    }

    bool semantic_success = semantic_analyze_program(analyzer, ast);
    if (!semantic_success) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        return false;
    }

    // Code generation
    AsthraBackend *backend = asthra_backend_create_by_type(ASTHRA_BACKEND_LLVM_IR);
    if (!backend) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        return false;
    }

    bool codegen_success = asthra_backend_generate_program(backend, ast);
    if (!codegen_success) {
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(ast);
        destroy_test_parser(parser);
        return false;
    }

    // Generate assembly
    bool assembly_success = false;
    if (assembly_output && output_size > 0) {
        assembly_success = asthra_backend_emit_assembly(backend, assembly_output, output_size);
    }

    // Cleanup
    asthra_backend_destroy(backend);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    destroy_test_parser(parser);

    return codegen_success && (assembly_output ? assembly_success : true);
}

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

/**
 * Test: Basic Option Usage Integration
 * Tests parsing, semantic analysis, and code generation for Option types
 */
static AsthraTestResult test_option_basic_integration(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn test_option_some(none) -> Option<i32> {\n"
                         "    return Option.Some(42);\n"
                         "}\n"
                         "\n"
                         "pub fn test_option_none(none) -> Option<i32> {\n"
                         "    return Option.None;\n"
                         "}\n"
                         "\n"
                         "pub fn test_option_usage(none) -> void {\n"
                         "    let some_val: Option<i32> = Option.Some(123);\n"
                         "    let none_val: Option<i32> = Option.None;\n"
                         "    let string_opt: Option<string> = Option.Some(\"hello\");\n"
                         "    return ();\n"
                         "}\n";

    char assembly[8192];
    bool success = compile_option_program(source, assembly, sizeof(assembly));

    if (!asthra_test_assert_bool_eq(context, success, true,
                                    "Option basic integration should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify that functions are in assembly
    if (!asthra_test_assert_bool_eq(context, strstr(assembly, "test_option_some") != NULL, true,
                                    "Assembly should contain test_option_some function")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool_eq(context, strstr(assembly, "test_option_none") != NULL, true,
                                    "Assembly should contain test_option_none function")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Option in Struct Integration
 * Tests Option as a struct field through the complete pipeline
 */
static AsthraTestResult test_option_struct_integration(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub struct Config {\n"
                         "    pub timeout: Option<i32>,\n"
                         "    pub name: Option<string>\n"
                         "}\n"
                         "\n"
                         "pub fn create_config(none) -> Config {\n"
                         "    let config: Config;\n"
                         "    // TODO: Initialize fields\n"
                         "    return config;\n"
                         "}\n";

    char assembly[8192];
    bool success = compile_option_program(source, assembly, sizeof(assembly));

    if (!asthra_test_assert_bool_eq(context, success, true,
                                    "Option struct integration should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify struct and function are in assembly
    if (!asthra_test_assert_bool_eq(context, strstr(assembly, "create_config") != NULL, true,
                                    "Assembly should contain create_config function")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Nested Option Integration
 * Tests Option<Option<T>> through the complete pipeline
 */
static AsthraTestResult test_nested_option_integration(AsthraTestContext *context) {
    const char *source =
        "package test;\n"
        "\n"
        "pub fn test_nested_option(none) -> void {\n"
        "    let nested: Option<Option<i32>>;\n"
        "    // TODO: Enable when nested generic inference works\n"
        "    // let some_nested: Option<Option<i32>> = Option.Some(Option.Some(42));\n"
        "    // let none_nested: Option<Option<i32>> = Option.None;\n"
        "    return ();\n"
        "}\n";

    char assembly[8192];
    bool success = compile_option_program(source, assembly, sizeof(assembly));

    if (!asthra_test_assert_bool_eq(context, success, true,
                                    "Nested Option integration should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Option with Different Types Integration
 * Tests Option with various inner types
 */
static AsthraTestResult test_option_multiple_types_integration(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn test_option_types(none) -> void {\n"
                         "    let int_opt: Option<i32> = Option.Some(42);\n"
                         "    let string_opt: Option<string> = Option.Some(\"hello\");\n"
                         "    let bool_opt: Option<bool> = Option.Some(true);\n"
                         "    let none_int: Option<i32> = Option.None;\n"
                         "    let none_string: Option<string> = Option.None;\n"
                         "    return ();\n"
                         "}\n";

    char assembly[8192];
    bool success = compile_option_program(source, assembly, sizeof(assembly));

    if (!asthra_test_assert_bool_eq(context, success, true,
                                    "Multiple Option types integration should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Option Pattern Matching Integration
 * Tests pattern matching on Option types (if-let)
 */
static AsthraTestResult test_option_pattern_matching_integration(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn extract_value(opt: Option<i32>) -> i32 {\n"
                         "    // TODO: Use pattern matching when available\n"
                         "    // if let Some(value) = opt {\n"
                         "    //     return value;\n"
                         "    // } else {\n"
                         "    //     return 0;\n"
                         "    // }\n"
                         "    return 42;\n"
                         "}\n";

    char assembly[8192];
    bool success = compile_option_program(source, assembly, sizeof(assembly));

    if (!asthra_test_assert_bool_eq(context, success, true,
                                    "Option pattern matching integration should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_option_integration_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_option_integration_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

AsthraTestSuite *create_option_integration_test_suite(void) {
    AsthraTestSuite *suite =
        asthra_test_suite_create("Option Integration Tests", "Complete Option<T> pipeline testing");

    if (!suite)
        return NULL;

    asthra_test_suite_set_setup(suite, setup_option_integration_tests);
    asthra_test_suite_set_teardown(suite, teardown_option_integration_tests);

    asthra_test_suite_add_test(suite, "test_option_basic_integration",
                               "Basic Option usage integration", test_option_basic_integration);

    asthra_test_suite_add_test(suite, "test_option_struct_integration",
                               "Option in struct integration", test_option_struct_integration);

    asthra_test_suite_add_test(suite, "test_nested_option_integration", "Nested Option integration",
                               test_nested_option_integration);

    asthra_test_suite_add_test(suite, "test_option_multiple_types_integration",
                               "Option with different types integration",
                               test_option_multiple_types_integration);

    asthra_test_suite_add_test(suite, "test_option_pattern_matching_integration",
                               "Option pattern matching integration",
                               test_option_pattern_matching_integration);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Option Integration Tests ===\n\n");

    AsthraTestSuite *suite = create_option_integration_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif