/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Tests - Strings and Slices
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for string and slice operations:
 * - String concatenation
 * - Slice length access
 * - Slice bounds checking
 * - Slice FFI conversion
 */

#include "test_ffi_assembly_common.h"

// =============================================================================
// STRING OPERATION TESTS
// =============================================================================

static bool test_string_concatenation(void) {
    printf("Testing string concatenation generation...\n");
    
    // Test string concatenation
    Register left_reg = REG_RAX;
    Register right_reg = REG_RCX;
    Register result_reg = REG_RDX;
    
    bool result = ffi_generate_string_concatenation(g_generator, left_reg, right_reg, result_reg);
    TEST_ASSERT(result, "String concatenation generation failed");
    
    // Verify statistics
    size_t string_ops = 0;
    ffi_get_generation_statistics(g_generator, NULL, NULL, &string_ops, NULL, NULL, NULL);
    TEST_ASSERT(string_ops > 0, "String operation statistics not updated");
    
    TEST_SUCCESS();
}

// String interpolation test removed - feature deprecated for AI generation efficiency

static bool test_deterministic_string_operations(void) {
    printf("Testing deterministic string operations...\n");
    
    // Setup operands
    Register operand_regs[2] = {REG_RAX, REG_RCX};
    Register result_reg = REG_RDX;
    
    // Test deterministic string operations
    bool result = ffi_generate_deterministic_string_op(
        g_generator, STRING_OP_COMPARE, operand_regs, 2, result_reg);
    TEST_ASSERT(result, "Deterministic string comparison generation failed");
    
    result = ffi_generate_deterministic_string_op(
        g_generator, STRING_OP_EQUALS, operand_regs, 2, result_reg);
    TEST_ASSERT(result, "Deterministic string equals generation failed");
    
    TEST_SUCCESS();
}

// =============================================================================
// SLICE OPERATION TESTS
// =============================================================================

static bool test_slice_length_access(void) {
    printf("Testing slice length access generation...\n");
    
    // Test slice length access
    Register slice_reg = REG_RAX;
    Register result_reg = REG_RCX;
    
    bool result = ffi_generate_slice_length_access(g_generator, slice_reg, result_reg);
    TEST_ASSERT(result, "Slice length access generation failed");
    
    // Verify statistics
    size_t slice_ops = 0;
    ffi_get_generation_statistics(g_generator, NULL, NULL, NULL, &slice_ops, NULL, NULL);
    TEST_ASSERT(slice_ops > 0, "Slice operation statistics not updated");
    
    TEST_SUCCESS();
}

static bool test_slice_bounds_checking(void) {
    printf("Testing slice bounds checking generation...\n");
    
    // Test slice bounds checking
    Register slice_reg = REG_RAX;
    Register index_reg = REG_RCX;
    const char *error_label = ".L_bounds_error";
    
    bool result = ffi_generate_slice_bounds_check(g_generator, slice_reg, index_reg, error_label);
    TEST_ASSERT(result, "Slice bounds checking generation failed");
    
    TEST_SUCCESS();
}

static bool test_slice_to_ffi_conversion(void) {
    printf("Testing slice to FFI conversion generation...\n");
    
    // Test slice to FFI conversion
    Register slice_reg = REG_RAX;
    Register ptr_reg = REG_RCX;
    Register len_reg = REG_RDX;
    
    bool result = ffi_generate_slice_to_ffi(g_generator, slice_reg, ptr_reg, len_reg);
    TEST_ASSERT(result, "Slice to FFI conversion generation failed");
    
    TEST_SUCCESS();
}

static bool test_slice_creation(void) {
    printf("Testing slice creation generation...\n");
    
    // Create a slice creation expression
    ASTNodeList *args = ast_node_list_create(2);
    ast_node_list_add(&args, create_test_identifier("name")); // Data pointer
    ast_node_list_add(&args, create_test_integer_literal(10)); // Length
    
    ASTNode *call_expr = create_test_call_expr("slice_create", args);
    
    // Generate slice creation code
    Register data_reg = REG_RAX;
    Register len_reg = REG_RCX;
    Register result_reg = REG_RDX;
    bool is_mutable = true;
    
    bool result = ffi_generate_slice_creation(g_generator, data_reg, len_reg, result_reg, is_mutable);
    TEST_ASSERT(result, "Slice creation generation failed");
    
    // Cleanup
    ast_free_node(call_expr);
    
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
    
    printf("=== String and Slice Tests ===\n");
    
    // Run string tests
    run_test(test_string_concatenation);
    run_test(test_deterministic_string_operations);
    
    // Run slice tests
    run_test(test_slice_length_access);
    run_test(test_slice_bounds_checking);
    run_test(test_slice_to_ffi_conversion);
    run_test(test_slice_creation);
    
    // Cleanup and report
    teardown_test_suite();
    
    return (g_tests_passed == g_tests_run) ? 0 : 1;
} 
