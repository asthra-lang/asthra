/**
 * Asthra Programming Language Compiler
 * AST Node Cloning - Expression Nodes
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains cloning implementations for expression AST nodes.
 */

#include "ast_node_cloning_internal.h"
#include <string.h>

ASTNode *clone_expression_node(ASTNode *node, ASTNode *clone) {
    switch (node->type) {
    case AST_STRUCT_LITERAL:
        if (node->data.struct_literal.struct_name) {
            clone->data.struct_literal.struct_name = strdup(node->data.struct_literal.struct_name);
        }
        clone->data.struct_literal.type_args =
            ast_node_list_clone_deep(node->data.struct_literal.type_args);
        clone->data.struct_literal.field_inits =
            ast_node_list_clone_deep(node->data.struct_literal.field_inits);
        break;

    case AST_BINARY_EXPR:
        clone->data.binary_expr.operator= node->data.binary_expr.operator;
        clone->data.binary_expr.left = ast_clone_node(node->data.binary_expr.left);
        clone->data.binary_expr.right = ast_clone_node(node->data.binary_expr.right);
        break;

    case AST_UNARY_EXPR:
        clone->data.unary_expr.operator= node->data.unary_expr.operator;
        clone->data.unary_expr.operand = ast_clone_node(node->data.unary_expr.operand);
        break;

    case AST_CALL_EXPR:
        clone->data.call_expr.function = ast_clone_node(node->data.call_expr.function);
        clone->data.call_expr.args = ast_node_list_clone_deep(node->data.call_expr.args);
        break;

    case AST_ARRAY_LITERAL:
        clone->data.array_literal.elements =
            ast_node_list_clone_deep(node->data.array_literal.elements);
        break;

    case AST_TUPLE_LITERAL:
        clone->data.tuple_literal.elements =
            ast_node_list_clone_deep(node->data.tuple_literal.elements);
        break;

    case AST_POSTFIX_EXPR:
        clone->data.postfix_expr.base = ast_clone_node(node->data.postfix_expr.base);
        clone->data.postfix_expr.suffixes =
            ast_node_list_clone_deep(node->data.postfix_expr.suffixes);
        break;

    case AST_ASSOCIATED_FUNC_CALL:
        if (node->data.associated_func_call.struct_name) {
            clone->data.associated_func_call.struct_name =
                strdup(node->data.associated_func_call.struct_name);
        }
        if (node->data.associated_func_call.function_name) {
            clone->data.associated_func_call.function_name =
                strdup(node->data.associated_func_call.function_name);
        }
        clone->data.associated_func_call.type_args =
            ast_node_list_clone_deep(node->data.associated_func_call.type_args);
        clone->data.associated_func_call.args =
            ast_node_list_clone_deep(node->data.associated_func_call.args);
        break;

    case AST_FIELD_ACCESS:
        clone->data.field_access.object = ast_clone_node(node->data.field_access.object);
        if (node->data.field_access.field_name) {
            clone->data.field_access.field_name = strdup(node->data.field_access.field_name);
        }
        break;

    case AST_INDEX_ACCESS:
        clone->data.index_access.array = ast_clone_node(node->data.index_access.array);
        clone->data.index_access.index = ast_clone_node(node->data.index_access.index);
        break;

    case AST_SLICE_EXPR:
        clone->data.slice_expr.array = ast_clone_node(node->data.slice_expr.array);
        clone->data.slice_expr.start = ast_clone_node(node->data.slice_expr.start);
        clone->data.slice_expr.end = ast_clone_node(node->data.slice_expr.end);
        break;

    case AST_SLICE_LENGTH_ACCESS:
        clone->data.slice_length_access.slice =
            ast_clone_node(node->data.slice_length_access.slice);
        break;

    case AST_ASSIGNMENT:
        clone->data.assignment.target = ast_clone_node(node->data.assignment.target);
        clone->data.assignment.value = ast_clone_node(node->data.assignment.value);
        break;

    case AST_AWAIT_EXPR:
        clone->data.await_expr.task_handle_expr =
            ast_clone_node(node->data.await_expr.task_handle_expr);
        clone->data.await_expr.timeout_expr = ast_clone_node(node->data.await_expr.timeout_expr);
        break;

    case AST_ENUM_VARIANT:
        if (node->data.enum_variant.enum_name) {
            clone->data.enum_variant.enum_name = strdup(node->data.enum_variant.enum_name);
        }
        if (node->data.enum_variant.variant_name) {
            clone->data.enum_variant.variant_name = strdup(node->data.enum_variant.variant_name);
        }
        clone->data.enum_variant.value = ast_clone_node(node->data.enum_variant.value);
        break;

    case AST_CONST_EXPR:
        clone->data.const_expr.expr_type = node->data.const_expr.expr_type;
        switch (node->data.const_expr.expr_type) {
        case CONST_EXPR_LITERAL:
            clone->data.const_expr.data.literal =
                ast_clone_node(node->data.const_expr.data.literal);
            break;
        case CONST_EXPR_BINARY_OP:
            clone->data.const_expr.data.binary.left =
                ast_clone_node(node->data.const_expr.data.binary.left);
            clone->data.const_expr.data.binary.op = node->data.const_expr.data.binary.op;
            clone->data.const_expr.data.binary.right =
                ast_clone_node(node->data.const_expr.data.binary.right);
            break;
        case CONST_EXPR_UNARY_OP:
            clone->data.const_expr.data.unary.op = node->data.const_expr.data.unary.op;
            clone->data.const_expr.data.unary.operand =
                ast_clone_node(node->data.const_expr.data.unary.operand);
            break;
        case CONST_EXPR_SIZEOF:
            clone->data.const_expr.data.sizeof_expr.type =
                ast_clone_node(node->data.const_expr.data.sizeof_expr.type);
            break;
        case CONST_EXPR_IDENTIFIER:
            if (node->data.const_expr.data.identifier) {
                clone->data.const_expr.data.identifier =
                    strdup(node->data.const_expr.data.identifier);
            }
            break;
        }
        break;

    default:
        // Should not happen
        break;
    }

    return clone;
}