/**
 * Asthra Programming Language Compiler - Expression Utilities
 * Utility functions for expression parsing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// EXPRESSION UTILITIES
// =============================================================================

bool is_expression_start(Parser *parser) {
    if (!parser)
        return false;

    switch (parser->current_token.type) {
    case TOKEN_BOOL_TRUE:
    case TOKEN_BOOL_FALSE:
    case TOKEN_INTEGER:
    case TOKEN_FLOAT:
    case TOKEN_STRING:
    case TOKEN_CHAR:
    case TOKEN_IDENTIFIER:
    case TOKEN_LEFT_PAREN:
    case TOKEN_LEFT_BRACKET:
    case TOKEN_MINUS:
    case TOKEN_LOGICAL_NOT:
    case TOKEN_BITWISE_NOT:
    case TOKEN_MULTIPLY:
    case TOKEN_BITWISE_AND:
    case TOKEN_SIZEOF:
    case TOKEN_AWAIT:
        // Note: TOKEN_RECV moved to stdlib (Tier 3)
        return true;
    default:
        return false;
    }
}
