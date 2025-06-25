/**
 * Asthra Programming Language Compiler - Concurrency Statement Grammar Productions
 * Concurrency statement parsing including spawn and log statements
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include "grammar_patterns.h"
#include "grammar_statements.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CONCURRENCY STATEMENTS
// =============================================================================

ASTNode *parse_spawn_stmt(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Check if this is spawn or spawn_with_handle
    bool is_spawn_with_handle = match_token(parser, TOKEN_SPAWN_WITH_HANDLE);

    if (!is_spawn_with_handle && !expect_token(parser, TOKEN_SPAWN)) {
        return NULL;
    }

    if (is_spawn_with_handle) {
        advance_token(parser); // consume TOKEN_SPAWN_WITH_HANDLE
    }

    char *handle_var_name = NULL;
    char *function_name = NULL;

    if (is_spawn_with_handle) {
        // spawn_with_handle syntax: spawn_with_handle variable = function(args);
        if (!match_token(parser, TOKEN_IDENTIFIER)) {
            report_error(parser, "Expected variable name after spawn_with_handle");
            return NULL;
        }

        handle_var_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);

        if (!expect_token(parser, TOKEN_ASSIGN)) {
            free(handle_var_name);
            return NULL;
        }

        if (!match_token(parser, TOKEN_IDENTIFIER)) {
            report_error(parser, "Expected function name after '=' in spawn_with_handle");
            free(handle_var_name);
            return NULL;
        }

        function_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);
    } else {
        // Regular spawn syntax: spawn function(args);
        if (!match_token(parser, TOKEN_IDENTIFIER)) {
            report_error(parser, "Expected function name after spawn");
            return NULL;
        }

        function_name = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);
    }

    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        free(function_name);
        if (handle_var_name)
            free(handle_var_name);
        return NULL;
    }

    ASTNode **args = NULL;
    size_t arg_count = 0;
    size_t arg_capacity = 4;

    args = malloc(arg_capacity * sizeof(ASTNode *));
    if (!args) {
        free(function_name);
        if (handle_var_name)
            free(handle_var_name);
        return NULL;
    }

    while (!match_token(parser, TOKEN_RIGHT_PAREN) && !at_end(parser)) {
        ASTNode *arg = parse_expr(parser);
        if (!arg) {
            for (size_t i = 0; i < arg_count; i++) {
                ast_free_node(args[i]);
            }
            free(args);
            free(function_name);
            if (handle_var_name)
                free(handle_var_name);
            return NULL;
        }

        if (arg_count >= arg_capacity) {
            arg_capacity *= 2;
            args = realloc(args, arg_capacity * sizeof(ASTNode *));
            if (!args) {
                ast_free_node(arg);
                free(function_name);
                if (handle_var_name)
                    free(handle_var_name);
                return NULL;
            }
        }

        args[arg_count++] = arg;

        if (!match_token(parser, TOKEN_COMMA)) {
            break;
        } else {
            advance_token(parser);
        }
    }

    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        for (size_t i = 0; i < arg_count; i++) {
            ast_free_node(args[i]);
        }
        free(args);
        free(function_name);
        if (handle_var_name)
            free(handle_var_name);
        return NULL;
    }

    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        for (size_t i = 0; i < arg_count; i++) {
            ast_free_node(args[i]);
        }
        free(args);
        free(function_name);
        if (handle_var_name)
            free(handle_var_name);
        return NULL;
    }

    // Create appropriate AST node based on spawn type
    ASTNodeType node_type = is_spawn_with_handle ? AST_SPAWN_WITH_HANDLE_STMT : AST_SPAWN_STMT;
    ASTNode *node = ast_create_node(node_type, start_loc);
    if (!node) {
        for (size_t i = 0; i < arg_count; i++) {
            ast_free_node(args[i]);
        }
        free(args);
        free(function_name);
        if (handle_var_name)
            free(handle_var_name);
        return NULL;
    }

    if (is_spawn_with_handle) {
        node->data.spawn_with_handle_stmt.function_name = function_name;
        node->data.spawn_with_handle_stmt.handle_var_name = handle_var_name;
    } else {
        node->data.spawn_stmt.function_name = function_name;
    }

    // Convert args array to ASTNodeList for both spawn types
    if (args && arg_count > 0) {
        ASTNodeList *arg_list = ast_node_list_create(arg_count);
        if (arg_list) {
            for (size_t i = 0; i < arg_count; i++) {
                ast_node_list_add(&arg_list, args[i]);
            }
        }

        if (is_spawn_with_handle) {
            node->data.spawn_with_handle_stmt.args = arg_list;
        } else {
            node->data.spawn_stmt.args = arg_list;
        }
        free(args);
    } else {
        if (is_spawn_with_handle) {
            node->data.spawn_with_handle_stmt.args = NULL;
        } else {
            node->data.spawn_stmt.args = NULL;
        }
    }

    return node;
}
