/**
 * Asthra Programming Language Compiler
 * Symbol Resolution Tests - Error Handling Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for error handling in symbol resolution
 */

#include "test_symbol_resolution_common.h"

// =============================================================================
// ERROR CASE TESTS FOR SYMBOL-RELATED ERRORS
// =============================================================================

ASTHRA_TEST_DEFINE(test_error_undefined_symbol, ASTHRA_TEST_SEVERITY_CRITICAL) {
    SymbolTestContext *ctx = create_symbol_test_context();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // Clear any existing errors
    semantic_clear_errors(ctx->analyzer);

    // Try to resolve undefined symbol
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "undefined_symbol");
    ASTHRA_TEST_ASSERT_NULL(context, entry, "Should not resolve undefined symbol");

    // Check error details
    size_t error_count = semantic_get_error_count(ctx->analyzer);
    ASTHRA_TEST_ASSERT_EQ(context, error_count, 1, "Should report exactly one error");

    const SemanticError *errors = semantic_get_errors(ctx->analyzer);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, errors, "Should have error details");
    ASTHRA_TEST_ASSERT_TRUE(context, errors->code == SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                            "Should report UNDEFINED_SYMBOL error");

    destroy_symbol_test_context(ctx);
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_error_duplicate_symbol, ASTHRA_TEST_SEVERITY_HIGH) {
    SymbolTestContext *ctx = create_symbol_test_context();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // Clear any existing errors
    semantic_clear_errors(ctx->analyzer);

    // Declare symbol first time
    bool success1 =
        semantic_declare_symbol(ctx->analyzer, "duplicate_test", SYMBOL_VARIABLE,
                                semantic_get_builtin_type(ctx->analyzer, "int"),
                                create_mock_variable_declaration("duplicate_test", "int"));
    ASTHRA_TEST_ASSERT_TRUE(context, success1, "First declaration should succeed");

    // Try to declare same symbol again
    bool success2 =
        semantic_declare_symbol(ctx->analyzer, "duplicate_test", SYMBOL_VARIABLE,
                                semantic_get_builtin_type(ctx->analyzer, "int"),
                                create_mock_variable_declaration("duplicate_test", "int"));
    ASTHRA_TEST_ASSERT_FALSE(context, success2, "Duplicate declaration should fail");

    // Check error details
    size_t error_count = semantic_get_error_count(ctx->analyzer);
    ASTHRA_TEST_ASSERT_GT(context, error_count, 0, "Should report error for duplicate symbol");

    const SemanticError *errors = semantic_get_errors(ctx->analyzer);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, errors, "Should have error details");
    ASTHRA_TEST_ASSERT_TRUE(context, errors->code == SEMANTIC_ERROR_DUPLICATE_SYMBOL,
                            "Should report DUPLICATE_SYMBOL error");

    destroy_symbol_test_context(ctx);
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_error_undefined_type, ASTHRA_TEST_SEVERITY_HIGH) {
    SymbolTestContext *ctx = create_symbol_test_context();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // Clear any existing errors
    semantic_clear_errors(ctx->analyzer);

    // Try to resolve undefined type
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "UndefinedType");
    ASTHRA_TEST_ASSERT_NULL(context, entry, "Should not resolve undefined type");

    // Check error details
    size_t error_count = semantic_get_error_count(ctx->analyzer);
    ASTHRA_TEST_ASSERT_GT(context, error_count, 0, "Should report error for undefined type");

    const SemanticError *errors = semantic_get_errors(ctx->analyzer);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, errors, "Should have error details");
    // Could be UNDEFINED_SYMBOL or UNDEFINED_TYPE depending on context
    bool valid_error = (errors->code == SEMANTIC_ERROR_UNDEFINED_SYMBOL ||
                        errors->code == SEMANTIC_ERROR_UNDEFINED_TYPE);
    ASTHRA_TEST_ASSERT_TRUE(context, valid_error, "Should report appropriate undefined error");

    destroy_symbol_test_context(ctx);
    ASTHRA_TEST_PASS(context);
}

// =============================================================================
// VISIBILITY MODIFIER TEST
// =============================================================================

ASTHRA_TEST_DEFINE(test_visibility_public_symbol, ASTHRA_TEST_SEVERITY_MEDIUM) {
    SymbolTestContext *ctx = create_symbol_test_context();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, ctx, "Failed to create test context");

    // In a simplified test, we assume all symbols are accessible
    // A full implementation would test pub/priv visibility rules

    // Declare a "public" symbol
    bool success = semantic_declare_symbol(ctx->analyzer, "public_func", SYMBOL_FUNCTION,
                                           type_descriptor_create_function(),
                                           create_mock_ast_node(AST_FUNCTION_DECL));
    ASTHRA_TEST_ASSERT_TRUE(context, success, "Failed to declare public function");

    // Should be able to resolve public symbol
    SymbolEntry *entry = semantic_resolve_identifier(ctx->analyzer, "public_func");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, entry, "Should resolve public symbol");

    destroy_symbol_test_context(ctx);
    ASTHRA_TEST_PASS(context);
}