/**
 * Asthra Programming Language
 * Lexer Operator Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test operator disambiguation and complex operator sequences
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "lexer.h"
#include <string.h>
#include <errno.h>

// =============================================================================
// OPERATOR DISAMBIGUATION TESTS
// =============================================================================

/**
 * Test compound operator recognition
 */
AsthraTestResult test_lexer_compound_operators(AsthraTestContext *context) {
    // NOTE: Based on Asthra grammar grammar.txt, compound assignment operators
    // (+=, -=, etc.) and increment/decrement (++, --) are NOT supported.
    // The grammar only supports basic assignment (=) and simple operators.
    
    const struct {
        const char* input;
        TokenType first_expected;
        TokenType second_expected;
    } test_cases[] = {
        // Test basic operators that are supported by the grammar
        {"+ =", TOKEN_PLUS, TOKEN_ASSIGN},
        {"- =", TOKEN_MINUS, TOKEN_ASSIGN},
        {"* =", TOKEN_MULTIPLY, TOKEN_ASSIGN},
        {"/ =", TOKEN_DIVIDE, TOKEN_ASSIGN},
        {"% =", TOKEN_MODULO, TOKEN_ASSIGN},
        {"& =", TOKEN_BITWISE_AND, TOKEN_ASSIGN},
        {"| =", TOKEN_BITWISE_OR, TOKEN_ASSIGN},
        {"^ =", TOKEN_BITWISE_XOR, TOKEN_ASSIGN},
        {"< <=", TOKEN_LESS_THAN, TOKEN_LESS_EQUAL},  // Note: <= is less-equal, not left-shift
        {"> >=", TOKEN_GREATER_THAN, TOKEN_GREATER_EQUAL}, // Note: >= is greater-equal, not right-shift
        {NULL, TOKEN_EOF, TOKEN_EOF}
    };

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char* input = test_cases[i].input;
        TokenType first_expected = test_cases[i].first_expected;
        TokenType second_expected = test_cases[i].second_expected;
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_compound_ops.ast");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for compound operator: %s", input);

        Token first_token = lexer_next_token(lexer);
        // Skip whitespace for first token
        while (first_token.type == TOKEN_WHITESPACE) {
            token_free(&first_token);
            first_token = lexer_next_token(lexer);
        }
        ASTHRA_TEST_ASSERT_TRUE(context, first_token.type == first_expected,
                               "Expected first token type %d, got %d", first_expected, first_token.type);

        Token second_token = lexer_next_token(lexer);
        // Skip whitespace for second token
        while (second_token.type == TOKEN_WHITESPACE) {
            token_free(&second_token);
            second_token = lexer_next_token(lexer);
        }
        ASTHRA_TEST_ASSERT_TRUE(context, second_token.type == second_expected,
                               "Expected second token type %d, got %d", second_expected, second_token.type);

        token_free(&first_token);
        token_free(&second_token);
        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test operator disambiguation in context
 */
AsthraTestResult test_lexer_operator_disambiguation(AsthraTestContext *context) {
    const struct {
        const char* input;
        TokenType tokens[5]; // Up to 5 expected tokens
    } test_cases[] = {
        // Test grammar-compliant operator sequences
        {"<<", {TOKEN_LEFT_SHIFT, TOKEN_EOF}},     // Left shift is supported
        {">>", {TOKEN_RIGHT_SHIFT, TOKEN_EOF}},    // Right shift is supported  
        {"= =", {TOKEN_ASSIGN, TOKEN_ASSIGN, TOKEN_EOF}},
        {"! =", {TOKEN_LOGICAL_NOT, TOKEN_ASSIGN, TOKEN_EOF}},
        {"< =", {TOKEN_LESS_THAN, TOKEN_ASSIGN, TOKEN_EOF}},
        {"> =", {TOKEN_GREATER_THAN, TOKEN_ASSIGN, TOKEN_EOF}},
        {"+ +", {TOKEN_PLUS, TOKEN_PLUS, TOKEN_EOF}},
        {"- -", {TOKEN_MINUS, TOKEN_MINUS, TOKEN_EOF}},
        {"* *", {TOKEN_MULTIPLY, TOKEN_MULTIPLY, TOKEN_EOF}},
        {"/ /", {TOKEN_DIVIDE, TOKEN_DIVIDE, TOKEN_EOF}},
        {"& &", {TOKEN_BITWISE_AND, TOKEN_BITWISE_AND, TOKEN_EOF}},
        {"| |", {TOKEN_BITWISE_OR, TOKEN_BITWISE_OR, TOKEN_EOF}},
        {"^ ^", {TOKEN_BITWISE_XOR, TOKEN_BITWISE_XOR, TOKEN_EOF}},
        {": :", {TOKEN_COLON, TOKEN_COLON, TOKEN_EOF}},
        {NULL, {TOKEN_EOF}}
    };

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char* input = test_cases[i].input;
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_op_disambiguation.ast");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for disambiguation test: %s", input);

        for (int j = 0; j < 5 && test_cases[i].tokens[j] != TOKEN_EOF; j++) {
            Token token = lexer_next_token(lexer);
            
            // Skip whitespace
            while (token.type == TOKEN_WHITESPACE) {
                token_free(&token);
                token = lexer_next_token(lexer);
            }
            
            ASTHRA_TEST_ASSERT_TRUE(context, token.type == test_cases[i].tokens[j],
                                   "Expected token type %d, got %d at position %d for input '%s'",
                                   test_cases[i].tokens[j], token.type, j, input);
            token_free(&token);
        }

        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test complex operator sequences
 */
