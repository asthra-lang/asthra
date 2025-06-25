/**
 * Asthra Programming Language Compiler
 * Core lexical analyzer implementation for Asthra grammar
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "lexer_internal.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// LEXER IMPLEMENTATION
// =============================================================================

Lexer *lexer_create(const char *source, size_t source_length, const char *filename) {
    Lexer *lexer = malloc(sizeof(Lexer));
    if (!lexer)
        return NULL;

    lexer->source = source;
    lexer->source_length = source_length;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->filename = filename;
    lexer->has_peek = false;
    lexer->error_message = NULL;

    // Initialize current token
    lexer->current_token = token_create(TOKEN_EOF, current_location(lexer));

    return lexer;
}

void lexer_destroy(Lexer *lexer) {
    if (!lexer)
        return;

    token_free(&lexer->current_token);
    if (lexer->has_peek) {
        token_free(&lexer->peek_token);
    }
    if (lexer->error_message) {
        free(lexer->error_message);
    }
    free(lexer);
}

Token lexer_next_token(Lexer *lexer) {
    if (!lexer) {
        Token error = {0};
        error.type = TOKEN_ERROR;
        return error;
    }

    token_free(&lexer->current_token);

    if (lexer->has_peek) {
        lexer->current_token = lexer->peek_token;
        lexer->has_peek = false;
        lexer->peek_token = (Token){0};
    } else {
        lexer->current_token = scan_token(lexer);
    }

    // Return a cloned token to prevent double-free errors
    // The caller owns the returned token and must free it
    return token_clone(&lexer->current_token);
}

Token lexer_peek_token(Lexer *lexer) {
    if (!lexer) {
        Token error = {0};
        error.type = TOKEN_ERROR;
        return error;
    }

    if (!lexer->has_peek) {
        lexer->peek_token = scan_token(lexer);
        lexer->has_peek = true;
    }

    // Return a cloned token to prevent memory sharing issues
    return token_clone(&lexer->peek_token);
}

bool lexer_match(Lexer *lexer, TokenType expected) {
    if (!lexer)
        return false;
    return lexer->current_token.type == expected;
}

bool lexer_consume(Lexer *lexer, TokenType expected) {
    if (!lexer)
        return false;

    if (lexer->current_token.type == expected) {
        lexer_next_token(lexer);
        return true;
    }
    return false;
}

Token lexer_current_token(Lexer *lexer) {
    if (!lexer) {
        Token error = {0};
        error.type = TOKEN_ERROR;
        return error;
    }
    // Return a cloned token to prevent memory sharing issues
    return token_clone(&lexer->current_token);
}

bool lexer_at_eof(Lexer *lexer) {
    if (!lexer)
        return true;
    return lexer->current_token.type == TOKEN_EOF;
}

const char *lexer_get_error(Lexer *lexer) {
    if (!lexer)
        return "Invalid lexer";
    return lexer->error_message;
}
