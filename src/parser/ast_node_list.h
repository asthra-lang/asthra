/**
 * Asthra Programming Language Compiler
 * AST Node List Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * AST node list management functions with C17 flexible arrays
 */

#ifndef ASTHRA_AST_NODE_LIST_H
#define ASTHRA_AST_NODE_LIST_H

#include <stddef.h>
#include <stdbool.h>
#include "ast_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// AST NODE LIST OPERATIONS - PHASE 2.1
// =============================================================================

// Create AST node list with initial capacity
ASTNodeList *ast_node_list_create(size_t initial_capacity);

// Destroy AST node list
void ast_node_list_destroy(ASTNodeList *list);

// Add node to list (grows automatically)
bool ast_node_list_add(ASTNodeList **list, ASTNode *node);

// Insert node at specific index
bool ast_node_list_insert(ASTNodeList **list, size_t index, ASTNode *node);

// Remove node at specific index
bool ast_node_list_remove(ASTNodeList **list, size_t index);

// Get node at specific index
ASTNode *ast_node_list_get(const ASTNodeList *list, size_t index);

// Get list size
size_t ast_node_list_size(const ASTNodeList *list);

// Clear all nodes from list
void ast_node_list_clear(ASTNodeList *list);

// Clone node list (shallow copy - retains existing nodes)
ASTNodeList *ast_node_list_clone(const ASTNodeList *list);

// Clone node list (deep copy - clones all nodes)
ASTNodeList *ast_node_list_clone_deep(const ASTNodeList *list);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AST_NODE_LIST_H 
