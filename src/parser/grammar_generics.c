/**
 * Asthra Programming Language Compiler - Generic Type Parsing
 * Parsing of generic types, generic enum constructors, and generic associated functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include "grammar_statements.h"
#include "parser_ast_helpers.h"
#include <stdlib.h>
#include <string.h>

/**
 * Parse generic enum constructor with type arguments
 */
static ASTNode *parse_generic_enum_constructor(Parser *parser, char *enum_name,
                                               ASTNodeList *type_args, SourceLocation start_loc) {
    if (!parser || !enum_name || !type_args)
        return NULL;

    advance_token(parser); // consume '.'

    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected variant name after '.' in generic enum constructor");
        return NULL;
    }

    char *variant_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

    // Check for optional arguments: ('(' ArgList ')')?
    ASTNode *value = NULL;
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        advance_token(parser); // consume '('

        // Parse single argument if present
        if (!match_token(parser, TOKEN_RIGHT_PAREN)) {
            value = parse_expr(parser);
            if (!value) {
                free(variant_name);
                return NULL;
            }

            // Check for additional arguments (not supported)
            if (match_token(parser, TOKEN_COMMA)) {
                report_error(parser, "Enum constructors currently support only single values. Use "
                                     "a tuple for multiple values.");
                ast_free_node(value);
                free(variant_name);
                return NULL;
            }
        }

        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
            if (value)
                ast_free_node(value);
            free(variant_name);
            return NULL;
        }
    }

    // Create enum type node to hold the generic type info
    ASTNode *enum_type_node = ast_create_node(AST_ENUM_TYPE, start_loc);
    if (!enum_type_node) {
        if (value)
            ast_free_node(value);
        free(variant_name);
        return NULL;
    }

    enum_type_node->data.enum_type.name = strdup(enum_name);
    enum_type_node->data.enum_type.type_args = type_args;

    // Create enum variant node
    ASTNode *node = ast_create_node(AST_ENUM_VARIANT, start_loc);
    if (!node) {
        ast_free_node(enum_type_node);
        if (value)
            ast_free_node(value);
        free(variant_name);
        return NULL;
    }

    node->data.enum_variant.enum_name = strdup(enum_name);
    node->data.enum_variant.variant_name = variant_name;
    node->data.enum_variant.value = value;

    // Store the enum type node as a child (this is a workaround to preserve type args)
    // In a proper implementation, we'd add a type_args field to enum_variant
    if (value) {
        // Create a wrapper node to hold both the type info and value
        // For now, we'll just use the simple enum_variant without generic type info
        // TODO: Extend AST to properly support generic enum constructors
        ast_free_node(enum_type_node);
    } else {
        // If there's no value, we can repurpose the value field to hold type info
        // This is a temporary solution
        node->data.enum_variant.value = enum_type_node;
    }

    return node;
}

/**
 * Try to parse generic type arguments and restore parser state if unsuccessful
 */
static ASTNodeList *try_parse_generic_type_args(Parser *parser, bool *success) {
    if (!parser || !success)
        return NULL;

    *success = false;

    // Save parser state for backtracking
    size_t saved_position = parser->lexer->position;
    size_t saved_line = parser->lexer->line;
    size_t saved_column = parser->lexer->column;
    Token saved_current = parser->current_token;
    Token saved_peek = parser->lexer->peek_token;
    bool saved_has_peek = parser->lexer->has_peek;

    advance_token(parser); // consume '<'

    // Try to parse as generic type arguments
    ASTNodeList *type_args = ast_node_list_create(2);
    bool parsed_successfully = false;

    if (type_args) {
        do {
            ASTNode *type_arg = parse_type(parser);
            if (!type_arg) {
                // Failed to parse as type - this is likely a binary expression
                break;
            }

            ast_node_list_add(&type_args, type_arg);

            if (match_token(parser, TOKEN_COMMA)) {
                advance_token(parser);
            } else {
                break;
            }
        } while (!at_end(parser));

        // Check if we can successfully close the generic type
        if (match_token(parser, TOKEN_GREATER_THAN)) {
            advance_token(parser);
            parsed_successfully = true;
        }
    }

    if (!parsed_successfully) {
        // Failed to parse as generic type - restore parser state
        if (type_args) {
            ast_node_list_destroy(type_args);
            type_args = NULL;
        }

        // Restore parser state
        parser->lexer->position = saved_position;
        parser->lexer->line = saved_line;
        parser->lexer->column = saved_column;
        parser->current_token = saved_current;
        parser->lexer->peek_token = saved_peek;
        parser->lexer->has_peek = saved_has_peek;
    }

    *success = parsed_successfully;
    return type_args;
}

/**
 * Parse identifier with potential generic type arguments
 */
ASTNode *parse_identifier_with_generics(Parser *parser, char *name, SourceLocation start_loc) {
    if (!parser || !name || !match_token(parser, TOKEN_LESS_THAN)) {
        return NULL; // Not a generic type
    }

    // Try to parse generic type arguments with backtracking
    bool is_generic_type = false;
    ASTNodeList *type_args = try_parse_generic_type_args(parser, &is_generic_type);

    if (!is_generic_type) {
        // Failed to parse as generic type - return NULL to let caller handle as regular identifier
        return NULL;
    }

    // Successfully parsed as generic type

    // Check for enum constructor with generic type: Enum<T>.Variant
    if (match_token(parser, TOKEN_DOT)) {
        ASTNode *enum_node = parse_generic_enum_constructor(parser, name, type_args, start_loc);
        if (!enum_node && type_args) {
            ast_node_list_destroy(type_args);
        }
        return enum_node;
    }
    // Check for associated function call with generic type: GenericType<T>::function()
    else if (match_token(parser, TOKEN_DOUBLE_COLON)) {
        advance_token(parser); // consume '::'

        if (!match_token(parser, TOKEN_IDENTIFIER)) {
            report_error(parser, "Expected function name after '::'");
            ast_node_list_destroy(type_args);
            return NULL;
        }

        char *function_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);

        // Create associated function call with generic type
        ASTNode *node = ast_create_node(AST_ASSOCIATED_FUNC_CALL, start_loc);
        if (!node) {
            free(function_name);
            ast_node_list_destroy(type_args);
            return NULL;
        }

        node->data.associated_func_call.struct_name = strdup(name);
        node->data.associated_func_call.function_name = function_name;
        node->data.associated_func_call.type_args = type_args; // Store generic args
        node->data.associated_func_call.args = NULL;           // Will be filled by postfix parsing

        return node;
    }

    // Create a struct type node (this could become a struct literal in postfix parsing)
    ASTNode *node = ast_create_node(AST_STRUCT_TYPE, start_loc);
    if (!node) {
        ast_node_list_destroy(type_args);
        return NULL;
    }

    node->data.struct_type.name = strdup(name);
    node->data.struct_type.type_args = type_args;

    return node;
}