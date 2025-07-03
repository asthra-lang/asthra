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
    // We've already consumed the dot or double colon, so parse the variant
    char *variant_name = NULL;
    if (match_token(parser, TOKEN_IDENTIFIER)) {
        variant_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);
    } else if (match_token(parser, TOKEN_NONE)) {
        // Allow "none" as a variant name even though it's a reserved keyword
        variant_name = strdup("none");
        advance_token(parser);
    } else {
        report_error(parser, "Expected variant name after '.' or '::'");
        free(name);
        return NULL;
    }

    char *binding = NULL;
    ASTNode *pattern = NULL;
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        advance_token(parser);

        // Parse according to current grammar: PatternArgs <- Pattern (',' Pattern)* /
        // 'none'
        if (match_token(parser, TOKEN_NONE)) {
            // Explicit none for empty pattern arguments (current grammar semantic clarity)
            advance_token(parser);
            binding = NULL; // No binding
            pattern = NULL;
        } else if (match_token(parser, TOKEN_VOID)) {
            // Legacy compatibility error with helpful message
            report_error(parser, "Unexpected 'void' in pattern arguments. Use 'none' for "
                                 "empty pattern arguments per current grammar");
            free(name);
            free(variant_name);
            return NULL;
        } else {
            // Parse nested pattern recursively
            pattern = parse_pattern(parser);
            if (!pattern) {
                free(name);
                free(variant_name);
                return NULL;
            }

            // For simple identifier patterns, extract the binding name for backward compatibility
            if (pattern->type == AST_IDENTIFIER) {
                binding = strdup(pattern->data.identifier.name);
            }
        }

        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
            free(name);
            free(variant_name);
            if (binding)
                free(binding);
            if (pattern)
                ast_free_node(pattern);
            return NULL;
        }
    }

    ASTNode *node = ast_create_node(AST_ENUM_PATTERN, start_loc);
    if (!node) {
        free(name);
        free(variant_name);
        if (binding)
            free(binding);
        if (pattern)
            ast_free_node(pattern);
        return NULL;
    }

    node->data.enum_pattern.enum_name = name;
    node->data.enum_pattern.variant_name = variant_name;
    node->data.enum_pattern.binding = binding;
    node->data.enum_pattern.pattern = pattern;

    return node;
}

ASTNode *parse_enum_pattern(Parser *parser) {
    // This is handled within parse_pattern
    return parse_pattern(parser);
}