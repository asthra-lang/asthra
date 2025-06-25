/**
 * Asthra Programming Language
 * Option Syntax Semantic Analysis Tests
 *
 * Tests to ensure standalone Some() and None() fail during semantic analysis
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../../src/analysis/semantic_analyzer.h"
#include "../../src/parser/parser.h"
#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST HELPERS
// =============================================================================

static bool test_semantic_analysis_fails(const char *source) {
    Parser *parser = create_test_parser(source);
    if (!parser)
        return true;

    ASTNode *ast = parser_parse_program(parser);
    if (!ast) {
        destroy_test_parser(parser);
        return true;
    }

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        ast_free_node(ast);
        destroy_test_parser(parser);
        return true;
    }

    bool success = semantic_analyze_program(analyzer, ast);

    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    destroy_test_parser(parser);

    return !success; // Return true if semantic analysis failed
}

static bool test_semantic_analysis_succeeds(const char *source) {
    return !test_semantic_analysis_fails(source);
}

// =============================================================================
// OPTION SEMANTIC TESTS
// =============================================================================

/**
 * Test: Option.Some semantic analysis succeeds
 */
static AsthraTestResult test_option_some_semantic_success(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub fn test_correct(none) -> void {\n"
                         "    let opt: Option<i32> = Option.Some(42);\n"
                         "    return ();\n"
                         "}\n";

    if (!asthra_test_assert_bool_eq(context, test_semantic_analysis_succeeds(source), true,
                                    "Option.Some(value) should pass semantic analysis")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Option.None semantic analysis succeeds
 */
static AsthraTestResult test_option_none_semantic_success(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub fn test_correct(none) -> void {\n"
                         "    let opt: Option<i32> = Option.None;\n"
                         "    return ();\n"
                         "}\n";

    if (!asthra_test_assert_bool_eq(context, test_semantic_analysis_succeeds(source), true,
                                    "Option.None should pass semantic analysis")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Standalone Some() fails semantic analysis
 */
static AsthraTestResult test_standalone_some_semantic_failure(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub fn test_invalid(none) -> void {\n"
                         "    let opt: Option<i32> = Some(42);\n" // Some is not a function
                         "    return ();\n"
                         "}\n";

    if (!asthra_test_assert_bool_eq(context, test_semantic_analysis_fails(source), true,
                                    "Standalone Some() should fail semantic analysis")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Standalone None() fails semantic analysis
 */
static AsthraTestResult test_standalone_none_semantic_failure(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub fn test_invalid(none) -> void {\n"
                         "    let opt: Option<i32> = None();\n" // None is not a function
                         "    return ();\n"
                         "}\n";

    if (!asthra_test_assert_bool_eq(context, test_semantic_analysis_fails(source), true,
                                    "Standalone None() should fail semantic analysis")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Cannot define functions named Some or None
 */
static AsthraTestResult test_cannot_define_some_none_functions(AsthraTestContext *context) {
    // Test defining a function named Some
    const char *source1 = "package test;\n"
                          "pub fn Some(value: i32) -> Option<i32> {\n"
                          "    return Option.Some(value);\n"
                          "}\n"
                          "pub fn test(none) -> void {\n"
                          "    let opt: Option<i32> = Some(42);\n" // This would call the function
                          "    return ();\n"
                          "}\n";

    // This should succeed - Some is not a reserved name
    if (!asthra_test_assert_bool_eq(context, test_semantic_analysis_succeeds(source1), true,
                                    "Can define function named Some (not reserved)")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test defining a function named None
    const char *source2 = "package test;\n"
                          "pub fn None(none) -> Option<i32> {\n"
                          "    return Option.None;\n"
                          "}\n"
                          "pub fn test(none) -> void {\n"
                          "    let opt: Option<i32> = None();\n" // This would call the function
                          "    return ();\n"
                          "}\n";

    // This should succeed - None is not a reserved name
    if (!asthra_test_assert_bool_eq(context, test_semantic_analysis_succeeds(source2), true,
                                    "Can define function named None (not reserved)")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_option_semantic_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_option_semantic_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

AsthraTestSuite *create_option_syntax_semantic_test_suite(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("Option Syntax Semantic Tests",
                                                      "Semantic analysis of Option syntax");

    if (!suite)
        return NULL;

    asthra_test_suite_set_setup(suite, setup_option_semantic_tests);
    asthra_test_suite_set_teardown(suite, teardown_option_semantic_tests);

    asthra_test_suite_add_test(suite, "test_option_some_semantic_success",
                               "Option.Some passes semantic analysis",
                               test_option_some_semantic_success);

    asthra_test_suite_add_test(suite, "test_option_none_semantic_success",
                               "Option.None passes semantic analysis",
                               test_option_none_semantic_success);

    asthra_test_suite_add_test(suite, "test_standalone_some_semantic_failure",
                               "Standalone Some() fails semantic analysis",
                               test_standalone_some_semantic_failure);

    asthra_test_suite_add_test(suite, "test_standalone_none_semantic_failure",
                               "Standalone None() fails semantic analysis",
                               test_standalone_none_semantic_failure);

    asthra_test_suite_add_test(suite, "test_cannot_define_some_none_functions",
                               "Can define functions named Some/None",
                               test_cannot_define_some_none_functions);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Option Syntax Semantic Tests ===\n\n");

    AsthraTestSuite *suite = create_option_syntax_semantic_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif