/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Node Cloning
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Header for AST node cloning operations
 */

#ifndef ASTHRA_AST_NODE_CLONING_H
#define ASTHRA_AST_NODE_CLONING_H

#include "ast_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// NODE CLONING OPERATIONS
// =============================================================================

/**
 * Create a deep copy of an AST node
 * This function recursively clones the entire subtree
 * @param node The node to clone
 * @return A new node that is a deep copy of the input, or NULL on failure
 */
ASTNode *ast_clone_node(ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AST_NODE_CLONING_H