/**
 * Asthra Programming Language Compiler - Struct Literal Expressions
 * Parsing of struct literal initialization expressions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// STRUCT LITERAL PARSING
// =============================================================================

ASTNode *parse_struct_literal_with_name(Parser *parser, char *struct_name,
                                        SourceLocation start_loc) {
    return parse_struct_literal_with_name_and_type_args(parser, struct_name, NULL, start_loc);
}

ASTNode *parse_struct_literal_with_name_and_type_args(Parser *parser, char *struct_name,
                                                      ASTNodeList *type_args,
                                                      SourceLocation start_loc) {
    if (!parser || !struct_name) {
        if (struct_name)
            free(struct_name);
        if (type_args)
            ast_node_list_destroy(type_args);
        return NULL;
    }

    if (!expect_token(parser, TOKEN_LEFT_BRACE)) {
        free(struct_name);
        if (type_args)
            ast_node_list_destroy(type_args);
        return NULL;
    }

    // Parse field initializations
    ASTNode **field_inits = NULL;
    size_t field_count = 0;
    size_t field_capacity = 4;

    field_inits = malloc(field_capacity * sizeof(ASTNode *));
    if (!field_inits) {
        free(struct_name);
        if (type_args)
            ast_node_list_destroy(type_args);
        return NULL;
    }

    while (!check_token(parser, TOKEN_RIGHT_BRACE) && !at_end(parser)) {
        // Parse field initialization: field_name: value
        if (!check_token(parser, TOKEN_IDENTIFIER)) {
            report_error(parser, "Expected field name in struct literal");
            for (size_t i = 0; i < field_count; i++) {
                ast_free_node(field_inits[i]);
            }
            free(field_inits);
            free(struct_name);
            if (type_args)
                ast_node_list_destroy(type_args);
            return NULL;
        }

        char *field_name = strdup(parser->current_token.data.identifier.name);
        SourceLocation field_loc = parser->current_token.location;
        advance_token(parser);

        if (!expect_token(parser, TOKEN_COLON)) {
            free(field_name);
            for (size_t i = 0; i < field_count; i++) {
                ast_free_node(field_inits[i]);
            }
            free(field_inits);
            free(struct_name);
            if (type_args)
                ast_node_list_destroy(type_args);
            return NULL;
        }

        ASTNode *value = parse_expr(parser);
        if (!value) {
            free(field_name);
            for (size_t i = 0; i < field_count; i++) {
                ast_free_node(field_inits[i]);
            }
            free(field_inits);
            free(struct_name);
            if (type_args)
                ast_node_list_destroy(type_args);
            return NULL;
        }

        // Create assignment node for field initialization
        ASTNode *field_init = ast_create_node(AST_ASSIGNMENT, field_loc);
        if (!field_init) {
            free(field_name);
            ast_free_node(value);
            for (size_t i = 0; i < field_count; i++) {
                ast_free_node(field_inits[i]);
            }
            free(field_inits);
            free(struct_name);
            if (type_args)
                ast_node_list_destroy(type_args);
            return NULL;
        }

        // Create identifier node for field name
        ASTNode *field_ident = ast_create_node(AST_IDENTIFIER, field_loc);
        if (!field_ident) {
            free(field_name);
            ast_free_node(value);
            ast_free_node(field_init);
            for (size_t i = 0; i < field_count; i++) {
                ast_free_node(field_inits[i]);
            }
            free(field_inits);
            free(struct_name);
            if (type_args)
                ast_node_list_destroy(type_args);
            return NULL;
        }

        field_ident->data.identifier.name = field_name;
        field_init->data.assignment.target = field_ident;
        field_init->data.assignment.value = value;

        // Add to field_inits array
        if (field_count >= field_capacity) {
            field_capacity *= 2;
            field_inits = realloc(field_inits, field_capacity * sizeof(ASTNode *));
            if (!field_inits) {
                ast_free_node(field_init);
                free(struct_name);
                if (type_args)
                    ast_node_list_destroy(type_args);
                return NULL;
            }
        }

        field_inits[field_count++] = field_init;

        // Parse comma between field initializations (v1.12: no trailing comma allowed)
        if (match_token(parser, TOKEN_COMMA)) {
            advance_token(parser);
            // After comma, we must have another field initialization (no trailing comma)
            if (match_token(parser, TOKEN_RIGHT_BRACE)) {
                report_error(parser, "Trailing comma not allowed in struct literals");
                for (size_t i = 0; i < field_count; i++) {
                    ast_free_node(field_inits[i]);
                }
                free(field_inits);
                free(struct_name);
                if (type_args)
                    ast_node_list_destroy(type_args);
                return NULL;
            }
        } else if (!match_token(parser, TOKEN_RIGHT_BRACE)) {
            report_error(parser,
                         "Expected ',' between field initializations or '}' after last field");
            for (size_t i = 0; i < field_count; i++) {
                ast_free_node(field_inits[i]);
            }
            free(field_inits);
            free(struct_name);
            if (type_args)
                ast_node_list_destroy(type_args);
            return NULL;
        } else {
            break;
        }
    }

    if (!expect_token(parser, TOKEN_RIGHT_BRACE)) {
        for (size_t i = 0; i < field_count; i++) {
            ast_free_node(field_inits[i]);
        }
        free(field_inits);
        free(struct_name);
        if (type_args)
            ast_node_list_destroy(type_args);
        return NULL;
    }

    // Create struct literal node
    ASTNode *struct_literal = ast_create_node(AST_STRUCT_LITERAL, start_loc);
    if (!struct_literal) {
        for (size_t i = 0; i < field_count; i++) {
            ast_free_node(field_inits[i]);
        }
        free(field_inits);
        free(struct_name);
        if (type_args)
            ast_node_list_destroy(type_args);
        return NULL;
    }

    struct_literal->data.struct_literal.struct_name = struct_name;
    struct_literal->data.struct_literal.type_args = type_args;

    // Convert field_inits array to ASTNodeList
    if (field_inits && field_count > 0) {
        struct_literal->data.struct_literal.field_inits = ast_node_list_create(field_count);
        if (struct_literal->data.struct_literal.field_inits) {
            for (size_t i = 0; i < field_count; i++) {
                ast_node_list_add(&struct_literal->data.struct_literal.field_inits, field_inits[i]);
            }
        }
        free(field_inits);
    } else {
        struct_literal->data.struct_literal.field_inits = NULL;
    }

    return struct_literal;
}
