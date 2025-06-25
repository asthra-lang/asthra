/**
 * Asthra Programming Language
 * Operator Precedence Fix Tests
 *
 * Tests for the critical operator precedence fix that corrects shift operator
 * positioning and implements missing bitwise operators.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "ast_types.h"
#include "grammar_expressions.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_precedence_tests(AsthraTestContext *context) {
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_precedence_tests(AsthraTestContext *context) {
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Helper function to verify binary expression structure
 */
static bool verify_binary_expr(ASTNode *node, BinaryOperator expected_op, ASTNode **left_out,
                               ASTNode **right_out) {
    if (!node || node->type != AST_BINARY_EXPR) {
        return false;
    }

    if (node->data.binary_expr.operator!= expected_op) {
        return false;
    }

    if (left_out)
        *left_out = node->data.binary_expr.left;
    if (right_out)
        *right_out = node->data.binary_expr.right;

    return true;
}

// =============================================================================
// PRECEDENCE VALIDATION TESTS
// =============================================================================

/**
 * Test: Shift Operator Precedence Fix
 * Critical test: a + b << c should parse as (a + b) << c, NOT a + (b << c)
 */
static AsthraTestResult test_shift_operator_precedence(AsthraTestContext *context) {
    const char *expression = "a + b << c";
    Parser *parser = create_test_parser(expression);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parse_expr(parser);

    if (!asthra_test_assert_not_null(context, result, "Failed to parse shift expression")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify structure: SHIFT(ADD(a, b), c)
    ASTNode *left_side;
    ASTNode *right_side;
    if (!verify_binary_expr(result, BINOP_LSHIFT, &left_side, &right_side)) {
        ast_free_node(result);
        destroy_test_parser(parser);
        asthra_test_assert_bool(context, false, "Root should be left shift operation");
        return ASTHRA_TEST_FAIL;
    }

    // Verify left side is ADD operation
    if (!verify_binary_expr(left_side, BINOP_ADD, NULL, NULL)) {
        ast_free_node(result);
        destroy_test_parser(parser);
        asthra_test_assert_bool(context, false, "Left side should be addition operation");
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Bitwise Operators Implementation
 * Verifies that all bitwise operators are now properly implemented
 */
static AsthraTestResult test_bitwise_operators_implemented(AsthraTestContext *context) {
    const char *expressions[] = {
        "a & b", // Bitwise AND
        "x | y", // Bitwise OR
        "m ^ n"  // Bitwise XOR
    };

    BinaryOperator expected_ops[] = {BINOP_BITWISE_AND, BINOP_BITWISE_OR, BINOP_BITWISE_XOR};

    size_t expr_count = sizeof(expressions) / sizeof(expressions[0]);

    for (size_t i = 0; i < expr_count; i++) {
        Parser *parser = create_test_parser(expressions[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parse_expr(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse bitwise expression")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        if (!verify_binary_expr(result, expected_ops[i], NULL, NULL)) {
            ast_free_node(result);
            destroy_test_parser(parser);
            asthra_test_assert_bool(context, false, "Incorrect bitwise operator type");
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Complex Bitwise Expression Precedence
 * Tests: FLAG_A | FLAG_B & FLAG_C should parse as FLAG_A | (FLAG_B & FLAG_C)
 */
static AsthraTestResult test_complex_bitwise_precedence(AsthraTestContext *context) {
    const char *expression = "FLAG_A | FLAG_B & FLAG_C";
    Parser *parser = create_test_parser(expression);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parse_expr(parser);

    if (!asthra_test_assert_not_null(context, result,
                                     "Failed to parse complex bitwise expression")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify structure: OR(FLAG_A, AND(FLAG_B, FLAG_C))
    ASTNode *left_side;
    ASTNode *right_side;
    if (!verify_binary_expr(result, BINOP_BITWISE_OR, &left_side, &right_side)) {
        ast_free_node(result);
        destroy_test_parser(parser);
        asthra_test_assert_bool(context, false, "Root should be bitwise OR operation");
        return ASTHRA_TEST_FAIL;
    }

    // Verify right side is AND operation
    if (!verify_binary_expr(right_side, BINOP_BITWISE_AND, NULL, NULL)) {
        ast_free_node(result);
        destroy_test_parser(parser);
        asthra_test_assert_bool(context, false, "Right side should be bitwise AND operation");
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Memory Address Calculation Fix
 * Critical AI generation test: base + offset << shift_amount
 */
static AsthraTestResult test_memory_address_calculation(AsthraTestContext *context) {
    const char *expression = "base + offset << 2";
    Parser *parser = create_test_parser(expression);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parse_expr(parser);

    if (!asthra_test_assert_not_null(context, result, "Failed to parse memory calculation")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Verify it's a shift operation at the root
    ASTNode *left_side;
    if (!verify_binary_expr(result, BINOP_LSHIFT, &left_side, NULL)) {
        ast_free_node(result);
        destroy_test_parser(parser);
        asthra_test_assert_bool(context, false, "Root should be left shift operation");
        return ASTHRA_TEST_FAIL;
    }

    // Verify left side is addition
    if (!verify_binary_expr(left_side, BINOP_ADD, NULL, NULL)) {
        ast_free_node(result);
        destroy_test_parser(parser);
        asthra_test_assert_bool(context, false, "Left side should be addition operation");
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
 * Register all operator precedence fix tests
 */
AsthraTestSuite *create_operator_precedence_test_suite(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("Operator Precedence Fix Tests",
                                                      "Critical operator precedence fixes");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_precedence_tests);
    asthra_test_suite_set_teardown(suite, teardown_precedence_tests);

    // Precedence validation tests
    asthra_test_suite_add_test(suite, "test_shift_operator_precedence",
                               "Fix shift operator precedence", test_shift_operator_precedence);

    asthra_test_suite_add_test(suite, "test_bitwise_operators_implemented",
                               "Implement missing bitwise operators",
                               test_bitwise_operators_implemented);

    asthra_test_suite_add_test(suite, "test_complex_bitwise_precedence",
                               "Complex bitwise operator precedence",
                               test_complex_bitwise_precedence);

    asthra_test_suite_add_test(suite, "test_memory_address_calculation",
                               "Memory address calculation fix", test_memory_address_calculation);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

int main(void) {
    printf("=== Asthra Operator Precedence Fix Tests ===\n\n");

    AsthraTestSuite *suite = create_operator_precedence_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
