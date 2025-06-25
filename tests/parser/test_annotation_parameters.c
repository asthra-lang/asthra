/**
 * Asthra Programming Language
 * Annotation Parameters Parsing Tests
 *
 * Tests for annotation parameters including parameter parsing, nested annotations,
 * and annotation validation.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "grammar_annotations.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_annotation_parameter_tests(AsthraTestContext *context) {
    (void)context; // Unused parameter
    // Initialize any global state if needed
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_annotation_parameter_tests(AsthraTestContext *context) {
    (void)context; // Unused parameter
    // Clean up any global state if needed
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// ANNOTATION PARAMETERS TESTS
// =============================================================================

/**
 * Test: Parse Annotation Parameters
 * Verifies that annotation parameters are parsed correctly
 */
static AsthraTestResult test_parse_annotation_parameters(AsthraTestContext *context) {
    const char *test_source =
        "#[validate(min=0, max=100)] pub fn set_percentage(value: i32) -> void {}";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parse_top_level_decl(parser);

    if (!asthra_test_assert_not_null(context, result, "Failed to parse parameterized annotation")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations,
                                     "Function should have parameterized annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Nested Annotations
 * Verifies that nested annotations are parsed correctly
 */
static AsthraTestResult test_parse_nested_annotations(AsthraTestContext *context) {
    const char *test_source = "#[doc(description=\"Function\", version=\"1.0\")] pub fn "
                              "documented_function(none) -> void {}";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parse_top_level_decl(parser);

    if (!asthra_test_assert_not_null(context, result, "Failed to parse nested annotations")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations,
                                     "Function should have nested annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Annotation Validation
 * Verifies that annotation validation works correctly
 */
static AsthraTestResult test_parse_annotation_validation(AsthraTestContext *context) {
    // Test valid annotation
    const char *valid_source = "#[deprecated(since=\"1.0\")] pub fn old_function(none) -> void {}";
    Parser *parser1 = create_test_parser(valid_source);

    if (!asthra_test_assert_not_null(context, parser1, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result1 = parse_top_level_decl(parser1);

    if (!asthra_test_assert_not_null(context, result1, "Valid annotation should parse")) {
        destroy_test_parser(parser1);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result1);
    destroy_test_parser(parser1);

    // Test invalid annotation syntax
    const char *invalid_source = "#[invalid_syntax(] pub fn bad_function(none) -> void {}";
    Parser *parser2 = create_test_parser(invalid_source);

    if (!asthra_test_assert_not_null(context, parser2, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result2 = parse_top_level_decl(parser2);

    // Should either fail to parse or report errors
    if (result2) {
        // If it parsed, check for errors
        if (!asthra_test_assert_bool_eq(context, parser_had_error(parser2), true,
                                        "Invalid annotation should report errors")) {
            ast_free_node(result2);
            destroy_test_parser(parser2);
            return ASTHRA_TEST_FAIL;
        }
        ast_free_node(result2);
    }

    destroy_test_parser(parser2);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Complex Parameter Types
 * Verifies that complex parameter types are parsed correctly
 */
static AsthraTestResult test_parse_complex_parameter_types(AsthraTestContext *context) {
    const char *test_cases[] = {
        "#[range(min=0, max=1)] pub fn set_ratio(value: f64) -> void {}",        // Float parameters
        "#[array(size=10, type=\"i32\")] pub fn process_array(none) -> void {}", // String and
                                                                                 // integer
                                                                                 // parameters
        "#[config(enabled=true, debug=false)] pub fn configure(none) -> void {}", // Boolean
                                                                                  // parameters
        "#[timeout(seconds=30, retries=3)] pub fn network_call(none) -> void {}" // Multiple integer
                                                                                 // parameters
    };

    size_t case_count = sizeof(test_cases) / sizeof(test_cases[0]);

    for (size_t i = 0; i < case_count; i++) {
        Parser *parser = create_test_parser(test_cases[i]);

        if (!asthra_test_assert_not_null(context, parser,
                                         "Failed to create test parser for case %zu", i)) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parse_top_level_decl(parser);

        if (!asthra_test_assert_not_null(
                context, result, "Failed to parse complex parameter types for case %zu", i)) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        if (!asthra_test_assert_not_null(
                context, result->data.function_decl.annotations,
                "Function should have complex parameter annotations for case %zu", i)) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Array Parameters
 * Verifies that array parameters in annotations are parsed correctly
 */
static AsthraTestResult test_parse_array_parameters(AsthraTestContext *context) {
    const char *test_source =
        "#[allowed_values(values=\"1,2,3,5,8\")] pub fn fibonacci_check(n: i32) -> bool {}";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parse_top_level_decl(parser);

    if (!asthra_test_assert_not_null(context, result,
                                     "Failed to parse array parameter annotation")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations,
                                     "Function should have array parameter annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Object Parameters
 * Verifies that object-like parameters in annotations are parsed correctly
 */
static AsthraTestResult test_parse_object_parameters(AsthraTestContext *context) {
    const char *test_source =
        "#[cache(type=\"LRU\", size=1000, ttl=3600)] pub fn cached_lookup(none) -> void {}";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parse_top_level_decl(parser);

    if (!asthra_test_assert_not_null(context, result,
                                     "Failed to parse object parameter annotation")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations,
                                     "Function should have object parameter annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Parameter Expressions
 * Verifies that expression parameters in annotations are parsed correctly
 */
static AsthraTestResult test_parse_parameter_expressions(AsthraTestContext *context) {
    const char *test_source =
        "#[bounds(min=MIN_VALUE, max=MAX_VALUE)] pub fn bounded_function(none) -> void {}";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parse_top_level_decl(parser);

    if (!asthra_test_assert_not_null(context, result,
                                     "Failed to parse expression parameter annotation")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations,
                                     "Function should have expression parameter annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Parameter Default Values
 * Verifies that default values in annotation parameters are parsed correctly
 */
static AsthraTestResult test_parse_parameter_defaults(AsthraTestContext *context) {
    const char *test_source = "#[retry(attempts=3, delay=1000, exponential_backoff=true)] pub fn "
                              "resilient_operation(none) -> void {}";
    Parser *parser = create_test_parser(test_source);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parse_top_level_decl(parser);

    if (!asthra_test_assert_not_null(context, result,
                                     "Failed to parse default parameter annotation")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_not_null(context, result->data.function_decl.annotations,
                                     "Function should have default parameter annotations")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all annotation parameter tests
 */
AsthraTestSuite *create_annotation_parameters_test_suite(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("Annotation Parameter Tests",
                                                      "Annotation parameter parsing testing");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_annotation_parameter_tests);
    asthra_test_suite_set_teardown(suite, teardown_annotation_parameter_tests);

    // Annotation parameters
    asthra_test_suite_add_test(suite, "test_parse_annotation_parameters",
                               "Parse annotation parameters", test_parse_annotation_parameters);

    asthra_test_suite_add_test(suite, "test_parse_nested_annotations", "Parse nested annotations",
                               test_parse_nested_annotations);

    asthra_test_suite_add_test(suite, "test_parse_annotation_validation",
                               "Parse annotation validation", test_parse_annotation_validation);

    asthra_test_suite_add_test(suite, "test_parse_complex_parameter_types",
                               "Parse complex parameter types", test_parse_complex_parameter_types);

    asthra_test_suite_add_test(suite, "test_parse_array_parameters", "Parse array parameters",
                               test_parse_array_parameters);

    asthra_test_suite_add_test(suite, "test_parse_object_parameters", "Parse object parameters",
                               test_parse_object_parameters);

    asthra_test_suite_add_test(suite, "test_parse_parameter_expressions",
                               "Parse parameter expressions", test_parse_parameter_expressions);

    asthra_test_suite_add_test(suite, "test_parse_parameter_defaults", "Parse parameter defaults",
                               test_parse_parameter_defaults);

    return suite;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite *suite = create_annotation_parameters_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create annotation parameters test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    return result;
}
