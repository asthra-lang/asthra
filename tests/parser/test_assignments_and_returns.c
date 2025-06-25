/**
 * Asthra Programming Language
 * Assignment and Return Statement Tests
 *
 * Tests for parsing assignment and return statements.
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

static AsthraTestResult setup_assignment_return_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_assignment_return_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// ASSIGNMENT AND RETURN TESTS
// =============================================================================

/**
 * Test: Parse Assignment Statements
 * Verifies that assignment statements are parsed correctly
 */
static AsthraTestResult test_parse_assignment_statements(AsthraTestContext *context) {
    const char *assignments[] = {
        "x = 42;" // Simple assignment to debug
    };

    size_t assign_count = sizeof(assignments) / sizeof(assignments[0]);

    for (size_t i = 0; i < assign_count; i++) {
        Parser *parser = create_test_parser(assignments[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse assignment statement")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Return Statements (v1.14+)
 * Verifies that return statements require expressions
 */
static AsthraTestResult test_parse_return_statements(AsthraTestContext *context) {
    // Valid return statements (should pass)
    const char *valid_returns[] = {"return 42;", "return x + y;",
                                   "return ();", // Unit return
                                   "return Result.Ok(value);", "return some_function();"};

    size_t valid_count = sizeof(valid_returns) / sizeof(valid_returns[0]);

    for (size_t i = 0; i < valid_count; i++) {
        Parser *parser = create_test_parser(valid_returns[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result,
                                         "Failed to parse valid return statement")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        // Verify expression is always present
        if (!asthra_test_assert_not_null(context, result->data.return_stmt.expression,
                                         "Return statement missing required expression")) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    // Invalid return statements (should fail in v1.14+)
    const char *invalid_returns[] = {
        "return;", // Bare return - no longer valid
    };

    size_t invalid_count = sizeof(invalid_returns) / sizeof(invalid_returns[0]);

    for (size_t i = 0; i < invalid_count; i++) {
        Parser *parser = create_test_parser(invalid_returns[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        // Should fail to parse
        if (result != NULL) {
            asthra_test_assert_bool(context, false,
                                    "Bare return should fail in v1.14+ but parsing succeeded");
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all assignment and return tests
 */
AsthraTestSuite *create_assignments_and_returns_test_suite(void) {
    AsthraTestSuite *suite =
        asthra_test_suite_create("Assignments and Returns Tests", "Assignment and return statement parsing testing");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_assignment_return_tests);
    asthra_test_suite_set_teardown(suite, teardown_assignment_return_tests);

    // Assignment and return tests
    asthra_test_suite_add_test(suite, "test_parse_assignment_statements",
                               "Parse assignment statements", test_parse_assignment_statements);

    asthra_test_suite_add_test(suite, "test_parse_return_statements", "Parse return statements",
                               test_parse_return_statements);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Assignments and Returns Tests ===\n\n");

    AsthraTestSuite *suite = create_assignments_and_returns_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED