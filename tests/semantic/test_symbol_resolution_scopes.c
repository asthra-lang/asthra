/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Tests - Scope Management Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for scope management, shadowing, and variable visibility
 */

#include "test_symbol_resolution_common.h"

// =============================================================================
// SCOPE MANAGEMENT TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_scope_shadowing, ASTHRA_TEST_SEVERITY_HIGH) {
    SymbolTestContext *ctx = create_symbol_test_context();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");
    
    // Declare variable in outer scope
    bool success1 = semantic_declare_symbol(
        ctx->analyzer, 
        "x", 
        SYMBOL_VARIABLE, 
        semantic_get_builtin_type(ctx->analyzer, "int"),
        create_mock_variable_declaration("x", "int")
    );
    ASTHRA_TEST_ASSERT_TRUE(context, success1, "Failed to declare variable in outer scope");
    
    // Enter new scope
    semantic_enter_scope(ctx->analyzer);
    
    // Declare variable with same name in inner scope (shadowing)
    bool success2 = semantic_declare_symbol(
        ctx->analyzer, 
        "x", 
        SYMBOL_VARIABLE, 
        semantic_get_builtin_type(ctx->analyzer, "string"),
        create_mock_variable_declaration("x", "string")
    );
    ASTHRA_TEST_ASSERT_TRUE(context, success2, "Failed to declare shadowing variable");
    
    // Resolve should find inner scope variable
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "x");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, entry, "Failed to resolve shadowed variable");
    
    // Exit inner scope
    semantic_exit_scope(ctx->analyzer);
    
    // Resolve should now find outer scope variable
    entry = semantic_resolve_identifier(ctx->analyzer, "x");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, entry, "Failed to resolve outer scope variable after exit");
    
    destroy_symbol_test_context(ctx);
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_scope_variable_out_of_scope, ASTHRA_TEST_SEVERITY_HIGH) {
    SymbolTestContext *ctx = create_symbol_test_context();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");
    
    // Enter new scope
    semantic_enter_scope(ctx->analyzer);
    
    // Declare variable in inner scope
    bool success = semantic_declare_symbol(
        ctx->analyzer, 
        "inner_var", 
        SYMBOL_VARIABLE, 
        semantic_get_builtin_type(ctx->analyzer, "int"),
        create_mock_variable_declaration("inner_var", "int")
    );
    ASTHRA_TEST_ASSERT_TRUE(context, success, "Failed to declare variable in inner scope");
    
    // Exit scope
    semantic_exit_scope(ctx->analyzer);
    
    // Try to resolve variable that should now be out of scope
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "inner_var");
    ASTHRA_TEST_ASSERT_NULL(context, entry, "Should not resolve out-of-scope variable");
    
    destroy_symbol_test_context(ctx);
    ASTHRA_TEST_PASS(context);
}