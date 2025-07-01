/**
 * Asthra Programming Language Compiler
 * AST Node Destruction - Types
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains destruction operations for type AST nodes
 * Uses common patterns from ast_destruction_common.h
 */

#include "ast_destruction_types.h"
#include "ast_destruction_common.h"

void ast_free_type_nodes(ASTNode *node) {
    AST_DESTRUCTION_NULL_CHECK(node);

    switch (node->type) {
    case AST_BASE_TYPE:
        AST_FREE_STRING(node->data.base_type.name);
        break;

    case AST_SLICE_TYPE:
        AST_RELEASE_NODE(node->data.slice_type.element_type);
        break;

    case AST_ARRAY_TYPE:
        AST_RELEASE_NODE(node->data.array_type.element_type);
        AST_RELEASE_NODE(node->data.array_type.size);
        break;

    case AST_STRUCT_TYPE:
        AST_FREE_STRING(node->data.struct_type.name);
        AST_DESTROY_NODE_LIST(node->data.struct_type.type_args);
        break;

    case AST_ENUM_TYPE:
        AST_FREE_STRING(node->data.enum_type.name);
        AST_DESTROY_NODE_LIST(node->data.enum_type.type_args);
        break;

    case AST_PTR_TYPE:
        AST_RELEASE_NODE(node->data.ptr_type.pointee_type);
        break;

    case AST_RESULT_TYPE:
        AST_RELEASE_NODE(node->data.result_type.ok_type);
        AST_RELEASE_NODE(node->data.result_type.err_type);
        break;

    case AST_OPTION_TYPE:
        AST_RELEASE_NODE(node->data.option_type.value_type);
        break;

    case AST_TASKHANDLE_TYPE:
        AST_RELEASE_NODE(node->data.taskhandle_type.result_type);
        break;

        /*
         * Tier 3 concurrency features removed in Phase 3:
         * AST_CHANNEL_TYPE, AST_WORKER_POOL_TYPE

        case AST_CHANNEL_TYPE:
            AST_RELEASE_NODE(node->data.channel_type.element_type);
            AST_RELEASE_NODE(node->data.channel_type.buffer_size);
            break;

        case AST_WORKER_POOL_TYPE:
            AST_RELEASE_NODE(node->data.worker_pool_type.task_type);
            break;
        */

    default:
        // Not a type node
        break;
    }
}
