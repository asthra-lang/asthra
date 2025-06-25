/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Reference Counting
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains the implementation of AST node reference counting
 * operations for memory management
 * Split from the original ast_node_creation.c for better modularity
 */

#include "ast_node_refcount.h"
#include "ast.h"
#include "ast_operations.h" // For ast_free_node
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

// =============================================================================
// REFERENCE COUNTING
// =============================================================================

ASTNode *ast_retain_node(ASTNode *node) {
    if (!node)
        return NULL;

    atomic_fetch_add_explicit(&node->ref_count, 1, memory_order_relaxed);
    return node;
}

void ast_release_node(ASTNode *node) {
    if (!node)
        return;

    uint32_t old_count = atomic_fetch_sub_explicit(&node->ref_count, 1, memory_order_acq_rel);
    if (old_count == 1) {
        // Last reference, free the node
        ast_free_node(node);
    }
}

uint32_t ast_get_ref_count(const ASTNode *node) {
    if (!node)
        return 0;
    return atomic_load_explicit(&node->ref_count, memory_order_acquire);
}

// Compatibility wrappers for existing code
void ast_node_ref(ASTNode *node) {
    ast_retain_node(node);
}

void ast_node_unref(ASTNode *node) {
    ast_release_node(node);
}