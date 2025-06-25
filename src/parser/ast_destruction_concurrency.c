/**
 * Asthra Programming Language Compiler
 * AST Node Destruction - Concurrency
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains destruction operations for concurrency AST nodes
 * Uses common patterns from ast_destruction_common.h
 */

#include "ast_destruction_concurrency.h"
#include "ast_destruction_common.h"

void ast_free_concurrency_nodes(ASTNode *node) {
    AST_DESTRUCTION_NULL_CHECK(node);

    switch (node->type) {
    case AST_SPAWN_STMT:
        AST_FREE_STRING(node->data.spawn_stmt.function_name);
        AST_DESTROY_NODE_LIST(node->data.spawn_stmt.args);
        break;

    case AST_SPAWN_WITH_HANDLE_STMT:
        AST_FREE_STRING(node->data.spawn_with_handle_stmt.handle_var_name);
        AST_FREE_STRING(node->data.spawn_with_handle_stmt.function_name);
        AST_DESTROY_NODE_LIST(node->data.spawn_with_handle_stmt.args);
        break;

        /*
         * Tier 3 concurrency feature removed in Phase 3:
         * AST_CHANNEL_DECL

        case AST_CHANNEL_DECL:
            AST_FREE_STRING(node->data.channel_decl.channel_name);
            AST_RELEASE_NODE(node->data.channel_decl.element_type);
            AST_RELEASE_NODE(node->data.channel_decl.buffer_size);
            break;
        */

        /*
         * Note: Tier 3 concurrency destruction moved to stdlib:
         * AST_SEND_STMT, AST_RECV_EXPR, AST_SELECT_STMT, AST_SELECT_CASE,
         * AST_WORKER_POOL_DECL, AST_CLOSE_STMT
         */

    default:
        // Not a concurrency node
        break;
    }
}
