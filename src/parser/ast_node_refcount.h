/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Reference Counting
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Header for AST node reference counting operations
 */

#ifndef ASTHRA_AST_NODE_REFCOUNT_H
#define ASTHRA_AST_NODE_REFCOUNT_H

#include "ast_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// REFERENCE COUNTING OPERATIONS
// =============================================================================

/**
 * Increment the reference count of an AST node
 * @param node The node to retain
 * @return The same node pointer for convenience
 */
ASTNode *ast_retain_node(ASTNode *node);

/**
 * Decrement the reference count of an AST node
 * Frees the node when count reaches zero
 * @param node The node to release
 */
void ast_release_node(ASTNode *node);

/**
 * Get the current reference count of a node
 * @param node The node to query
 * @return The current reference count
 */
uint32_t ast_get_ref_count(const ASTNode *node);

// Compatibility wrappers for existing code
void ast_node_ref(ASTNode *node);
void ast_node_unref(ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AST_NODE_REFCOUNT_H