/**
 * Asthra Programming Language Compiler - Enum Pattern Grammar Implementation
 * Enum pattern parsing functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_patterns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// ENUM PATTERN PARSING
// =============================================================================

ASTNode *parse_enum_pattern_impl(Parser *parser, char *name, SourceLocation start_loc) {
    // We've already consumed the dot, so parse the variant
    advance_token(parser);

    char *variant_name = NULL;
    if (match_token(parser, TOKEN_IDENTIFIER)) {
        variant_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);
    } else if (match_token(parser, TOKEN_NONE)) {
        // Allow "none" as a variant name even though it's a reserved keyword
        variant_name = strdup("none");
        advance_token(parser);
    } else {
        report_error(parser, "Expected variant name after '.'");
        free(name);
        return NULL;
    }

    char *binding = NULL;
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        advance_token(parser);

        // Parse according to current grammar: PatternArgs <- Pattern (',' Pattern)* /
        // 'none'
        if (match_token(parser, TOKEN_NONE)) {
            // Explicit none for empty pattern arguments (current grammar semantic clarity)
            advance_token(parser);
            binding = NULL; // No binding
        } else if (match_token(parser, TOKEN_VOID)) {
            // Legacy compatibility error with helpful message
            report_error(parser, "Unexpected 'void' in pattern arguments. Use 'none' for "
                                 "empty pattern arguments per current grammar");
            free(name);
            free(variant_name);
            return NULL;
        } else if (match_token(parser, TOKEN_IDENTIFIER)) {
            // Parse actual pattern binding
            binding = strdup(parser->current_token.data.identifier.name);
            advance_token(parser);
        }

        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
            free(name);
            free(variant_name);
            if (binding)
                free(binding);
            return NULL;
        }
    }

    ASTNode *node = ast_create_node(AST_ENUM_PATTERN, start_loc);
    if (!node) {
        free(name);
        free(variant_name);
        if (binding)
            free(binding);
        return NULL;
    }

    node->data.enum_pattern.enum_name = name;
    node->data.enum_pattern.variant_name = variant_name;
    node->data.enum_pattern.binding = binding;

    return node;
}

ASTNode *parse_enum_pattern(Parser *parser) {
    // This is handled within parse_pattern
    return parse_pattern(parser);
}