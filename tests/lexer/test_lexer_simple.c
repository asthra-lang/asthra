/**
 * Asthra Programming Language
 * Simple Lexer Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Simple lexer tests migrated to standardized test framework
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../framework/test_assertions.h"
#include "../framework/test_suite.h"
#include "lexer.h"

// Test basic token recognition
static AsthraTestResult test_basic_tokens(AsthraTestContext *ctx) {
    const char *source = "let x = 42;";

    Lexer *lexer = lexer_create(source, strlen(source), "test.ast");
    ASTHRA_TEST_ASSERT(ctx, lexer != NULL, "Lexer creation failed");

    Token token = lexer_next_token(lexer);
    ASTHRA_TEST_ASSERT(ctx, token.type == TOKEN_LET, "Expected LET token");

    token = lexer_next_token(lexer);
    ASTHRA_TEST_ASSERT(ctx, token.type == TOKEN_IDENTIFIER, "Expected IDENTIFIER token");

    token = lexer_next_token(lexer);
    ASTHRA_TEST_ASSERT(ctx, token.type == TOKEN_ASSIGN, "Expected ASSIGN token");

    token = lexer_next_token(lexer);
    ASTHRA_TEST_ASSERT(ctx, token.type == TOKEN_INTEGER, "Expected INTEGER token");

    token = lexer_next_token(lexer);
    ASTHRA_TEST_ASSERT(ctx, token.type == TOKEN_SEMICOLON, "Expected SEMICOLON token");

    token = lexer_next_token(lexer);
    ASTHRA_TEST_ASSERT(ctx, token.type == TOKEN_EOF, "Expected EOF token");

    lexer_destroy(lexer);
    return ASTHRA_TEST_PASS;
}

// Test keyword recognition
static AsthraTestResult test_keywords(AsthraTestContext *ctx) {
    const char *keywords[] = {"let", "fn", "if", "else", "return", NULL};
    TokenType expected[] = {TOKEN_LET, TOKEN_FN, TOKEN_IF, TOKEN_ELSE, TOKEN_RETURN};

    for (int i = 0; keywords[i] != NULL; i++) {
        Lexer *lexer = lexer_create(keywords[i], strlen(keywords[i]), "test.ast");
        ASTHRA_TEST_ASSERT(ctx, lexer != NULL, "Lexer creation failed");

        Token token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT(ctx, token.type == expected[i], "Keyword recognition failed");

        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

// Test operator recognition
static AsthraTestResult test_operators(AsthraTestContext *ctx) {
    const char *operators[] = {"+", "-", "*", "/", "==", "!=", NULL};
    TokenType expected[] = {TOKEN_PLUS,   TOKEN_MINUS, TOKEN_MULTIPLY,
                            TOKEN_DIVIDE, TOKEN_EQUAL, TOKEN_NOT_EQUAL};

    for (int i = 0; operators[i] != NULL; i++) {
        Lexer *lexer = lexer_create(operators[i], strlen(operators[i]), "test.ast");
        ASTHRA_TEST_ASSERT(ctx, lexer != NULL, "Lexer creation failed");

        Token token = lexer_next_token(lexer);
        ASTHRA_TEST_ASSERT(ctx, token.type == expected[i], "Operator recognition failed");

        lexer_destroy(lexer);
    }

    return ASTHRA_TEST_PASS;
}

// Test literal recognition
static AsthraTestResult test_literals(AsthraTestContext *ctx) {
    // Test integer
    Lexer *lexer = lexer_create("123", 3, "test.ast");
    ASTHRA_TEST_ASSERT(ctx, lexer != NULL, "Lexer creation failed");

    Token token = lexer_next_token(lexer);
    ASTHRA_TEST_ASSERT(ctx, token.type == TOKEN_INTEGER, "Integer literal recognition failed");

    lexer_destroy(lexer);

    // Test string
    lexer = lexer_create("\"hello\"", 7, "test.ast");
    ASTHRA_TEST_ASSERT(ctx, lexer != NULL, "Lexer creation failed");

    token = lexer_next_token(lexer);
    ASTHRA_TEST_ASSERT(ctx, token.type == TOKEN_STRING, "String literal recognition failed");

    lexer_destroy(lexer);

    return ASTHRA_TEST_PASS;
}

int main(void) {
    // Create lightweight test suite for simple lexer testing
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Simple Lexer Tests");

    // Add all test functions to the suite
    asthra_test_suite_add_test(suite, "basic_tokens", "Test basic token recognition",
                               test_basic_tokens);
    asthra_test_suite_add_test(suite, "keywords", "Test keyword recognition", test_keywords);
    asthra_test_suite_add_test(suite, "operators", "Test operator recognition", test_operators);
    asthra_test_suite_add_test(suite, "literals", "Test literal recognition", test_literals);

    // Run the test suite and exit with appropriate code
    return asthra_test_suite_run_and_exit(suite);
}
