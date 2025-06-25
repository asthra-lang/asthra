/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Tests - Common Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Shared definitions and declarations for symbol resolution tests
 */

#ifndef SYMBOL_RESOLUTION_TEST_COMMON_H
#define SYMBOL_RESOLUTION_TEST_COMMON_H

#include "../framework/test_framework.h"
#include "ast.h"
#include "semantic_analyzer_core.h"
#include "semantic_errors.h"
#include "semantic_scopes.h"
#include "semantic_symbols.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST CONTEXT AND HELPERS
// =============================================================================

typedef struct {
    SemanticAnalyzer *analyzer;
    ASTNode *test_node;
} SymbolTestContext;

// Context management
SymbolTestContext *create_symbol_test_context(void);
void destroy_symbol_test_context(SymbolTestContext *ctx);

// Mock AST node creation
ASTNode *create_mock_ast_node(ASTNodeType type);
ASTNode *create_mock_variable_declaration(const char *name, const char *type_name);

// =============================================================================
// TEST DECLARATIONS - These are internal to the test files
// =============================================================================
// Test functions are declared statically in their respective files
// and called directly from the main test runner

#endif // SYMBOL_RESOLUTION_TEST_COMMON_H