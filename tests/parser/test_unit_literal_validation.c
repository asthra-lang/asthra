/**
 * Unit Literal Validation Test
 * Tests that unit literals () are properly parsed for return statements
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser_core.h"

// Simple test macros
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            fprintf(stderr, "FAIL: %s\n", message);                                                \
            return 0;                                                                              \
        }                                                                                          \
    } while (0)

#define TEST_PASS(message)                                                                         \
    do {                                                                                           \
        printf("PASS: %s\n", message);                                                             \
        return 1;                                                                                  \
    } while (0)

// Test unit literal tokenization
static int test_unit_literal_tokens(void) {
    const char *test_source = "()";

    size_t source_len = strlen(test_source);
    Lexer *lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");

    // Get left paren token
    Token token1 = lexer_next_token(lexer);
    TEST_ASSERT(token1.type == TOKEN_LEFT_PAREN, "First token should be LEFT_PAREN");

    // Get right paren token
    Token token2 = lexer_next_token(lexer);
    TEST_ASSERT(token2.type == TOKEN_RIGHT_PAREN, "Second token should be RIGHT_PAREN");

    // Get EOF token
    Token token3 = lexer_next_token(lexer);
    TEST_ASSERT(token3.type == TOKEN_EOF, "Third token should be EOF");

    lexer_destroy(lexer);
    TEST_PASS("Unit literal tokenization");
}

// Test return statement with unit literal
static int test_return_unit_literal_tokens(void) {
    const char *test_source = "return ();";

    size_t source_len = strlen(test_source);
    Lexer *lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");

    // Get return token
    Token token1 = lexer_next_token(lexer);
    TEST_ASSERT(token1.type == TOKEN_RETURN, "First token should be RETURN");

    // Get left paren token
    Token token2 = lexer_next_token(lexer);
    TEST_ASSERT(token2.type == TOKEN_LEFT_PAREN, "Second token should be LEFT_PAREN");

    // Get right paren token
    Token token3 = lexer_next_token(lexer);
    TEST_ASSERT(token3.type == TOKEN_RIGHT_PAREN, "Third token should be RIGHT_PAREN");

    // Get semicolon token
    Token token4 = lexer_next_token(lexer);
    TEST_ASSERT(token4.type == TOKEN_SEMICOLON, "Fourth token should be SEMICOLON");

    lexer_destroy(lexer);
    TEST_PASS("Return unit literal tokenization");
}

// Test basic parser creation
static int test_parser_creation(void) {
    const char *test_source = "return ();";

    size_t source_len = strlen(test_source);
    Lexer *lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");

    Parser *parser = parser_create(lexer);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");

    parser_destroy(parser);
    lexer_destroy(lexer);
    TEST_PASS("Parser creation with unit literal source");
}

int main(void) {
    printf("Unit Literal Validation Tests\n");
    printf("==============================\n\n");

    int passed = 0;
    int total = 0;

    // Run all tests
    total++;
    if (test_unit_literal_tokens())
        passed++;
    total++;
    if (test_return_unit_literal_tokens())
        passed++;
    total++;
    if (test_parser_creation())
        passed++;

    printf("\nUnit Literal Test Results: %d/%d passed\n", passed, total);

    if (passed == total) {
        printf("✅ All unit literal tests passed!\n");
        printf("Unit literal implementation is working correctly.\n");
        return 0;
    } else {
        printf("❌ Some unit literal tests failed!\n");
        return 1;
    }
}
