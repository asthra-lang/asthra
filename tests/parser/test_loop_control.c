/**
 * Asthra Programming Language
 * Loop Control Statement Tests
 *
 * Tests for parsing break and continue statements in various contexts.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_data.h"
#include "../framework/test_framework.h"
#include "grammar_statements.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_loop_control_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_loop_control_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// LOOP CONTROL TESTS
// =============================================================================

/**
 * Test: Parse Break Statements
 * Verifies that break statements are parsed correctly
 */
static AsthraTestResult test_parse_break_statements(AsthraTestContext *context) {
    const char *break_statements[] = {
        "break;", // Simple break statement
    };

    size_t break_count = sizeof(break_statements) / sizeof(break_statements[0]);

    for (size_t i = 0; i < break_count; i++) {
        Parser *parser = create_test_parser(break_statements[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse break statement")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        // Verify that the result is actually a break statement
        if (!asthra_test_assert_int_eq(context, result->type, AST_BREAK_STMT,
                                       "Expected AST_BREAK_STMT node type")) {
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
 * Test: Parse Continue Statements
 * Verifies that continue statements are parsed correctly
 */
static AsthraTestResult test_parse_continue_statements(AsthraTestContext *context) {
    const char *continue_statements[] = {
        "continue;", // Simple continue statement
    };

    size_t continue_count = sizeof(continue_statements) / sizeof(continue_statements[0]);

    for (size_t i = 0; i < continue_count; i++) {
        Parser *parser = create_test_parser(continue_statements[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse continue statement")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        // Verify that the result is actually a continue statement
        if (!asthra_test_assert_int_eq(context, result->type, AST_CONTINUE_STMT,
                                       "Expected AST_CONTINUE_STMT node type")) {
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
 * Test: Parse Loop Control Statements in Context
 * Verifies that break and continue statements work within for loops
 */
static AsthraTestResult test_parse_loop_control_in_context(AsthraTestContext *context) {
    const char *loop_control_examples[] = {
        "for i in range { if i > 5 { break; } }",
        "for item in list { if item == null { continue; } process(item); }",
        "for x in nums { if x < 0 { continue; } if x > 100 { break; } sum = sum + x; }"};

    size_t example_count = sizeof(loop_control_examples) / sizeof(loop_control_examples[0]);

    for (size_t i = 0; i < example_count; i++) {
        Parser *parser = create_test_parser(loop_control_examples[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result,
                                         "Failed to parse loop with control statements")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        // Verify that the result is a for statement
        if (!asthra_test_assert_int_eq(context, result->type, AST_FOR_STMT,
                                       "Expected AST_FOR_STMT node type")) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all loop control tests
 */
AsthraTestSuite *create_loop_control_test_suite(void) {
    AsthraTestSuite *suite =
        asthra_test_suite_create("Loop Control Tests", "Loop control statement parsing testing");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_loop_control_tests);
    asthra_test_suite_set_teardown(suite, teardown_loop_control_tests);

    // Loop control tests
    asthra_test_suite_add_test(suite, "test_parse_break_statements", "Parse break statements",
                               test_parse_break_statements);

    asthra_test_suite_add_test(suite, "test_parse_continue_statements", "Parse continue statements",
                               test_parse_continue_statements);

    asthra_test_suite_add_test(suite, "test_parse_loop_control_in_context",
                               "Parse loop control statements in context",
                               test_parse_loop_control_in_context);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Loop Control Tests ===\n\n");

    AsthraTestSuite *suite = create_loop_control_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED