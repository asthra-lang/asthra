/**
 * Asthra Programming Language
 * Lexer Test Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test utility functions for lexer testing
 */

#ifndef ASTHRA_LEXER_TEST_UTILS_H
#define ASTHRA_LEXER_TEST_UTILS_H

#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST HELPER STRUCTURES
// =============================================================================

/**
 * Test lexer configuration
 */
typedef struct {
    const char* source_code;
    const char* filename;
    bool enable_location_tracking;
    bool enable_comment_preservation;
    size_t buffer_size;
} TestLexerConfig;

/**
 * Token expectation for testing
 */
typedef struct {
    TokenType type;
    const char* value;
    size_t line;
    size_t column;
    const char* filename;
} TokenExpectation;

// =============================================================================
// LEXER TEST UTILITIES
// =============================================================================

/**
 * Create a test lexer with the given source code
 * @param source The source code to tokenize
 * @param filename The filename for error reporting
 * @return A new lexer state for testing
 */
Lexer* create_test_lexer(const char* source, const char* filename);

/**
 * Create a test lexer with custom configuration
 * @param config Test lexer configuration
 * @return A new lexer state for testing
 */
Lexer* create_test_lexer_with_config(const TestLexerConfig* config);

/**
 * Destroy a test lexer and clean up resources
 * @param lexer The lexer to destroy
 */
void destroy_test_lexer(Lexer* lexer);

/**
 * Tokenize source and return all tokens
 * @param source The source code to tokenize
 * @param filename The filename for error reporting
 * @param token_count Output parameter for number of tokens
 * @return Array of tokens (caller must free)
 */
Token* tokenize_test_source(const char* source, const char* filename, size_t* token_count);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LEXER_TEST_UTILS_H
