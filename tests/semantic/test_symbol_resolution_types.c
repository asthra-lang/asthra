/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Tests - Type Resolution Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for type name resolution for both builtin and custom types
 */

#include "test_symbol_resolution_common.h"

// =============================================================================
// TYPE NAME RESOLUTION TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_type_name_resolution_builtin, ASTHRA_TEST_SEVERITY_HIGH) {
    printf("STARTING test_type_name_resolution_builtin...\n");
    fflush(stdout);

    SymbolTestContext *ctx = create_symbol_test_context();
    printf("Created test context: %p\n", (void *)ctx);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // Test builtin type resolution
    TypeDescriptor *int_type = semantic_get_builtin_type(ctx->analyzer, "int");
    printf("Got int type: %p (should not be NULL)\n", (void *)int_type);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, int_type, "Failed to resolve builtin type 'int'");

    TypeDescriptor *string_type = semantic_get_builtin_type(ctx->analyzer, "string");
    printf("Got string type: %p (should not be NULL)\n", (void *)string_type);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, string_type, "Failed to resolve builtin type 'string'");

    destroy_symbol_test_context(ctx);
    printf("COMPLETED test_type_name_resolution_builtin\n");
    fflush(stdout);
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_type_name_resolution_custom, ASTHRA_TEST_SEVERITY_HIGH) {
    printf("STARTING test_type_name_resolution_custom...\n");
    fflush(stdout);

    SymbolTestContext *ctx = create_symbol_test_context();
    printf("Created test context: %p\n", (void *)ctx);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // Create custom struct type
    TypeDescriptor *struct_type = type_descriptor_create_struct("MyType", 2);
    printf("Created struct type: %p\n", (void *)struct_type);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, struct_type, "Failed to create struct type");

    // Declare custom type symbol
    bool success = semantic_declare_symbol(ctx->analyzer, "MyType", SYMBOL_TYPE, struct_type,
                                           create_mock_ast_node(AST_STRUCT_DECL));
    printf("Declaration success: %d (should be 1)\n", success);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_TRUE(context, success, "Failed to declare custom type");

    // Resolve custom type
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "MyType");
    printf("Resolved entry: %p (should not be NULL)\n", (void *)entry);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, entry, "Failed to resolve custom type");

    destroy_symbol_test_context(ctx);
    printf("COMPLETED test_type_name_resolution_custom\n");
    fflush(stdout);
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_type_name_resolution_undefined, ASTHRA_TEST_SEVERITY_HIGH) {
    printf("STARTING test_type_name_resolution_undefined...\n");
    fflush(stdout);

    SymbolTestContext *ctx = create_symbol_test_context();
    printf("Created test context: %p\n", (void *)ctx);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // Try to resolve undefined type
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "UndefinedType");
    printf("Resolved undefined type: %p (should be NULL)\n", (void *)entry);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_NULL(context, entry, "Should not resolve undefined type");

    // Check that an error was reported
    size_t error_count = semantic_get_error_count(ctx->analyzer);
    printf("Error count: %zu (should be > 0)\n", error_count);
    fflush(stdout);
    ASTHRA_TEST_ASSERT_GT(context, error_count, 0, "Should report error for undefined type");

    destroy_symbol_test_context(ctx);
    printf("COMPLETED test_type_name_resolution_undefined\n");
    fflush(stdout);
    ASTHRA_TEST_PASS(context);
}