/**
 * Asthra Programming Language
 * Block and Unsafe Block Tests
 *
 * Tests for parsing block statements and unsafe blocks.
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

static AsthraTestResult setup_block_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_block_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// BLOCK TESTS
// =============================================================================

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

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all block tests
 */
AsthraTestSuite *create_blocks_and_unsafe_test_suite(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("Blocks and Unsafe Tests",
                                                      "Block and unsafe block parsing testing");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_block_tests);
    asthra_test_suite_set_teardown(suite, teardown_block_tests);

    // Block tests
    asthra_test_suite_add_test(suite, "test_parse_block_statements", "Parse block statements",
                               test_parse_block_statements);

    asthra_test_suite_add_test(suite, "test_parse_unsafe_blocks", "Parse unsafe blocks",
                               test_parse_unsafe_blocks);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Blocks and Unsafe Tests ===\n\n");

    AsthraTestSuite *suite = create_blocks_and_unsafe_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED