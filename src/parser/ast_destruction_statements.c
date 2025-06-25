/**
 * Asthra Programming Language Compiler
 * AST Node Destruction - Statements
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains destruction operations for statement AST nodes
 * Uses common patterns from ast_destruction_common.h
 */

#include "ast_destruction_statements.h"
#include "ast_destruction_common.h"

void ast_free_statement_nodes(ASTNode *node) {
    AST_DESTRUCTION_NULL_CHECK(node);

    switch (node->type) {
    case AST_BLOCK:
        AST_DESTROY_NODE_LIST(node->data.block.statements);
        break;

    case AST_EXPR_STMT:
        AST_RELEASE_NODE(node->data.expr_stmt.expression);
        break;

    case AST_LET_STMT:
        AST_FREE_STRING(node->data.let_stmt.name);
        AST_RELEASE_NODE(node->data.let_stmt.type);
        AST_RELEASE_NODE(node->data.let_stmt.initializer);
        AST_DESTROY_NODE_LIST(node->data.let_stmt.annotations);
        break;

    case AST_RETURN_STMT:
        AST_RELEASE_NODE(node->data.return_stmt.expression);
        break;

    case AST_IF_STMT:
        AST_DESTROY_IF_STMT(node);
        break;

    case AST_MATCH_STMT:
        AST_DESTROY_MATCH_STMT(node);
        break;

    case AST_IF_LET_STMT:
        AST_RELEASE_NODE(node->data.if_let_stmt.pattern);
        AST_RELEASE_NODE(node->data.if_let_stmt.expression);
        AST_RELEASE_NODE(node->data.if_let_stmt.then_block);
        AST_RELEASE_NODE(node->data.if_let_stmt.else_block);
        break;

    case AST_ASSIGNMENT:
        AST_RELEASE_NODE(node->data.assignment.target);
        AST_RELEASE_NODE(node->data.assignment.value);
        break;

    case AST_UNSAFE_BLOCK:
        AST_RELEASE_NODE(node->data.unsafe_block.block);
        break;

    default:
        // Not a statement node
        break;
    }
}
