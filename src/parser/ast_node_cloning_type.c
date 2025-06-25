/**
 * Asthra Programming Language Compiler
 * AST Node Cloning - Type Nodes
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains cloning implementations for type AST nodes.
 */

#include "ast_node_cloning_internal.h"
#include <string.h>

ASTNode *clone_type_node(ASTNode *node, ASTNode *clone) {
    switch (node->type) {
    case AST_BASE_TYPE:
        if (node->data.base_type.name) {
            clone->data.base_type.name = strdup(node->data.base_type.name);
        }
        break;

    case AST_SLICE_TYPE:
        clone->data.slice_type.element_type = ast_clone_node(node->data.slice_type.element_type);
        break;

    case AST_ARRAY_TYPE:
        clone->data.array_type.element_type = ast_clone_node(node->data.array_type.element_type);
        clone->data.array_type.size = ast_clone_node(node->data.array_type.size);
        break;

    case AST_PTR_TYPE:
        clone->data.ptr_type.is_mutable = node->data.ptr_type.is_mutable;
        clone->data.ptr_type.pointee_type = ast_clone_node(node->data.ptr_type.pointee_type);
        break;

    case AST_RESULT_TYPE:
        clone->data.result_type.ok_type = ast_clone_node(node->data.result_type.ok_type);
        clone->data.result_type.err_type = ast_clone_node(node->data.result_type.err_type);
        break;

    case AST_OPTION_TYPE:
        clone->data.option_type.value_type = ast_clone_node(node->data.option_type.value_type);
        break;

    case AST_STRUCT_TYPE:
        if (node->data.struct_type.name) {
            clone->data.struct_type.name = strdup(node->data.struct_type.name);
        }
        clone->data.struct_type.type_args =
            ast_node_list_clone_deep(node->data.struct_type.type_args);
        break;

    case AST_ENUM_TYPE:
        if (node->data.enum_type.name) {
            clone->data.enum_type.name = strdup(node->data.enum_type.name);
        }
        clone->data.enum_type.type_args = ast_node_list_clone_deep(node->data.enum_type.type_args);
        break;

    case AST_TUPLE_TYPE:
        clone->data.tuple_type.element_types =
            ast_node_list_clone_deep(node->data.tuple_type.element_types);
        break;

    default:
        // Should not happen
        break;
    }

    return clone;
}