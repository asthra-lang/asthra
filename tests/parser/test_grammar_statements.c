/**
 * Asthra Programming Language
 * Statement Grammar Production Tests
 *
 * Tests for statement parsing including variable declarations, assignment
 * statements, if statements, for statements, match statements, and return
 * statements.
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

static AsthraTestResult setup_statement_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_statement_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// STATEMENT PARSING TESTS
// =============================================================================

/**
 * Test: Parse Variable Declarations
 * Verifies that variable declarations are parsed correctly
 */
static AsthraTestResult test_parse_variable_declarations(AsthraTestContext *context) {
    const char *var_declarations[] = {
        "let x: i32 = 42;",             // v1.15+ requires type annotation
        "let y: f64 = 3.14;",           // v1.15+ requires type annotation
        "let name: string = \"hello\";" // v1.15+ requires type annotation
    };

    size_t decl_count = sizeof(var_declarations) / sizeof(var_declarations[0]);

    for (size_t i = 0; i < decl_count; i++) {
        Parser *parser = create_test_parser(var_declarations[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse variable declaration")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

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
 * Test: Parse While Statements - REMOVED
 * While statements are not in the PEG grammar v1.22
 * The grammar only supports: if, for, match, return, spawn statements
 */
// Removed test_parse_while_statements - not supported in grammar

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

/**
 * Test: Parse Block Statements
 * Verifies that block statements are parsed correctly
 */
static AsthraTestResult test_parse_block_statements(AsthraTestContext *context) {
    const char *block_statements[] = {"{ }", "{ let x: i32 = 42; }",
                                      "{ let x: i32 = 42; let y: i32 = x + 1; return y; }"};

    size_t block_count = sizeof(block_statements) / sizeof(block_statements[0]);

    for (size_t i = 0; i < block_count; i++) {
        Parser *parser = create_test_parser(block_statements[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse block statement")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Unsafe Blocks
 * Verifies that unsafe blocks are parsed correctly
 */
static AsthraTestResult test_parse_unsafe_blocks(AsthraTestContext *context) {
    const char *unsafe_blocks[] = {"unsafe { }", "unsafe { let x: i32 = 42; }",
                                   "unsafe { let x: i32 = 42; let y: i32 = x + 1; }",
                                   "unsafe { ffi_function(); }"};

    size_t unsafe_count = sizeof(unsafe_blocks) / sizeof(unsafe_blocks[0]);

    for (size_t i = 0; i < unsafe_count; i++) {
        Parser *parser = create_test_parser(unsafe_blocks[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse unsafe block")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        // Verify that the result is actually an unsafe block
        if (!asthra_test_assert_int_eq(context, result->type, AST_UNSAFE_BLOCK,
                                       "Expected AST_UNSAFE_BLOCK node type")) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        // Verify that the unsafe block contains a block
        if (!asthra_test_assert_not_null(context, result->data.unsafe_block.block,
                                         "Unsafe block should contain a block")) {
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
 * Register all statement parsing tests
 */
AsthraTestSuite *create_grammar_statements_test_suite(void) {
    AsthraTestSuite *suite =
        asthra_test_suite_create("Grammar Statement Tests", "Statement parsing testing");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_statement_tests);
    asthra_test_suite_set_teardown(suite, teardown_statement_tests);

    // Statement parsing tests
    asthra_test_suite_add_test(suite, "test_parse_variable_declarations",
                               "Parse variable declarations", test_parse_variable_declarations);

    asthra_test_suite_add_test(suite, "test_parse_assignment_statements",
                               "Parse assignment statements", test_parse_assignment_statements);

    asthra_test_suite_add_test(suite, "test_parse_if_statements", "Parse if statements",
                               test_parse_if_statements);

    asthra_test_suite_add_test(suite, "test_parse_for_statements", "Parse for statements",
                               test_parse_for_statements);

    asthra_test_suite_add_test(suite, "test_parse_match_statements", "Parse match statements",
                               test_parse_match_statements);

    asthra_test_suite_add_test(suite, "test_parse_return_statements", "Parse return statements",
                               test_parse_return_statements);

    asthra_test_suite_add_test(suite, "test_parse_block_statements", "Parse block statements",
                               test_parse_block_statements);

    asthra_test_suite_add_test(suite, "test_parse_unsafe_blocks", "Parse unsafe blocks",
                               test_parse_unsafe_blocks);

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
    printf("=== Asthra Grammar Statement Tests ===\n\n");

    AsthraTestSuite *suite = create_grammar_statements_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED
