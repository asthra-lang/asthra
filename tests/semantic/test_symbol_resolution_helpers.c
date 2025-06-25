/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Tests - Helper Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test fixtures and helper functions for symbol resolution tests
 */

#include "test_symbol_resolution_common.h"

// =============================================================================
// TEST FIXTURES AND HELPERS
// =============================================================================

SymbolTestContext *create_symbol_test_context(void) {
    SymbolTestContext *ctx = calloc(1, sizeof(SymbolTestContext));
    if (!ctx)
        return NULL;

    ctx->analyzer = semantic_analyzer_create();
    if (!ctx->analyzer) {
        free(ctx);
        return NULL;
    }

    ctx->test_node = NULL;
    return ctx;
}

void destroy_symbol_test_context(SymbolTestContext *ctx) {
    if (!ctx)
        return;

    if (ctx->analyzer) {
        semantic_analyzer_destroy(ctx->analyzer);
    }
    if (ctx->test_node) {
        // Note: AST nodes should be destroyed by their own lifecycle
    }
    free(ctx);
}

// Helper to create a mock ASTNode for testing
ASTNode *create_mock_ast_node(ASTNodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node)
        return NULL;

    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->location.line = 1;
    node->location.column = 1;
    return node;
}

// Helper to create a mock variable declaration node
ASTNode *create_mock_variable_declaration(const char *name, const char *type_name) {
    ASTNode *node = create_mock_ast_node(AST_LET_STMT);
    if (!node)
        return NULL;

    // This would need proper AST structure setup in a real implementation
    // For now, we'll use the node as a placeholder
    return node;
}