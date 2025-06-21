/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Node Creation and Memory Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file contains the implementation of AST node creation, destruction,
 * and reference counting operations
 * Split from the original ast.c for better maintainability
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdatomic.h>
#include "ast.h"
#include "ast_node_creation.h"

// =============================================================================
// AST NODE CREATION
// =============================================================================

ASTNode *ast_create_node(ASTNodeType type, SourceLocation location) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    
    node->type = type;
    node->location = location;
    node->type_info = NULL;
    
    // Initialize atomic reference count
    atomic_init(&node->ref_count, 1);
    
    // Initialize flags
    node->flags.is_validated = false;
    node->flags.is_type_checked = false;
    node->flags.is_constant_expr = false;
    node->flags.has_side_effects = false;
    node->flags.is_lvalue = false;
    node->flags.is_mutable = false;
    node->flags.reserved = 0;
    
    return node;
}

ASTNode *ast_create_node_with_data(ASTNodeType type, SourceLocation location, ASTNodeData data) {
    ASTNode *node = ast_create_node(type, location);
    if (!node) return NULL;
    
    // Copy the appropriate data based on node type
    switch (type) {
        case AST_BINARY_EXPR:
            node->data.binary_expr.operator = data.binary_expr.operator;
            node->data.binary_expr.left = data.binary_expr.left;
            node->data.binary_expr.right = data.binary_expr.right;
            break;
        case AST_UNARY_EXPR:
            node->data.unary_expr.operator = data.unary_expr.operator;
            node->data.unary_expr.operand = data.unary_expr.operand;
            break;
        case AST_IDENTIFIER:
            node->data.identifier.name = data.identifier.name ? strdup(data.identifier.name) : NULL;
            break;
        case AST_INTEGER_LITERAL:
            node->data.integer_literal.value = data.integer_literal.value;
            break;
        case AST_FLOAT_LITERAL:
            node->data.float_literal.value = data.float_literal.value;
            break;
        case AST_STRING_LITERAL:
            node->data.string_literal.value = data.string_literal.value ? strdup(data.string_literal.value) : NULL;
            break;
        case AST_BOOL_LITERAL:
            node->data.bool_literal.value = data.bool_literal.value;
            break;
        case AST_UNIT_LITERAL:
            // Unit literal has no data to set
            break;
        default:
            // For other types, the caller should set the data manually
            break;
    }
    
    return node;
}

// =============================================================================
// REFERENCE COUNTING
// =============================================================================

ASTNode *ast_retain_node(ASTNode *node) {
    if (!node) return NULL;
    
    atomic_fetch_add_explicit(&node->ref_count, 1, memory_order_relaxed);
    return node;
}

void ast_release_node(ASTNode *node) {
    if (!node) return;
    
    uint32_t old_count = atomic_fetch_sub_explicit(&node->ref_count, 1, memory_order_acq_rel);
    if (old_count == 1) {
        // Last reference, free the node
        ast_free_node(node);
    }
}

uint32_t ast_get_ref_count(const ASTNode *node) {
    if (!node) return 0;
    return atomic_load_explicit(&node->ref_count, memory_order_acquire);
}

// Compatibility wrappers for existing code
void ast_node_ref(ASTNode *node) {
    ast_retain_node(node);
}

void ast_node_unref(ASTNode *node) {
    ast_release_node(node);
}

// =============================================================================
// NODE CLONING
// =============================================================================

ASTNode *ast_clone_node(ASTNode *node) {
    if (!node) return NULL;
    
    ASTNode *clone = ast_create_node(node->type, node->location);
    if (!clone) return NULL;
    
    // Copy type info
    clone->type_info = node->type_info;
    
    switch (node->type) {
        case AST_PACKAGE_DECL:
            if (node->data.package_decl.name) {
                clone->data.package_decl.name = strdup(node->data.package_decl.name);
            }
            break;
            
        case AST_IMPORT_DECL:
            if (node->data.import_decl.path) {
                clone->data.import_decl.path = strdup(node->data.import_decl.path);
            }
            if (node->data.import_decl.alias) {
                clone->data.import_decl.alias = strdup(node->data.import_decl.alias);
            }
            break;
            
        case AST_INTEGER_LITERAL:
            clone->data.integer_literal.value = node->data.integer_literal.value;
            break;
            
        case AST_FLOAT_LITERAL:
            clone->data.float_literal.value = node->data.float_literal.value;
            break;
            
        case AST_STRING_LITERAL:
            if (node->data.string_literal.value) {
                clone->data.string_literal.value = strdup(node->data.string_literal.value);
            }
            break;
            
        case AST_BOOL_LITERAL:
            clone->data.bool_literal.value = node->data.bool_literal.value;
            break;
            
        case AST_CHAR_LITERAL:
            clone->data.char_literal.value = node->data.char_literal.value;
            break;
            
        case AST_UNIT_LITERAL:
            // Unit literal has no data to copy
            break;
            
        case AST_STRUCT_LITERAL:
            if (node->data.struct_literal.struct_name) {
                clone->data.struct_literal.struct_name = strdup(node->data.struct_literal.struct_name);
            }
            // TODO: Implement deep cloning of type_args and field_inits when needed
            clone->data.struct_literal.type_args = node->data.struct_literal.type_args;
            clone->data.struct_literal.field_inits = node->data.struct_literal.field_inits;
            break;
            
        case AST_IDENTIFIER:
            if (node->data.identifier.name) {
                clone->data.identifier.name = strdup(node->data.identifier.name);
            }
            break;
            
        case AST_BINARY_EXPR:
            clone->data.binary_expr.operator = node->data.binary_expr.operator;
            clone->data.binary_expr.left = ast_clone_node(node->data.binary_expr.left);
            clone->data.binary_expr.right = ast_clone_node(node->data.binary_expr.right);
            break;
            
        case AST_UNARY_EXPR:
            clone->data.unary_expr.operator = node->data.unary_expr.operator;
            clone->data.unary_expr.operand = ast_clone_node(node->data.unary_expr.operand);
            break;
            
        case AST_ENUM_DECL:
            if (node->data.enum_decl.name) {
                clone->data.enum_decl.name = strdup(node->data.enum_decl.name);
            }
            clone->data.enum_decl.visibility = node->data.enum_decl.visibility;
            // TODO: Implement deep cloning of variants and type_params when needed
            clone->data.enum_decl.variants = node->data.enum_decl.variants;
            clone->data.enum_decl.type_params = node->data.enum_decl.type_params;
            clone->data.enum_decl.annotations = node->data.enum_decl.annotations;
            break;
            
        case AST_ENUM_VARIANT_DECL:
            if (node->data.enum_variant_decl.name) {
                clone->data.enum_variant_decl.name = strdup(node->data.enum_variant_decl.name);
            }
            clone->data.enum_variant_decl.visibility = node->data.enum_variant_decl.visibility;
            clone->data.enum_variant_decl.associated_type = ast_clone_node(node->data.enum_variant_decl.associated_type);
            clone->data.enum_variant_decl.value = ast_clone_node(node->data.enum_variant_decl.value);
            break;
            
        // Add more cases as needed for other node types
        default:
            // For now, just copy the data union directly for simple cases
            clone->data = node->data;
            break;
    }
    
    return clone;
} 
