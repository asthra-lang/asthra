/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Tests - Variable Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for variable declaration and lookup functionality
 */

#include "test_symbol_resolution_common.h"

// =============================================================================
// VARIABLE DECLARATION AND LOOKUP TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_variable_declaration_basic, ASTHRA_TEST_SEVERITY_CRITICAL) {
    printf("STARTING test_variable_declaration_basic...\n");
    fflush(stdout);
    
    SymbolTestContext *ctx = create_symbol_test_context();
    printf("Created test context: %p\n", (void*)ctx);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");
    
    // Test basic variable declaration
    bool success = semantic_declare_symbol(
        ctx->analyzer, 
        "x", 
        SYMBOL_VARIABLE, 
        semantic_get_builtin_type(ctx->analyzer, "int"),
        create_mock_variable_declaration("x", "int")
    );
    
    printf("Declaration success: %d\n", success);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_TRUE(context, success, "Failed to declare variable 'x'");
    
    // Test lookup of declared variable
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "x");
    printf("Resolved entry: %p\n", (void*)entry);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, entry, "Failed to resolve variable 'x'");
    
    destroy_symbol_test_context(ctx);
    printf("COMPLETED test_variable_declaration_basic\n");
    fflush(stdout);
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_variable_lookup_undefined, ASTHRA_TEST_SEVERITY_CRITICAL) {
    printf("STARTING test_variable_lookup_undefined...\n");
    fflush(stdout);
    
    SymbolTestContext *ctx = create_symbol_test_context();
    printf("Created test context: %p\n", (void*)ctx);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");
    
    // Test lookup of undefined variable
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "undefined_var");
    printf("Resolved undefined entry: %p (should be NULL)\n", (void*)entry);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NULL(context, entry, "Should not resolve undefined variable");
    
    // Check that an error was reported
    size_t error_count = semantic_get_error_count(ctx->analyzer);
    printf("Error count: %zu (should be > 0)\n", error_count);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_GT(context, error_count, 0, "Should report error for undefined symbol");
    
    destroy_symbol_test_context(ctx);
    printf("COMPLETED test_variable_lookup_undefined\n");
    fflush(stdout);
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_variable_declaration_duplicate, ASTHRA_TEST_SEVERITY_HIGH) {
    printf("STARTING test_variable_declaration_duplicate...\n");
    fflush(stdout);
    
    SymbolTestContext *ctx = create_symbol_test_context();
    printf("Created test context: %p\n", (void*)ctx);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");
    
    // Declare variable first time
    bool success1 = semantic_declare_symbol(
        ctx->analyzer, 
        "duplicate_var", 
        SYMBOL_VARIABLE, 
        semantic_get_builtin_type(ctx->analyzer, "int"),
        create_mock_variable_declaration("duplicate_var", "int")
    );
    printf("First declaration success: %d (should be 1)\n", success1);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_TRUE(context, success1, "First declaration should succeed");
    
    // Try to declare same variable again in same scope
    bool success2 = semantic_declare_symbol(
        ctx->analyzer, 
        "duplicate_var", 
        SYMBOL_VARIABLE, 
        semantic_get_builtin_type(ctx->analyzer, "int"),
        create_mock_variable_declaration("duplicate_var", "int")
    );
    printf("Second declaration success: %d (should be 0)\n", success2);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_FALSE(context, success2, "Duplicate declaration should fail");
    
    // Check that an error was reported
    size_t error_count = semantic_get_error_count(ctx->analyzer);
    printf("Error count: %zu (should be > 0)\n", error_count);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_GT(context, error_count, 0, "Should report error for duplicate symbol");
    
    destroy_symbol_test_context(ctx);
    printf("COMPLETED test_variable_declaration_duplicate\n");
    fflush(stdout);
    ASTHRA_TEST_PASS(context);
}