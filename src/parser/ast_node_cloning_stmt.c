/**
 * Asthra Programming Language Compiler
 * AST Node Cloning - Statement Nodes
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains cloning implementations for statement AST nodes.
 */

#include "ast_node_cloning_internal.h"
#include <string.h>

ASTNode *clone_statement_node(ASTNode *node, ASTNode *clone) {
    switch (node->type) {
    case AST_EXPR_STMT:
        clone->data.expr_stmt.expression = ast_clone_node(node->data.expr_stmt.expression);
        break;

    case AST_LET_STMT:
        if (node->data.let_stmt.name) {
            clone->data.let_stmt.name = strdup(node->data.let_stmt.name);
        }
        clone->data.let_stmt.type = ast_clone_node(node->data.let_stmt.type);
        clone->data.let_stmt.initializer = ast_clone_node(node->data.let_stmt.initializer);
        clone->data.let_stmt.is_mutable = node->data.let_stmt.is_mutable;
        clone->data.let_stmt.annotations =
            ast_node_list_clone_deep(node->data.let_stmt.annotations);
        break;

    case AST_RETURN_STMT:
        clone->data.return_stmt.expression = ast_clone_node(node->data.return_stmt.expression);
        break;

    case AST_IF_STMT:
        clone->data.if_stmt.condition = ast_clone_node(node->data.if_stmt.condition);
        clone->data.if_stmt.then_block = ast_clone_node(node->data.if_stmt.then_block);
        clone->data.if_stmt.else_block = ast_clone_node(node->data.if_stmt.else_block);
        break;

    case AST_FOR_STMT:
        if (node->data.for_stmt.variable) {
            clone->data.for_stmt.variable = strdup(node->data.for_stmt.variable);
        }
        clone->data.for_stmt.iterable = ast_clone_node(node->data.for_stmt.iterable);
        clone->data.for_stmt.body = ast_clone_node(node->data.for_stmt.body);
        break;

    case AST_IF_LET_STMT:
        clone->data.if_let_stmt.pattern = ast_clone_node(node->data.if_let_stmt.pattern);
        clone->data.if_let_stmt.expression = ast_clone_node(node->data.if_let_stmt.expression);
        clone->data.if_let_stmt.then_block = ast_clone_node(node->data.if_let_stmt.then_block);
        clone->data.if_let_stmt.else_block = ast_clone_node(node->data.if_let_stmt.else_block);
        break;

    case AST_MATCH_STMT:
        clone->data.match_stmt.expression = ast_clone_node(node->data.match_stmt.expression);
        clone->data.match_stmt.arms = ast_node_list_clone_deep(node->data.match_stmt.arms);
        break;

    case AST_SPAWN_STMT:
        if (node->data.spawn_stmt.function_name) {
            clone->data.spawn_stmt.function_name = strdup(node->data.spawn_stmt.function_name);
        }
        clone->data.spawn_stmt.args = ast_node_list_clone_deep(node->data.spawn_stmt.args);
        break;

    case AST_SPAWN_WITH_HANDLE_STMT:
        if (node->data.spawn_with_handle_stmt.function_name) {
            clone->data.spawn_with_handle_stmt.function_name =
                strdup(node->data.spawn_with_handle_stmt.function_name);
        }
        if (node->data.spawn_with_handle_stmt.handle_var_name) {
            clone->data.spawn_with_handle_stmt.handle_var_name =
                strdup(node->data.spawn_with_handle_stmt.handle_var_name);
        }
        clone->data.spawn_with_handle_stmt.args =
            ast_node_list_clone_deep(node->data.spawn_with_handle_stmt.args);
        break;

    case AST_UNSAFE_BLOCK:
        clone->data.unsafe_block.block = ast_clone_node(node->data.unsafe_block.block);
        break;

    default:
        // Should not happen
        break;
    }

    return clone;
}