AsthraTestResult test_lexer_complex_operator_sequences(AsthraTestContext *context) {
    const struct {
        const char* input;
        const char* description;
        int expected_token_count;
    } test_cases[] = {
        // Note: Compound assignments (+=, -=) and increment/decrement (++, --) 
        // are NOT supported by Asthra grammar, so using grammar-compliant alternatives
        {"a + b * c", "Basic arithmetic", 6}, // a, +, b, *, c, EOF
        {"x<<y>>z", "Shift operators", 6}, // x, <<, y, >>, z, EOF
        {"ptr->field", "Arrow operator", 4}, // ptr, ->, field, EOF
        {"Class::method", "Scope resolution", 4}, // Class, ::, method, EOF
        {"a==b!=c", "Comparison chain", 6}, // a, ==, b, !=, c, EOF
        {"x<=y>=z", "Comparison operators", 6}, // x, <=, y, >=, z, EOF
        {"a&&b||c", "Logical operators", 6}, // a, &&, b, ||, c, EOF
        {"&ptr*val", "Address and dereference", 5}, // &, ptr, *, val, EOF
        {"a<b>c", "Template-like syntax", 6}, // a, <, b, >, c, EOF
        {NULL, NULL, 0}
    };

    for (int i = 0; test_cases[i].input != NULL; i++) {
        const char* input = test_cases[i].input;
        const char* description = test_cases[i].description;
        int expected_count = test_cases[i].expected_token_count;
        
        Lexer* lexer = lexer_create(input, strlen(input), "test_complex_operators.ast");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for test: %s", description);

        int token_count = 0;
        Token token;
        do {
            token = lexer_next_token(lexer);
            if (token.type != TOKEN_WHITESPACE) {
                token_count++;
            }
            token_free(&token);
        } while (token.type != TOKEN_EOF);

        if (!asthra_test_assert_int_eq(context, token_count, expected_count,
                                      "Expected %d tokens for '%s', got %d", 
                                      expected_count, description, token_count)) {
            return ASTHRA_TEST_FAIL;
        }

        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test operator precedence parsing preparation
 */
AsthraTestResult test_lexer_operator_precedence_tokens(AsthraTestContext *context) {
    const char* input = "a + b * c / d - e % f";
    
    Lexer* lexer = lexer_create(input, strlen(input), "test_precedence.ast");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, lexer, "Failed to create lexer for precedence test");

    TokenType expected_operators[] = {
        TOKEN_PLUS,     // +
        TOKEN_MULTIPLY, // *
        TOKEN_DIVIDE,   // /
        TOKEN_MINUS,    // -
        TOKEN_MODULO    // %
    };
    
    int operator_index = 0;
    Token token;
    do {
        token = lexer_next_token(lexer);
        
        if (token.type == TOKEN_PLUS || token.type == TOKEN_MINUS || 
            token.type == TOKEN_MULTIPLY || token.type == TOKEN_DIVIDE || 
            token.type == TOKEN_MODULO) {
            
            ASTHRA_TEST_ASSERT_TRUE(context, operator_index < 5,
                                   "Too many operators found");
            
            ASTHRA_TEST_ASSERT_TRUE(context, token.type == expected_operators[operator_index],
                                   "Expected operator %d, got %d at position %d",
                                   expected_operators[operator_index], token.type, operator_index);
            
            operator_index++;
        }
        
        token_free(&token);
    } while (token.type != TOKEN_EOF);

    if (!asthra_test_assert_int_eq(context, operator_index, 5,
                                  "Expected 5 operators, found %d", operator_index)) {
        return ASTHRA_TEST_FAIL;
    }

    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Main test function for lexer operators
 */
int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Lexer Operator Tests");
    
    asthra_test_suite_add_test(suite, "test_lexer_compound_operators",
                              "Test compound operator recognition",
                              test_lexer_compound_operators);
    asthra_test_suite_add_test(suite, "test_lexer_operator_disambiguation",
                              "Test operator disambiguation in context",
                              test_lexer_operator_disambiguation);
    asthra_test_suite_add_test(suite, "test_lexer_complex_operator_sequences",
                              "Test complex operator sequences",
                              test_lexer_complex_operator_sequences);
    asthra_test_suite_add_test(suite, "test_lexer_operator_precedence_tokens",
                              "Test operator precedence parsing preparation",
                              test_lexer_operator_precedence_tokens);
    
    return asthra_test_suite_run_and_exit(suite);
} 
