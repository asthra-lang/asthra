/**
 * Asthra Programming Language
 * Lexer Test Utilities
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Shared utilities for lexer tests to avoid code duplication
 */

#ifndef LEXER_TEST_UTILS_H
#define LEXER_TEST_UTILS_H

#include "../framework/test_framework.h"
#include "lexer.h"

// =============================================================================
// SHARED TEST UTILITIES
// =============================================================================

/**
 * Helper function to skip whitespace and newline tokens
 */
static inline bool is_whitespace_token(TokenType type) {
    return type == TOKEN_WHITESPACE || type == TOKEN_NEWLINE;
}

/**
 * Helper function to skip whitespace, newlines, and comments
 */
static inline bool is_skippable_token(TokenType type) {
    return type == TOKEN_WHITESPACE || type == TOKEN_NEWLINE || type == TOKEN_COMMENT;
}

/**
 * Create a test lexer with standard error handling (wrapper for framework function)
 */
static inline Lexer* create_test_lexer_with_context(AsthraTestContext *context, const char* input, const char* filename) {
    Lexer* lexer = create_test_lexer(input, filename);
    if (!asthra_test_assert_pointer(context, lexer, "Failed to create lexer for input: %s", filename)) {
        return NULL;
    }
    return lexer;
}

/**
 * Count tokens of a specific type in input
 */
static inline size_t count_tokens_of_type(Lexer* lexer, TokenType target_type) {
    size_t count = 0;
    Token token;
    
    do {
        token = lexer_next_token(lexer);
        if (token.type == target_type) {
            count++;
        }
        token_free(&token);
    } while (token.type != TOKEN_EOF);
    
    return count;
}

/**
 * Verify that braces are balanced in a token stream
 */
static inline int check_brace_balance(Lexer* lexer) {
    int balance = 0;
    Token token;
    
    do {
        token = lexer_next_token(lexer);
        if (token.type == TOKEN_LEFT_BRACE) {
            balance++;
        } else if (token.type == TOKEN_RIGHT_BRACE) {
            balance--;
        }
        token_free(&token);
    } while (token.type != TOKEN_EOF);
    
    return balance;
}

#endif // LEXER_TEST_UTILS_H 
