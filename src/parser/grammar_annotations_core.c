/**
 * Asthra Programming Language Compiler - Core Annotation Parsing
 * Core annotation parsing logic and bracketed annotation support
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CORE ANNOTATION PARSING
// =============================================================================

ASTNode *parse_annotation(Parser *parser) {
    if (!parser)
        return NULL;

    // Add helpful error message for @ annotations
    if (match_token(parser, TOKEN_AT)) {
        report_error(parser, "@ annotations are not supported. Use #[...] syntax instead. "
                             "Example: Use #[ownership(gc)] instead of @gc. "
                             "See docs/AT_ANNOTATIONS_MIGRATION.md for migration guide.");
        return NULL;
    } else if (check_token(parser, TOKEN_HASH)) {
        // Look ahead to see if this is a bracketed annotation #[...] or simple ownership tag #gc
        Token next_token = peek_token(parser);
        if (next_token.type == TOKEN_LEFT_BRACKET) {
            return parse_bracketed_annotation(parser);
        } else {
            return parse_ownership_annotation(parser);
        }
    }

    return NULL;
}

// parse_semantic_annotation function removed - @ annotations no longer supported
// Use #[...] syntax instead. See docs/AT_ANNOTATIONS_MIGRATION.md for migration guide.

// NEW: Parse bracketed annotations like #[deprecated], #[doc("text")], etc.
ASTNode *parse_bracketed_annotation(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Consume the # token
    if (!expect_token(parser, TOKEN_HASH)) {
        return NULL;
    }

    // Consume the [ token
    if (!expect_token(parser, TOKEN_LEFT_BRACKET)) {
        return NULL;
    }

    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected annotation name");
        return NULL;
    }

    char *annotation_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

    // Parse optional parameters according to grammar:
    // AnnotationParams <- AnnotationParam (',' AnnotationParam)* / 'none'
    // AnnotationParam <- IDENT '=' AnnotationValue
    // AnnotationValue <- STRING / IDENT / INT / BOOL
    // Special case: ownership(value) format for ownership annotations
    char *parameters = NULL;
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        advance_token(parser);

        // Check for 'none' marker
        if (match_token(parser, TOKEN_IDENTIFIER) &&
            strcmp(parser->current_token.data.identifier.name, "none") == 0) {
            advance_token(parser);
            if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                free(annotation_name);
                return NULL;
            }
            // parameters remains NULL for 'none'
        } else {
            // Special handling for ownership annotation: ownership(gc|c|pinned)
            if (strcmp(annotation_name, "ownership") == 0) {
                if (!match_token(parser, TOKEN_IDENTIFIER)) {
                    report_error(parser, "Expected ownership type (gc, c, or pinned)");
                    free(annotation_name);
                    return NULL;
                }

                const char *ownership_type = parser->current_token.data.identifier.name;
                if (strcmp(ownership_type, "gc") != 0 && strcmp(ownership_type, "c") != 0 &&
                    strcmp(ownership_type, "pinned") != 0) {
                    report_error(parser, "Unknown ownership type. Expected 'gc', 'c', or 'pinned'");
                    free(annotation_name);
                    return NULL;
                }

                parameters = strdup(ownership_type);
                advance_token(parser);

                if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                    free(annotation_name);
                    free(parameters);
                    return NULL;
                }
            } else {
                // Parse standard AnnotationParam (',' AnnotationParam)*
                char param_buffer[1024] = {0};
                bool first_param = true;

                while (!match_token(parser, TOKEN_RIGHT_PAREN) && !at_end(parser)) {
                    if (!first_param) {
                        if (!expect_token(parser, TOKEN_COMMA)) {
                            free(annotation_name);
                            return NULL;
                        }
                        strcat(param_buffer, ", ");
                    }

                    // Parse IDENT '=' AnnotationValue
                    if (!match_token(parser, TOKEN_IDENTIFIER)) {
                        report_error(parser, "Expected parameter name");
                        free(annotation_name);
                        return NULL;
                    }

                    strcat(param_buffer, parser->current_token.data.identifier.name);
                    advance_token(parser);

                    if (!expect_token(parser, TOKEN_ASSIGN)) {
                        free(annotation_name);
                        return NULL;
                    }
                    strcat(param_buffer, "=");

                    // Parse AnnotationValue <- STRING / IDENT / INT / BOOL
                    if (match_token(parser, TOKEN_STRING)) {
                        strcat(param_buffer, "\"");
                        strcat(param_buffer, parser->current_token.data.string.value);
                        strcat(param_buffer, "\"");
                        advance_token(parser);
                    } else if (match_token(parser, TOKEN_IDENTIFIER)) {
                        strcat(param_buffer, parser->current_token.data.identifier.name);
                        advance_token(parser);
                    } else if (match_token(parser, TOKEN_INTEGER)) {
                        char int_str[32];
                        snprintf(int_str, sizeof(int_str), "%lld",
                                 (long long)parser->current_token.data.integer.value);
                        strcat(param_buffer, int_str);
                        advance_token(parser);
                    } else if (match_token(parser, TOKEN_BOOL_TRUE) ||
                               match_token(parser, TOKEN_BOOL_FALSE)) {
                        strcat(param_buffer,
                               parser->current_token.type == TOKEN_BOOL_TRUE ? "true" : "false");
                        advance_token(parser);
                    } else {
                        report_error(
                            parser,
                            "Expected annotation value (string, identifier, integer, or boolean)");
                        free(annotation_name);
                        return NULL;
                    }

                    first_param = false;
                }

                if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                    free(annotation_name);
                    return NULL;
                }

                // Store the parsed parameters
                if (strlen(param_buffer) > 0) {
                    parameters = strdup(param_buffer);
                }
            }
        }
    }

    if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
        free(annotation_name);
        free(parameters);
        return NULL;
    }

    // Check if this is an ownership annotation and create the appropriate node type
    if (strcmp(annotation_name, "ownership") == 0 && parameters) {
        // Create AST_OWNERSHIP_TAG node for ownership annotations
        ASTNode *node = ast_create_node(AST_OWNERSHIP_TAG, start_loc);
        if (!node) {
            free(annotation_name);
            free(parameters);
            return NULL;
        }

        // Set ownership type based on parameter
        OwnershipType ownership;
        if (strcmp(parameters, "gc") == 0) {
            ownership = OWNERSHIP_GC;
        } else if (strcmp(parameters, "c") == 0) {
            ownership = OWNERSHIP_C;
        } else if (strcmp(parameters, "pinned") == 0) {
            ownership = OWNERSHIP_PINNED;
        } else {
            // This shouldn't happen as we validated earlier
            ownership = OWNERSHIP_GC;
        }

        node->data.ownership_tag.ownership = ownership;
        free(annotation_name);
        free(parameters);
        return node;
    }

    // Create a semantic tag node for general annotations
    ASTNode *node = ast_create_node(AST_SEMANTIC_TAG, start_loc);
    if (!node) {
        free(annotation_name);
        free(parameters);
        return NULL;
    }

    node->data.semantic_tag.name = annotation_name;

    // Store parameters in params list if we have any
    if (parameters) {
        // For now, we'll create a simple parameter list with one string node
        // This is a simplified implementation - a full implementation would parse individual
        // parameters
        node->data.semantic_tag.params = ast_node_list_create(1);
        if (node->data.semantic_tag.params) {
            // Create a simple string node to hold the parameters
            ASTNode *param_node = ast_create_node(AST_STRING_LITERAL, start_loc);
            if (param_node) {
                param_node->data.string_literal.value = parameters;
                ast_node_list_add(&node->data.semantic_tag.params, param_node);
            } else {
                free(parameters);
            }
        } else {
            free(parameters);
        }
    } else {
        node->data.semantic_tag.params = NULL;
    }

    return node;
}