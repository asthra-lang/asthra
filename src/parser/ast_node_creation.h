/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Node Creation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Header for AST node creation operations
 */

#ifndef ASTHRA_AST_NODE_CREATION_H
#define ASTHRA_AST_NODE_CREATION_H

#include "ast_types.h"
#include "ast_generic.h" // For ASTNodeData

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// NODE CREATION OPERATIONS
// =============================================================================

/**
 * Create a new AST node of the specified type
 * @param type The type of node to create
 * @param location The source location for this node
 * @return A new AST node or NULL on allocation failure
 */
ASTNode *ast_create_node(ASTNodeType type, SourceLocation location);

/**
 * Create a new AST node with pre-initialized data
 * @param type The type of node to create
 * @param location The source location for this node
 * @param data The data to initialize the node with
 * @return A new AST node or NULL on allocation failure
 */
ASTNode *ast_create_node_with_data(ASTNodeType type, SourceLocation location, ASTNodeData data);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AST_NODE_CREATION_H
