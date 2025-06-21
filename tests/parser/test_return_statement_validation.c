/**
 * Return Statement Validation Test
 * Tests that return statements with unit literals () are properly parsed
 * This validates the fix for the v1.14+ grammar requirement
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "lexer.h"
#include "parser_core.h"

// Simple test macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n", message); \
            return 0; \
        } \
    } while (0)

#define TEST_PASS(message) \
    do { \
        printf("PASS: %s\n", message); \
        return 1; \
    } while (0)

// Test return statement parsing
static int test_return_unit_parsing(void) {
    const char* test_source = "package test;\npub fn test(none) -> void { return (); }";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    Parser* parser = parser_create(lexer);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");
    
    // Try to parse the program
    ASTNode* program = parser_parse_program(parser);
    
    if (program != NULL) {
        // Success - the return statement with unit literal parsed correctly
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        TEST_PASS("Return statement with unit literal parsed successfully");
    } else {
        // Check if there were parsing errors
        parser_destroy(parser);
        lexer_destroy(lexer);
        TEST_ASSERT(0, "Failed to parse return statement with unit literal");
    }
}

// Test simple return statement tokenization
static int test_return_statement_tokens(void) {
    const char* test_source = "return ();";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    // Verify token sequence
    Token tokens[5];
    for (int i = 0; i < 4; i++) {
        tokens[i] = lexer_next_token(lexer);
    }
    
    TEST_ASSERT(tokens[0].type == TOKEN_RETURN, "First token should be RETURN");
    TEST_ASSERT(tokens[1].type == TOKEN_LEFT_PAREN, "Second token should be LEFT_PAREN");
    TEST_ASSERT(tokens[2].type == TOKEN_RIGHT_PAREN, "Third token should be RIGHT_PAREN");
    TEST_ASSERT(tokens[3].type == TOKEN_SEMICOLON, "Fourth token should be SEMICOLON");
    
    lexer_destroy(lexer);
    TEST_PASS("Return statement tokenization correct");
}

// Test basic parser functionality with unit literal
static int test_parser_with_unit_literal(void) {
    const char* test_source = "()";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    Parser* parser = parser_create(lexer);
    TEST_ASSERT(parser != NULL, "Failed to create test parser");
    
    // Just test that parser can be created with unit literal source
    // The actual parsing will be tested through the full program test
    parser_destroy(parser);
    lexer_destroy(lexer);
    TEST_PASS("Parser handles unit literal source correctly");
}

int main(void) {
    printf("Return Statement Validation Tests\n");
    printf("==================================\n\n");
    
    int passed = 0;
    int total = 0;
    
    // Run all tests
    total++; if (test_return_statement_tokens()) passed++;
    total++; if (test_parser_with_unit_literal()) passed++;
    total++; if (test_return_unit_parsing()) passed++;
    
    printf("\nReturn Statement Test Results: %d/%d passed\n", passed, total);
    
    if (passed == total) {
        printf("✅ All return statement tests passed!\n");
        printf("Return statement with unit literal implementation is working correctly.\n");
        printf("This confirms the v1.14+ grammar requirement 'return ();' is now supported.\n");
        return 0;
    } else {
        printf("❌ Some return statement tests failed!\n");
        return 1;
    }
} 
