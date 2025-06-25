/**
 * Asthra Programming Language
 * Tuple Type Semantic Analysis Tests
 *
 * Tests for type checking tuple types, literals, and destructuring
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/parser_test_utils.h"
#include "../framework/semantic_test_utils.h"
#include "../framework/test_framework.h"
#include "ast_operations.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "semantic_errors.h"
#include "semantic_types.h"
#include "test_type_system_common.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// TEST HELPER FUNCTIONS
// =============================================================================

/**
 * Test that a source program passes semantic analysis
 */
static bool test_type_check_success(const char *source, const char *test_name) {
    SemanticAnalyzer *analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer for test: %s\n", test_name);
        return false;
    }

    ASTNode *ast = parse_test_source(source, test_name);
    if (!ast) {
        printf("Failed to parse source for test: %s\n", test_name);
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed for test: %s\n", test_name);
        SemanticError *error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", error->message, error->location.line,
                   error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

/**
 * Test that a source program produces a specific semantic error
 */
static bool test_type_check_error(const char *source, SemanticErrorCode expected_error,
                                  const char *test_name) {
    SemanticAnalyzer *analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer for test: %s\n", test_name);
        return false;
    }

    ASTNode *ast = parse_test_source(source, test_name);
    if (!ast) {
        printf("Failed to parse source for test: %s\n", test_name);
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool analysis_result = analyze_test_ast(analyzer, ast);
    bool has_expected_error = false;

    SemanticError *error = analyzer->errors;
    while (error) {
        if (error->code == expected_error) {
            has_expected_error = true;
            break;
        }
        error = error->next;
    }

    if (analysis_result && !has_expected_error) {
        printf("Expected error %d but analysis succeeded for test: %s\n", expected_error,
               test_name);
    } else if (!has_expected_error) {
        printf("Expected error %d but got different errors for test: %s\n", expected_error,
               test_name);
        error = analyzer->errors;
        while (error) {
            printf("  Got error %d: %s\n", error->code, error->message);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return has_expected_error;
}

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_tuple_type_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_tuple_type_tests(AsthraTestContext *context) {
    (void)context;
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TUPLE TYPE TESTS
// =============================================================================

/**
 * Test: Basic Tuple Type Checking
 * Verifies that tuple types are correctly type-checked
 */
static AsthraTestResult test_basic_tuple_types(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let pair: (i32, string) = (42, \"hello\");\n"
                         "    let triple: (i32, f64, bool) = (1, 3.14, true);\n"
                         "    return ();\n"
                         "}\n";

    if (!asthra_test_assert_bool(context, test_type_check_success(source, "basic_tuple_types"),
                                 "Basic tuple type declarations should pass type checking")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Tuple Type Mismatch
 * Verifies that tuple type mismatches are detected
 */
static AsthraTestResult test_tuple_type_mismatch(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let pair: (i32, string) = (\"wrong\", 42);\n"
                         "    return ();\n"
                         "}\n";

    if (!asthra_test_assert_bool(
            context,
            test_type_check_error(source, SEMANTIC_ERROR_TYPE_MISMATCH, "tuple_type_mismatch"),
            "Tuple type mismatch should produce type mismatch error")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Tuple Element Count Mismatch
 * Verifies that tuple element count mismatches are detected
 */
static AsthraTestResult test_tuple_element_count_mismatch(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let pair: (i32, string) = (42, \"hello\", true);\n"
                         "    return ();\n"
                         "}\n";

    if (!asthra_test_assert_bool(
            context,
            test_type_check_error(source, SEMANTIC_ERROR_TYPE_MISMATCH,
                                  "tuple_element_count_mismatch"),
            "Tuple element count mismatch should produce type mismatch error")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Tuple Destructuring
 * Verifies that tuple destructuring works correctly
 */
static AsthraTestResult test_tuple_destructuring(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let pair: (i32, string) = (42, \"hello\");\n"
                         "    let (x, y): (i32, string) = pair;\n"
                         "    return ();\n"
                         "}\n";

    if (!asthra_test_assert_bool(context, test_type_check_success(source, "tuple_destructuring"),
                                 "Tuple destructuring should pass type checking")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Nested Tuple Types
 * Verifies that nested tuple types work correctly
 */
static AsthraTestResult test_nested_tuple_types(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let nested: ((i32, i32), string) = ((1, 2), \"test\");\n"
                         "    let complex: (i32, (string, bool)) = (42, (\"hello\", true));\n"
                         "    return ();\n"
                         "}\n";

    if (!asthra_test_assert_bool(context, test_type_check_success(source, "nested_tuple_types"),
                                 "Nested tuple types should pass type checking")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Tuple Access
 * Verifies that tuple element access works correctly
 */
static AsthraTestResult test_tuple_access(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let pair: (i32, string) = (42, \"hello\");\n"
                         "    let first: i32 = pair.0;\n"
                         "    let second: string = pair.1;\n"
                         "    return ();\n"
                         "}\n";

    // This test will likely fail initially as tuple access syntax may not be implemented
    if (!asthra_test_assert_bool(context, test_type_check_success(source, "tuple_access"),
                                 "Tuple element access should pass type checking")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all tuple type checking tests
 */
AsthraTestSuite *create_tuple_type_checking_test_suite(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("Tuple Type Checking Tests",
                                                      "Tests for semantic analysis of tuple types");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_tuple_type_tests);
    asthra_test_suite_set_teardown(suite, teardown_tuple_type_tests);

    // Register tests
    asthra_test_suite_add_test(suite, "test_basic_tuple_types",
                               "Test basic tuple type declarations and literals",
                               test_basic_tuple_types);

    asthra_test_suite_add_test(suite, "test_tuple_type_mismatch",
                               "Test detection of tuple type mismatches", test_tuple_type_mismatch);

    asthra_test_suite_add_test(suite, "test_tuple_element_count_mismatch",
                               "Test detection of tuple element count mismatches",
                               test_tuple_element_count_mismatch);

    asthra_test_suite_add_test(suite, "test_tuple_destructuring",
                               "Test tuple destructuring in let statements",
                               test_tuple_destructuring);

    asthra_test_suite_add_test(suite, "test_nested_tuple_types", "Test nested tuple type support",
                               test_nested_tuple_types);

    asthra_test_suite_add_test(suite, "test_tuple_access",
                               "Test tuple element access with .0, .1 syntax", test_tuple_access);

    return suite;
}

// Main entry point for standalone execution
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    AsthraTestSuite *suite = create_tuple_type_checking_test_suite();

    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    return result;
}