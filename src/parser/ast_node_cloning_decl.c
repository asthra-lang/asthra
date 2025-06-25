/**
 * Asthra Programming Language Compiler
 * AST Node Cloning - Declaration Nodes
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains cloning implementations for declaration AST nodes.
 */

#include "ast_node_cloning_internal.h"
#include <string.h>

ASTNode *clone_declaration_node(ASTNode *node, ASTNode *clone) {
    switch (node->type) {
    case AST_FUNCTION_DECL:
        if (node->data.function_decl.name) {
            clone->data.function_decl.name = strdup(node->data.function_decl.name);
        }
        clone->data.function_decl.params =
            ast_node_list_clone_deep(node->data.function_decl.params);
        clone->data.function_decl.return_type =
            ast_clone_node(node->data.function_decl.return_type);
        clone->data.function_decl.body = ast_clone_node(node->data.function_decl.body);
        clone->data.function_decl.visibility = node->data.function_decl.visibility;
        clone->data.function_decl.annotations =
            ast_node_list_clone_deep(node->data.function_decl.annotations);
        break;

    case AST_STRUCT_DECL:
        if (node->data.struct_decl.name) {
            clone->data.struct_decl.name = strdup(node->data.struct_decl.name);
        }
        clone->data.struct_decl.fields = ast_node_list_clone_deep(node->data.struct_decl.fields);
        clone->data.struct_decl.type_params =
            ast_node_list_clone_deep(node->data.struct_decl.type_params);
        clone->data.struct_decl.visibility = node->data.struct_decl.visibility;
        clone->data.struct_decl.annotations =
            ast_node_list_clone_deep(node->data.struct_decl.annotations);
        break;

    case AST_ENUM_DECL:
        if (node->data.enum_decl.name) {
            clone->data.enum_decl.name = strdup(node->data.enum_decl.name);
        }
        clone->data.enum_decl.visibility = node->data.enum_decl.visibility;
        clone->data.enum_decl.variants = ast_node_list_clone_deep(node->data.enum_decl.variants);
        clone->data.enum_decl.type_params =
            ast_node_list_clone_deep(node->data.enum_decl.type_params);
        clone->data.enum_decl.annotations =
            ast_node_list_clone_deep(node->data.enum_decl.annotations);
        break;

    case AST_ENUM_VARIANT_DECL:
        if (node->data.enum_variant_decl.name) {
            clone->data.enum_variant_decl.name = strdup(node->data.enum_variant_decl.name);
        }
        clone->data.enum_variant_decl.visibility = node->data.enum_variant_decl.visibility;
        clone->data.enum_variant_decl.associated_type =
            ast_clone_node(node->data.enum_variant_decl.associated_type);
        clone->data.enum_variant_decl.value = ast_clone_node(node->data.enum_variant_decl.value);
        break;

    case AST_IMPL_BLOCK:
        if (node->data.impl_block.struct_name) {
            clone->data.impl_block.struct_name = strdup(node->data.impl_block.struct_name);
        }
        clone->data.impl_block.methods = ast_node_list_clone_deep(node->data.impl_block.methods);
        clone->data.impl_block.annotations =
            ast_node_list_clone_deep(node->data.impl_block.annotations);
        break;

    case AST_METHOD_DECL:
        if (node->data.method_decl.name) {
            clone->data.method_decl.name = strdup(node->data.method_decl.name);
        }
        clone->data.method_decl.params = ast_node_list_clone_deep(node->data.method_decl.params);
        clone->data.method_decl.return_type = ast_clone_node(node->data.method_decl.return_type);
        clone->data.method_decl.body = ast_clone_node(node->data.method_decl.body);
        clone->data.method_decl.is_instance_method = node->data.method_decl.is_instance_method;
        clone->data.method_decl.visibility = node->data.method_decl.visibility;
        clone->data.method_decl.annotations =
            ast_node_list_clone_deep(node->data.method_decl.annotations);
        break;

    case AST_EXTERN_DECL:
        if (node->data.extern_decl.name) {
            clone->data.extern_decl.name = strdup(node->data.extern_decl.name);
        }
        if (node->data.extern_decl.extern_name) {
            clone->data.extern_decl.extern_name = strdup(node->data.extern_decl.extern_name);
        }
        clone->data.extern_decl.params = ast_node_list_clone_deep(node->data.extern_decl.params);
        clone->data.extern_decl.return_type = ast_clone_node(node->data.extern_decl.return_type);
        clone->data.extern_decl.annotations =
            ast_node_list_clone_deep(node->data.extern_decl.annotations);
        break;

    case AST_PARAM_DECL:
        if (node->data.param_decl.name) {
            clone->data.param_decl.name = strdup(node->data.param_decl.name);
        }
        clone->data.param_decl.type = ast_clone_node(node->data.param_decl.type);
        clone->data.param_decl.annotations =
            ast_node_list_clone_deep(node->data.param_decl.annotations);
        break;

    case AST_CONST_DECL:
        if (node->data.const_decl.name) {
            clone->data.const_decl.name = strdup(node->data.const_decl.name);
        }
        clone->data.const_decl.type = ast_clone_node(node->data.const_decl.type);
        clone->data.const_decl.value = ast_clone_node(node->data.const_decl.value);
        clone->data.const_decl.visibility = node->data.const_decl.visibility;
        clone->data.const_decl.annotations =
            ast_node_list_clone_deep(node->data.const_decl.annotations);
        break;

    case AST_STRUCT_FIELD:
        if (node->data.struct_field.name) {
            clone->data.struct_field.name = strdup(node->data.struct_field.name);
        }
        clone->data.struct_field.type = ast_clone_node(node->data.struct_field.type);
        clone->data.struct_field.visibility = node->data.struct_field.visibility;
        break;

    default:
        // Should not happen
        break;
    }

    return clone;
}