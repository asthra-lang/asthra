/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Tests - Import Resolution Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for import resolution and module alias functionality
 */

#include "test_symbol_resolution_common.h"

// =============================================================================
// IMPORT RESOLUTION TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_module_alias_registration, ASTHRA_TEST_SEVERITY_HIGH) {
    printf("STARTING test_module_alias_registration...\n");
    fflush(stdout);
    
    SymbolTestContext *ctx = create_symbol_test_context();
    printf("Created test context: %p\n", (void*)ctx);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");
    
    // Create mock module symbol table
    SymbolTable *module_table = symbol_table_create(16);
    printf("Created module table: %p\n", (void*)module_table);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, module_table, "Failed to create module symbol table");
    
    // Register module alias
    bool success = symbol_table_add_alias(
        ctx->analyzer->current_scope,
        "io",
        "std/io",
        module_table
    );
    printf("Alias registration success: %d (should be 1)\n", success);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_TRUE(context, success, "Failed to register module alias");
    
    // Resolve module alias
    SymbolTable *resolved = symbol_table_resolve_alias(ctx->analyzer->current_scope, "io");
    printf("Resolved table: %p, module_table: %p (should be equal)\n", (void*)resolved, (void*)module_table);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, resolved, "Failed to resolve module alias");
    ASTHRA_TEST_ASSERT_EQ(context, resolved, module_table, "Resolved wrong module table");
    
    symbol_table_destroy(module_table);
    destroy_symbol_test_context(ctx);
    printf("COMPLETED test_module_alias_registration\n");
    fflush(stdout);
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_import_resolution_undefined_module, ASTHRA_TEST_SEVERITY_MEDIUM) {
    printf("STARTING test_import_resolution_undefined_module...\n");
    fflush(stdout);
    
    SymbolTestContext *ctx = create_symbol_test_context();
    printf("Created test context: %p\n", (void*)ctx);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");
    
    // Try to resolve undefined module alias
    SymbolTable *resolved = symbol_table_resolve_alias(ctx->analyzer->current_scope, "undefined_module");
    printf("Resolved undefined module: %p (should be NULL)\n", (void*)resolved);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NULL(context, resolved, "Should not resolve undefined module alias");
    
    destroy_symbol_test_context(ctx);
    printf("COMPLETED test_import_resolution_undefined_module\n");
    fflush(stdout);
    ASTHRA_TEST_PASS(context);
}