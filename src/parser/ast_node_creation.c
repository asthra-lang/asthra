/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Node Creation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains the implementation of AST node creation operations
 * Split from the original ast.c for better maintainability
 */

#include "ast_node_creation.h"
#include "ast.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    if (!node)
        return NULL;

    // Copy the appropriate data based on node type
    switch (type) {
    case AST_BINARY_EXPR:
        node->data.binary_expr.operator= data.binary_expr.operator;
        node->data.binary_expr.left = data.binary_expr.left;
        node->data.binary_expr.right = data.binary_expr.right;
        break;
    case AST_UNARY_EXPR:
        node->data.unary_expr.operator= data.unary_expr.operator;
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
        node->data.string_literal.value =
            data.string_literal.value ? strdup(data.string_literal.value) : NULL;
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