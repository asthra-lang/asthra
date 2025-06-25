/**
 * Asthra Programming Language Compiler
 * AST Traditional Operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Traditional AST interface maintained for compatibility
 */

#ifndef ASTHRA_AST_OPERATIONS_H
#define ASTHRA_AST_OPERATIONS_H

#include "ast_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TRADITIONAL AST INTERFACE (MAINTAINED FOR COMPATIBILITY)
// =============================================================================

// AST visitor function type
typedef void (*ASTVisitor)(ASTNode *node, void *context);

// Destroy AST nodes
void ast_free_node(ASTNode *node);

// Alias for compatibility
static inline void ast_destroy_node(ASTNode *node) {
    ast_free_node(node);
}

// AST traversal and utilities
void ast_print(ASTNode *node, int indent);
bool ast_validate(ASTNode *node);
ASTNode *ast_find_node(ASTNode *root, ASTNodeType type, const char *name);

// AST traversal
void ast_traverse_preorder(ASTNode *node, ASTVisitor visitor, void *context);
void ast_traverse_postorder(ASTNode *node, ASTVisitor visitor, void *context);

// Utility functions
const char *ast_node_type_name(ASTNodeType type);
const char *binary_operator_name(BinaryOperator op);
const char *unary_operator_name(UnaryOperator op);

// Reference counting - moved to ast_node_refcount.h

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AST_OPERATIONS_H
