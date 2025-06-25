/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Tests - Function Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for function declaration and resolution functionality
 */

#include "test_symbol_resolution_common.h"

// =============================================================================
// FUNCTION RESOLUTION TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_function_declaration_and_resolution, ASTHRA_TEST_SEVERITY_CRITICAL) {
    printf("STARTING test_function_declaration_and_resolution...\n");
    fflush(stdout);

    SymbolTestContext *ctx = create_symbol_test_context();
    printf("Created test context: %p\n", (void *)ctx);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // Create function type descriptor
    TypeDescriptor *func_type = type_descriptor_create_function();
    printf("Created function type: %p\n", (void *)func_type);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, func_type, "Failed to create function type");

    // Declare function
    bool success = semantic_declare_symbol(ctx->analyzer, "add", SYMBOL_FUNCTION, func_type,
                                           create_mock_ast_node(AST_FUNCTION_DECL));
    printf("Declaration success: %d (should be 1)\n", success);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_TRUE(context, success, "Failed to declare function 'add'");

    // Test function resolution
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "add");
    printf("Resolved entry: %p (should not be NULL)\n", (void *)entry);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, entry, "Failed to resolve function 'add'");

    destroy_symbol_test_context(ctx);
    printf("COMPLETED test_function_declaration_and_resolution\n");
    fflush(stdout);
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_function_resolution_undefined, ASTHRA_TEST_SEVERITY_HIGH) {
    SymbolTestContext *ctx = create_symbol_test_context();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // Test lookup of undefined function
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "undefined_func");
    ASTHRA_TEST_ASSERT_NULL(context, entry, "Should not resolve undefined function");

    // Check that an error was reported
    size_t error_count = semantic_get_error_count(ctx->analyzer);
    ASTHRA_TEST_ASSERT_GT(context, error_count, 0, "Should report error for undefined function");

    destroy_symbol_test_context(ctx);
    ASTHRA_TEST_PASS(context);
}