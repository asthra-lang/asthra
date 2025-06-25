/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Tests - Method Resolution Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for method resolution on types
 */

#include "test_symbol_resolution_common.h"

// =============================================================================
// METHOD RESOLUTION TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_method_resolution_basic, ASTHRA_TEST_SEVERITY_MEDIUM) {
    SymbolTestContext *ctx = create_symbol_test_context();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // Create struct type
    TypeDescriptor *point_type = type_descriptor_create_struct("Point", 2);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, point_type, "Failed to create Point type");

    // Declare struct type
    bool success1 = semantic_declare_symbol(ctx->analyzer, "Point", SYMBOL_TYPE, point_type,
                                            create_mock_ast_node(AST_STRUCT_DECL));
    ASTHRA_TEST_ASSERT_TRUE(context, success1, "Failed to declare Point type");

    // Create method type
    TypeDescriptor *method_type = type_descriptor_create_function();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, method_type, "Failed to create method type");

    // Declare method (simplified - in real implementation, this would be more complex)
    bool success2 = semantic_declare_symbol(ctx->analyzer, "Point.distance", SYMBOL_METHOD,
                                            method_type, create_mock_ast_node(AST_FUNCTION_DECL));
    ASTHRA_TEST_ASSERT_TRUE(context, success2, "Failed to declare method");

    // Resolve method
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "Point.distance");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, entry, "Failed to resolve method");

    destroy_symbol_test_context(ctx);
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_method_resolution_undefined, ASTHRA_TEST_SEVERITY_MEDIUM) {
    SymbolTestContext *ctx = create_symbol_test_context();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // Try to resolve undefined method
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "UndefinedType.invalid_method");
    ASTHRA_TEST_ASSERT_NULL(context, entry, "Should not resolve undefined method");

    destroy_symbol_test_context(ctx);
    ASTHRA_TEST_PASS(context);
}