/**
 * Asthra Programming Language Compiler
 * Token creation and management core functionality
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ast.h"
#include "lexer.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TOKEN CREATION AND MANAGEMENT
// =============================================================================

Token token_create(TokenType type, SourceLocation location) {
    Token token = {0};
    token.type = type;
    token.location = location;
    return token;
}

void token_free(Token *token) {
    if (!token)
        return;

    switch (token->type) {
    case TOKEN_STRING:
        if (token->data.string.value) {
            free(token->data.string.value);
            token->data.string.value = NULL;
        }
        break;
    case TOKEN_IDENTIFIER:
        if (token->data.identifier.name) {
            free(token->data.identifier.name);
            token->data.identifier.name = NULL;
        }
        break;

    default:
        break;
    }
}

Token token_clone(const Token *token) {
    if (!token) {
        Token empty = {0};
        return empty;
    }

    // Create clean token with only basic fields copied
    Token clone = {0};
    clone.type = token->type;
    clone.location = token->location;

    // Deep copy only the token types that have dynamically allocated data
    switch (token->type) {
    case TOKEN_STRING:
        if (token->data.string.value) {
            clone.data.string.value = malloc(token->data.string.length + 1);
            if (clone.data.string.value) {
                memcpy(clone.data.string.value, token->data.string.value,
                       token->data.string.length + 1);
                clone.data.string.length = token->data.string.length;
            }
        } else {
            clone.data.string.value = NULL;
            clone.data.string.length = 0;
        }
        break;
    case TOKEN_IDENTIFIER:
        if (token->data.identifier.name) {
            clone.data.identifier.name = malloc(token->data.identifier.length + 1);
            if (clone.data.identifier.name) {
                memcpy(clone.data.identifier.name, token->data.identifier.name,
                       token->data.identifier.length + 1);
                clone.data.identifier.length = token->data.identifier.length;
            }
        } else {
            clone.data.identifier.name = NULL;
            clone.data.identifier.length = 0;
        }
        break;

    default:
        // For all other token types (INTEGER, FLOAT, CHAR, etc.), copy the data directly
        // These don't use dynamically allocated memory, so direct copy is safe
        clone.data = token->data;
        break;
    }

    return clone;
}