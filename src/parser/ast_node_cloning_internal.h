/**
 * Asthra Programming Language Compiler
 * AST Node Cloning Internal Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This internal header provides shared declarations for modularized
 * AST node cloning implementations.
 */

#ifndef AST_NODE_CLONING_INTERNAL_H
#define AST_NODE_CLONING_INTERNAL_H

#include "ast_node_cloning.h"
#include "ast.h"
#include "ast_node_creation.h"
#include "ast_node_list.h"

// Forward declarations for cloning functions by category
ASTNode *clone_declaration_node(ASTNode *node, ASTNode *clone);
ASTNode *clone_expression_node(ASTNode *node, ASTNode *clone);
ASTNode *clone_statement_node(ASTNode *node, ASTNode *clone);
ASTNode *clone_type_node(ASTNode *node, ASTNode *clone);
ASTNode *clone_pattern_node(ASTNode *node, ASTNode *clone);

#endif // AST_NODE_CLONING_INTERNAL_H