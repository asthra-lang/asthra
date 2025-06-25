/**
 * Asthra Programming Language
 * Control Flow Statement Tests
 *
 * Tests for parsing control flow statements including if, for, and match statements.
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

static AsthraTestResult setup_control_flow_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_control_flow_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// CONTROL FLOW TESTS
// =============================================================================

/**
 * Test: Parse If Statements
 * Verifies that if statements are parsed correctly
 */
static AsthraTestResult test_parse_if_statements(AsthraTestContext *context) {
    const char *if_statements[] = {
        "if x > 0 { return x; }",                     // Grammar: if Expr Block
        "if x > 0 { return x; } else { return -x; }", // Grammar: if Expr Block else Block
        "if x > 0 { return x; } else if x < 0 { return -x; } else { return 0; }" // Grammar: if Expr
                                                                                 // Block else
                                                                                 // IfStmt
    };

    size_t if_count = sizeof(if_statements) / sizeof(if_statements[0]);

    for (size_t i = 0; i < if_count; i++) {
        Parser *parser = create_test_parser(if_statements[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse if statement")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse For Statements
 * Verifies that for statements are parsed correctly
 */
static AsthraTestResult test_parse_for_statements(AsthraTestContext *context) {
    const char *for_statements[] = {
        "for item in array { process(item); }", // Grammar: for SimpleIdent in Expr Block
        "for i in range { print(i); }"          // Grammar: for SimpleIdent in Expr Block
    };

    size_t for_count = sizeof(for_statements) / sizeof(for_statements[0]);

    for (size_t i = 0; i < for_count; i++) {
        Parser *parser = create_test_parser(for_statements[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse for statement")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Match Statements
 * Verifies that match statements are parsed correctly
 */
static AsthraTestResult test_parse_match_statements(AsthraTestContext *context) {
    const char *match_statements[] = {
        "match value { 1 => { return \"one\"; } 2 => { return \"two\"; } _ => { return \"other\"; "
        "} }", // Grammar: match Expr { MatchArm* }
        "match option { Option.Some(value) => { process(value); } Option.None => { handle_none(); "
        "} }" // Grammar: MatchArm <- Pattern => Block
    };

    size_t match_count = sizeof(match_statements) / sizeof(match_statements[0]);

    for (size_t i = 0; i < match_count; i++) {
        Parser *parser = create_test_parser(match_statements[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse match statement")) {
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
 * Register all control flow tests
 */
AsthraTestSuite *create_control_flow_test_suite(void) {
    AsthraTestSuite *suite =
        asthra_test_suite_create("Control Flow Tests", "Control flow statement parsing testing");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_control_flow_tests);
    asthra_test_suite_set_teardown(suite, teardown_control_flow_tests);

    // Control flow tests
    asthra_test_suite_add_test(suite, "test_parse_if_statements", "Parse if statements",
                               test_parse_if_statements);

    asthra_test_suite_add_test(suite, "test_parse_for_statements", "Parse for statements",
                               test_parse_for_statements);

    asthra_test_suite_add_test(suite, "test_parse_match_statements", "Parse match statements",
                               test_parse_match_statements);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Control Flow Tests ===\n\n");

    AsthraTestSuite *suite = create_control_flow_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED