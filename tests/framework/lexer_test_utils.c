/**
 * Asthra Programming Language
 * Lexer Test Utilities Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of test utility functions for lexer testing
 */

#include "lexer_test_utils.h"
#include "performance_test_utils.h"
#include <string.h>

// =============================================================================
// LEXER TEST UTILITIES
// =============================================================================

Lexer* create_test_lexer(const char* source, const char* filename) {
    TestLexerConfig config = {
        .source_code = source,
        .filename = filename ? filename : "test.ast",
        .enable_location_tracking = true,
        .enable_comment_preservation = false,
        .buffer_size = strlen(source) + 1
    };
    return create_test_lexer_with_config(&config);
}

Lexer* create_test_lexer_with_config(const TestLexerConfig* config) {
    if (!config || !config->source_code) {
        return NULL;
    }

    // Use the actual lexer creation function
    Lexer* lexer = lexer_create(config->source_code, strlen(config->source_code), config->filename);
    if (lexer) {
        track_memory_allocation(sizeof(Lexer));
    }
    return lexer;
}

void destroy_test_lexer(Lexer* lexer) {
    if (lexer) {
        track_memory_deallocation(sizeof(Lexer));
        lexer_destroy(lexer);
    }
}

Token* tokenize_test_source(const char* source, const char* filename, size_t* token_count) {
    if (!source || !token_count) {
        return NULL;
    }

    Lexer* lexer = create_test_lexer(source, filename);
    if (!lexer) {
        return NULL;
    }

    // Estimate token count and allocate array
    size_t estimated_tokens = strlen(source) / 5 + 10; // Rough estimate
    Token* tokens = malloc(sizeof(Token) * estimated_tokens);
    if (!tokens) {
        destroy_test_lexer(lexer);
        return NULL;
    }

    // Tokenize the source
    *token_count = 0;
    size_t capacity = estimated_tokens;

    Token token;
    do {
        token = lexer_next_token(lexer);

        // Resize array if needed
        if (*token_count >= capacity) {
            capacity *= 2;
            Token* new_tokens = realloc(tokens, sizeof(Token) * capacity);
            if (!new_tokens) {
                free(tokens);
                destroy_test_lexer(lexer);
                return NULL;
            }
            tokens = new_tokens;
        }

        tokens[*token_count] = token;
        (*token_count)++;

    } while (token.type != TOKEN_EOF);

    track_memory_allocation(sizeof(Token) * capacity);
    destroy_test_lexer(lexer);
    return tokens;
}
