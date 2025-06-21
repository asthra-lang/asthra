/**
 * Asthra Programming Language Compiler
 * AST Node Destruction - Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This header declares destruction functions for different categories of AST nodes
 */

#ifndef AST_DESTRUCTION_H
#define AST_DESTRUCTION_H

#include "ast.h"

// Declaration destruction functions
void ast_free_declaration_nodes(ASTNode *node);

// Statement destruction functions
void ast_free_statement_nodes(ASTNode *node);

// Expression destruction functions
void ast_free_expression_nodes(ASTNode *node);

// Type destruction functions
void ast_free_type_nodes(ASTNode *node);

// Concurrency destruction functions
void ast_free_concurrency_nodes(ASTNode *node);

// Pattern destruction functions
void ast_free_pattern_nodes(ASTNode *node);

// Literal and annotation destruction functions
void ast_free_literal_nodes(ASTNode *node);

#endif // AST_DESTRUCTION_H 
