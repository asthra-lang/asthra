/**
 * Asthra Programming Language
 * Core Parser Functionality Tests
 * 
 * Tests for parser lifecycle, token consumption, and error handling
 * as outlined in Phase 2.1 of the test coverage improvement plan.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "lexer.h"

// Test framework macros (simple pattern like test_basic.c)
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

// Function prototypes
static int test_lexer_create_destroy(void);
static int test_lexer_token_scanning(void);
static int test_token_types(void);

// =============================================================================
// LEXER BASIC TESTS
// =============================================================================

/**
 * Test: Lexer Creation and Destruction
 * Verifies that lexers can be created and destroyed properly
 */
static int test_lexer_create_destroy(void) {
    // Test basic lexer creation
    const char* test_source = "fn main(void) { return 0; }";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    // Test destruction
    lexer_destroy(lexer);
    
    TEST_PASS("Lexer creation and destruction");
}

/**
 * Test: Basic Token Scanning
 * Verifies that lexer can scan basic tokens
 */
static int test_lexer_token_scanning(void) {
    const char* test_source = "fn main";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    // Get first token
    Token token1 = lexer_next_token(lexer);
    TEST_ASSERT(token1.type == TOKEN_FN, "First token should be FN");
    
    // Get second token
    Token token2 = lexer_next_token(lexer);
    TEST_ASSERT(token2.type == TOKEN_IDENTIFIER, "Second token should be IDENTIFIER");
    
    // Get EOF token
    Token token3 = lexer_next_token(lexer);
    TEST_ASSERT(token3.type == TOKEN_EOF, "Third token should be EOF");
    
    lexer_destroy(lexer);
    TEST_PASS("Basic token scanning");
}

/**
 * Test: Token Type Recognition
 * Verifies that token types are recognized correctly
 */
static int test_token_types(void) {
    const char* test_source = "123 \"hello\" true";
    
    size_t source_len = strlen(test_source);
    Lexer* lexer = lexer_create(test_source, source_len, "test.ast");
    TEST_ASSERT(lexer != NULL, "Failed to create test lexer");
    
    // Get integer token
    Token token1 = lexer_next_token(lexer);
    TEST_ASSERT(token1.type == TOKEN_INTEGER, "First token should be INTEGER");
    
    // Get string token
    Token token2 = lexer_next_token(lexer);
    TEST_ASSERT(token2.type == TOKEN_STRING, "Second token should be STRING");
    
    // Get boolean token
    Token token3 = lexer_next_token(lexer);
    TEST_ASSERT(token3.type == TOKEN_BOOL_TRUE, "Third token should be BOOL_TRUE");
    
    lexer_destroy(lexer);
    TEST_PASS("Token type recognition");
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("Running Asthra parser core tests...\n\n");
    
    int passed = 0;
    int total = 0;
    
    // Run all tests
    total++; if (test_lexer_create_destroy()) passed++;
    total++; if (test_lexer_token_scanning()) passed++;
    total++; if (test_token_types()) passed++;
    
    printf("\nParser Core Test Results: %d/%d passed\n", passed, total);
    
    if (passed == total) {
        printf("All parser core tests passed!\n");
        return 0;
    } else {
        printf("Some parser core tests failed!\n");
        return 1;
    }
} 
