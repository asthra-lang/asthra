/**
 * Asthra Programming Language
 * Expression Grammar Production Tests
 * 
 * Tests for expression parsing including primary expressions, binary
 * expressions, unary expressions, postfix expressions, and call expressions.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "../framework/test_data.h"
#include "parser.h"
#include "grammar_expressions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_expression_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_expression_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// EXPRESSION PARSING TESTS
// =============================================================================

/**
 * Test: Parse Primary Expressions
 * Verifies that primary expressions are parsed correctly
 */
static AsthraTestResult test_parse_primary_expressions(AsthraTestContext* context) {
    const char* primary_expressions[] = {
        "42",           // Integer literal
        "3.14",         // Float literal
        "true",         // Boolean literal
        "'a'",          // Character literal
        "\"hello\"",    // String literal
        "identifier",   // Identifier
        "(42)"          // Parenthesized expression
    };
    
    size_t expr_count = sizeof(primary_expressions) / sizeof(primary_expressions[0]);
    
    for (size_t i = 0; i < expr_count; i++) {
        Parser* parser = create_test_parser(primary_expressions[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse primary expression")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Binary Expressions
 * Verifies that binary expressions are parsed correctly
 */
static AsthraTestResult test_parse_binary_expressions(AsthraTestContext* context) {
    const char* binary_expressions[] = {
        "a + b",
        "x - y",
        "a * b",
        "x / y",
        "a % b",
        "x == y",
        "a != b",
        "x < y",
        "a <= b",
        "x > y",
        "a >= b",
        "x && y",
        "a || b"
    };
    
    size_t expr_count = sizeof(binary_expressions) / sizeof(binary_expressions[0]);
    
    for (size_t i = 0; i < expr_count; i++) {
        Parser* parser = create_test_parser(binary_expressions[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse binary expression")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Unary Expressions
 * Verifies that unary expressions are parsed correctly
 */
static AsthraTestResult test_parse_unary_expressions(AsthraTestContext* context) {
    const char* unary_expressions[] = {
        "-x",
        "!flag",
        "~bits",
        "*ptr",
        "&value"
    };
    
    size_t expr_count = sizeof(unary_expressions) / sizeof(unary_expressions[0]);
    
    for (size_t i = 0; i < expr_count; i++) {
        Parser* parser = create_test_parser(unary_expressions[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse unary expression")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Postfix Expressions
 * Verifies that postfix expressions are parsed correctly
 */
static AsthraTestResult test_parse_postfix_expressions(AsthraTestContext* context) {
    const char* postfix_expressions[] = {
        "array[0]",
        "obj.field",
        "array.len"
    };
    
    size_t expr_count = sizeof(postfix_expressions) / sizeof(postfix_expressions[0]);
    
    for (size_t i = 0; i < expr_count; i++) {
        Parser* parser = create_test_parser(postfix_expressions[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse postfix expression")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Call Expressions
 * Verifies that function call expressions are parsed correctly
 */
static AsthraTestResult test_parse_call_expressions(AsthraTestContext* context) {
    const char* call_expressions[] = {
        "foo(none)",
        "bar(x)",
        "baz(a, b, c)"
    };
    
    size_t expr_count = sizeof(call_expressions) / sizeof(call_expressions[0]);
    
    for (size_t i = 0; i < expr_count; i++) {
        Parser* parser = create_test_parser(call_expressions[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse call expression")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Assignment Expressions
 * Verifies that assignment expressions are parsed correctly
 */
static AsthraTestResult test_parse_assignment_expressions(AsthraTestContext* context) {
    const char* assignment_expressions[] = {
        "x = 42",
        "array[0] = value",
        "obj.field = new_value"
    };
    
    size_t expr_count = sizeof(assignment_expressions) / sizeof(assignment_expressions[0]);
    
    for (size_t i = 0; i < expr_count; i++) {
        Parser* parser = create_test_parser(assignment_expressions[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse assignment expression")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }
        
        ast_free_node(result);
        destroy_test_parser(parser);
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Parse Complex Expressions
 * Verifies that complex nested expressions are parsed correctly
 */
static AsthraTestResult test_parse_complex_expressions(AsthraTestContext* context) {
    const char* complex_expressions[] = {
        "a + b * c",
        "(a + b) * c",
        "func(x + y, z)",
        "array[i + 1].field",
        "obj.field + other_value"
    };
    
    size_t expr_count = sizeof(complex_expressions) / sizeof(complex_expressions[0]);
    
    for (size_t i = 0; i < expr_count; i++) {
        Parser* parser = create_test_parser(complex_expressions[i]);
        
        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }
        
        ASTNode* result = parser_parse_expression(parser);
        
        if (!asthra_test_assert_not_null(context, result, "Failed to parse complex expression")) {
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
 * Register all expression parsing tests
 */
AsthraTestSuite* create_grammar_expressions_test_suite(void) {
    AsthraTestSuite* suite = asthra_test_suite_create("Grammar Expression Tests", 
                                                     "Expression parsing testing");
    
    if (!suite) return NULL;
    
    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_expression_tests);
    asthra_test_suite_set_teardown(suite, teardown_expression_tests);
    
    // Expression parsing tests
    asthra_test_suite_add_test(suite, "test_parse_primary_expressions", 
                              "Parse primary expressions", 
                              test_parse_primary_expressions);
    
    asthra_test_suite_add_test(suite, "test_parse_binary_expressions", 
                              "Parse binary expressions", 
                              test_parse_binary_expressions);
    
    asthra_test_suite_add_test(suite, "test_parse_unary_expressions", 
                              "Parse unary expressions", 
                              test_parse_unary_expressions);
    
    asthra_test_suite_add_test(suite, "test_parse_postfix_expressions", 
                              "Parse postfix expressions", 
                              test_parse_postfix_expressions);
    
    asthra_test_suite_add_test(suite, "test_parse_call_expressions", 
                              "Parse call expressions", 
                              test_parse_call_expressions);
    
    asthra_test_suite_add_test(suite, "test_parse_assignment_expressions", 
                              "Parse assignment expressions", 
                              test_parse_assignment_expressions);
    
    asthra_test_suite_add_test(suite, "test_parse_complex_expressions", 
                              "Parse complex expressions", 
                              test_parse_complex_expressions);
    
    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Grammar Expression Tests ===\n\n");
    
    AsthraTestSuite* suite = create_grammar_expressions_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED 
