/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Node Destruction
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file contains the implementation of AST node destruction operations
 * Split from the original ast.c for better maintainability
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast.h"
#include "ast_destruction.h"

// =============================================================================
// AST NODE DESTRUCTION
// =============================================================================

void ast_free_node(ASTNode *node) {
    if (!node) return;
    
    // Try each category of destruction functions
    ast_free_declaration_nodes(node);
    ast_free_statement_nodes(node);
    ast_free_expression_nodes(node);
    ast_free_type_nodes(node);
    ast_free_concurrency_nodes(node);
    ast_free_pattern_nodes(node);
    ast_free_literal_nodes(node);
    
    // Free the node itself
    free(node);
} 
