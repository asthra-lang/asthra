/**
 * Asthra Programming Language Compiler
 * AST Node Destruction - Expressions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file contains destruction operations for expression AST nodes
 * Uses common patterns from ast_destruction_common.h
 */

#include "ast_destruction_common.h"
#include "ast_destruction_expressions.h"

void ast_free_expression_nodes(ASTNode *node) {
    AST_DESTRUCTION_NULL_CHECK(node);
    
    switch (node->type) {
        case AST_BINARY_EXPR:
            AST_DESTROY_BINARY_EXPR(node);
            break;
            
        case AST_UNARY_EXPR:
            AST_DESTROY_UNARY_EXPR(node);
            break;
            
        case AST_POSTFIX_EXPR:
            AST_RELEASE_NODE(node->data.postfix_expr.base);
            AST_DESTROY_NODE_LIST(node->data.postfix_expr.suffixes);
            break;
            
        case AST_CALL_EXPR:
            AST_DESTROY_CALL_EXPR(node);
            break;
            
        case AST_ASSOCIATED_FUNC_CALL:
            AST_FREE_STRING(node->data.associated_func_call.struct_name);
            AST_FREE_STRING(node->data.associated_func_call.function_name);
            AST_DESTROY_NODE_LIST(node->data.associated_func_call.type_args);
            AST_DESTROY_NODE_LIST(node->data.associated_func_call.args);
            break;
            
        case AST_FIELD_ACCESS:
            AST_DESTROY_FIELD_ACCESS(node);
            break;
            
        case AST_INDEX_ACCESS:
            AST_DESTROY_INDEX_ACCESS(node);
            break;
            
        case AST_SLICE_EXPR:
            AST_RELEASE_NODE(node->data.slice_expr.array);
            AST_RELEASE_NODE(node->data.slice_expr.start);
            AST_RELEASE_NODE(node->data.slice_expr.end);
            break;
            
        case AST_SLICE_LENGTH_ACCESS:
            AST_RELEASE_NODE(node->data.slice_length_access.slice);
            break;
            
        case AST_ARRAY_LITERAL:
            AST_DESTROY_NODE_LIST(node->data.array_literal.elements);
            break;
            
        case AST_ENUM_VARIANT:
            AST_FREE_STRING(node->data.enum_variant.enum_name);
            AST_FREE_STRING(node->data.enum_variant.variant_name);
            AST_RELEASE_NODE(node->data.enum_variant.value);
            break;
            
        case AST_AWAIT_EXPR:
            AST_RELEASE_NODE(node->data.await_expr.task_handle_expr);
            AST_RELEASE_NODE(node->data.await_expr.timeout_expr);
            break;
            
        /*
         * Tier 3 concurrency feature removed in Phase 3:
         * AST_TIMEOUT_EXPR
        
        case AST_TIMEOUT_EXPR:
            AST_RELEASE_NODE(node->data.timeout_expr.expr);
            AST_RELEASE_NODE(node->data.timeout_expr.duration);
            break;
        */
            
        default:
            // Not an expression node
            break;
    }
} 
