/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Tests - Call Operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for FFI call functionality:
 * - External function calls
 * - Variadic function calls
 * - Parameter marshaling
 * - Return value unmarshaling
 * - Ownership transfer
 */

#include "test_ffi_assembly_common.h"

// =============================================================================
// FFI CALL GENERATION TESTS
// =============================================================================

static bool test_ffi_extern_call_generation(void) {
    printf("Testing FFI extern call generation...\n");

    // Create test arguments
    ASTNodeList *args = ast_node_list_create(2);
    ast_node_list_add(&args, create_test_integer_literal(42));
    ast_node_list_add(&args, create_test_string_literal("test"));

    // Create FFI call expression
    ASTNode *call_expr = create_test_call_expr("external_function", args);

    // Generate FFI call
    bool result = ffi_generate_extern_call(g_generator, call_expr);
    TEST_ASSERT(result, "FFI extern call generation failed");

    // Verify statistics
    size_t ffi_calls = 0;
    ffi_get_generation_statistics(g_generator, &ffi_calls, NULL, NULL, NULL, NULL, NULL);
    TEST_ASSERT(ffi_calls > 0, "FFI call statistics not updated");

    // Cleanup - ast_free_node will free the args list
    ast_free_node(call_expr);

    TEST_SUCCESS();
}

static bool test_ffi_variadic_call_generation(void) {
    printf("Testing FFI variadic call generation...\n");

    // Create test arguments (fixed + variadic)
    ASTNodeList *args = ast_node_list_create(4);
    ast_node_list_add(&args, create_test_string_literal("format: %d, %s"));
    ast_node_list_add(&args, create_test_integer_literal(123));
    ast_node_list_add(&args, create_test_string_literal("text"));
    ast_node_list_add(&args, create_test_integer_literal(456)); // Extra arg

    // Create FFI call expression
    ASTNode *call_expr = create_test_call_expr("printf", args);

    // Generate variadic FFI call (first arg is format string, rest are variadic)
    bool result = ffi_generate_variadic_call(g_generator, call_expr, 1);
    TEST_ASSERT(result, "FFI variadic call generation failed");

    // Cleanup
    ast_free_node(call_expr);

    TEST_SUCCESS();
}

static bool test_ffi_parameter_marshaling(void) {
    printf("Testing FFI parameter marshaling...\n");

    // Create test parameter node
    ASTNode *param = create_test_identifier("test_var");

    // Test direct marshaling
    bool result = ffi_generate_parameter_marshaling(g_generator, param, FFI_MARSHAL_DIRECT,
                                                    FFI_OWNERSHIP_COPY, ASTHRA_REG_RDI);
    TEST_ASSERT(result, "Direct parameter marshaling failed");

    // Test pointer marshaling
    result = ffi_generate_parameter_marshaling(g_generator, param, FFI_MARSHAL_POINTER,
                                               FFI_OWNERSHIP_MOVE, ASTHRA_REG_RSI);
    TEST_ASSERT(result, "Pointer parameter marshaling failed");

    // Cleanup
    free_test_identifier(param);

    TEST_SUCCESS();
}

static bool test_ffi_return_unmarshaling(void) {
    printf("Testing FFI return value unmarshaling...\n");

    // Test direct unmarshaling
    bool result = ffi_generate_return_unmarshaling(
        g_generator, FFI_MARSHAL_DIRECT, FFI_OWNERSHIP_COPY, ASTHRA_REG_RAX, ASTHRA_REG_RCX);
    TEST_ASSERT(result, "Direct return unmarshaling failed");

    // Test pointer unmarshaling
    result = ffi_generate_return_unmarshaling(g_generator, FFI_MARSHAL_POINTER, FFI_OWNERSHIP_MOVE,
                                              ASTHRA_REG_RAX, ASTHRA_REG_RCX);
    TEST_ASSERT(result, "Pointer return unmarshaling failed");

    TEST_SUCCESS();
}

static bool test_ownership_transfer(void) {
    printf("Testing ownership transfer generation...\n");

    // Create the code generator and assembly generator
    CodeGenerator *code_gen = g_generator->base_generator;

    // Test generation of move semantics
    Register src_reg = ASTHRA_REG_RAX;
    Register dst_reg = ASTHRA_REG_RCX;

    bool result =
        ffi_generate_ownership_transfer(g_generator, src_reg, dst_reg, FFI_OWNERSHIP_MOVE);
    TEST_ASSERT(result, "Ownership transfer (move) generation failed");

    // Test generation of copy semantics
    result = ffi_generate_ownership_transfer(g_generator, src_reg, dst_reg, FFI_OWNERSHIP_COPY);
    TEST_ASSERT(result, "Ownership transfer (copy) generation failed");

    // Test generation of borrow semantics
    result = ffi_generate_ownership_transfer(g_generator, src_reg, dst_reg, FFI_OWNERSHIP_BORROW);
    TEST_ASSERT(result, "Ownership transfer (borrow) generation failed");

    TEST_SUCCESS();
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(int argc, char *argv[]) {
    // Initialize test suite
    if (!setup_test_suite()) {
        fprintf(stderr, "Failed to set up test suite\n");
        return 1;
    }

    printf("=== FFI Call Tests ===\n");

    // Run tests
    run_test(test_ffi_extern_call_generation);
    run_test(test_ffi_variadic_call_generation);
    run_test(test_ffi_parameter_marshaling);
    run_test(test_ffi_return_unmarshaling);
    run_test(test_ownership_transfer);

    // Cleanup and report
    teardown_test_suite();

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
