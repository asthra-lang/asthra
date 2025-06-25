/**
 * Asthra Programming Language Compiler
 * Code generation utilities - shared functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "codegen_internal.h"
#include <stdbool.h>
#include <string.h>

// Global context for tracking current function during code generation
const char *current_function_name = NULL;
bool current_function_returns_void = false;

// Helper to check if an AST node ends with a return statement
bool ends_with_return(ASTNode *node) {
    if (!node)
        return false;

    switch (node->type) {
    case AST_RETURN_STMT:
        return true;

    case AST_BLOCK:
        if (node->data.block.statements && node->data.block.statements->count > 0) {
            size_t last_idx = node->data.block.statements->count - 1;
            return ends_with_return(node->data.block.statements->nodes[last_idx]);
        }
        return false;

    case AST_IF_STMT:
        // Both branches must end with return for the if-else to count as ending with return
        return node->data.if_stmt.else_block != NULL &&
               ends_with_return(node->data.if_stmt.then_block) &&
               ends_with_return(node->data.if_stmt.else_block);

    default:
        return false;
    }
}