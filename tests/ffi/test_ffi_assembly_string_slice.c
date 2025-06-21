/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Tests - String and Slice Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for string operations (concatenation) and slice operations
 */

#include "test_ffi_assembly_common.h"

// =============================================================================
// STRING OPERATION TESTS
// =============================================================================

static bool test_string_concatenation(void) {
    printf("Testing string concatenation...\n");
    
    // Generate string concatenation
    bool result = ffi_generate_string_concatenation(g_generator, REG_RDI, REG_RSI, REG_RAX);
    TEST_ASSERT(result, "String concatenation generation failed");
    
    // Verify statistics
    size_t string_ops = 0;
    ffi_get_generation_statistics(g_generator, NULL, NULL, &string_ops, NULL, NULL, NULL);
    TEST_ASSERT(string_ops > 0, "String operation statistics not updated");
    
    TEST_SUCCESS();
}

// String interpolation test removed - feature deprecated for AI generation efficiency

// =============================================================================
// SLICE OPERATION TESTS
// =============================================================================

static bool test_slice_length_access(void) {
    printf("Testing slice length access...\n");
    
    // Generate slice length access
    bool result = ffi_generate_slice_length_access(g_generator, REG_RDI, REG_RAX);
    TEST_ASSERT(result, "Slice length access generation failed");
    
    // Verify statistics
    size_t slice_ops = 0;
    ffi_get_generation_statistics(g_generator, NULL, NULL, NULL, &slice_ops, NULL, NULL);
    TEST_ASSERT(slice_ops > 0, "Slice operation statistics not updated");
    
    TEST_SUCCESS();
}

static bool test_slice_bounds_checking(void) {
    printf("Testing slice bounds checking...\n");
    
    char *error_label = "bounds_error";
    
    // Generate slice bounds check
    bool result = ffi_generate_slice_bounds_check(g_generator, REG_RDI, REG_RSI, error_label);
    TEST_ASSERT(result, "Slice bounds check generation failed");
    
    TEST_SUCCESS();
}

static bool test_slice_to_ffi_conversion(void) {
    printf("Testing slice to FFI conversion...\n");
    
    // Generate slice to FFI conversion
    bool result = ffi_generate_slice_to_ffi(g_generator, REG_RDI, REG_RSI, REG_RDX);
    TEST_ASSERT(result, "Slice to FFI conversion failed");
    
    TEST_SUCCESS();
}

static bool test_slice_creation(void) {
    printf("Testing slice creation...\n");
    
    // Create array literal
    ASTNodeList *elements = ast_node_list_create(3);
    ast_node_list_add(&elements, create_test_integer_literal(1));
    ast_node_list_add(&elements, create_test_integer_literal(2));
    ast_node_list_add(&elements, create_test_integer_literal(3));
    
    ASTNode *array_literal = ast_create_node(AST_ARRAY_LITERAL, (SourceLocation){0, 0, 0});
    array_literal->data.array_literal.elements = elements;
    
    // Generate slice creation
    bool result = ffi_generate_slice_creation(g_generator, array_literal, REG_RAX);
    TEST_ASSERT(result, "Slice creation generation failed");
    
    // Cleanup - only free the main node, it will handle the list
    ast_free_node(array_literal);
    
    TEST_SUCCESS();
}

static bool test_slice_indexing(void) {
    printf("Testing slice indexing...\n");
    
    // Generate slice indexing with bounds check
    bool result = ffi_generate_slice_index_access(g_generator, REG_RDI, REG_RSI, REG_RAX, true);
    TEST_ASSERT(result, "Slice indexing generation failed");
    
    // Test without bounds check
    result = ffi_generate_slice_index_access(g_generator, REG_RDI, REG_RSI, REG_RAX, false);
    TEST_ASSERT(result, "Unsafe slice indexing generation failed");
    
    TEST_SUCCESS();
}

static bool test_slice_subslicing(void) {
    printf("Testing slice subslicing...\n");
    
    // Generate subslice operation [start:end]
    bool result = ffi_generate_slice_subslice(g_generator, REG_RDI, REG_RSI, REG_RDX, REG_RAX);
    TEST_ASSERT(result, "Slice subslicing generation failed");
    
    TEST_SUCCESS();
}

static bool test_slice_iteration(void) {
    printf("Testing slice iteration...\n");
    
    // Generate slice iteration setup
    bool result = ffi_generate_slice_iteration_setup(g_generator, REG_RDI, REG_RSI, REG_RDX);
    TEST_ASSERT(result, "Slice iteration setup failed");
    
    TEST_SUCCESS();
}

// =============================================================================
// COMBINED STRING AND SLICE TESTS
// =============================================================================

static bool test_string_to_slice_conversion(void) {
    printf("Testing string to slice conversion...\n");
    
    // Generate string to slice conversion
    bool result = ffi_generate_string_to_slice(g_generator, REG_RDI, REG_RAX);
    TEST_ASSERT(result, "String to slice conversion failed");
    
    TEST_SUCCESS();
}

static bool test_slice_to_string_conversion(void) {
    printf("Testing slice to string conversion...\n");
    
    // Generate slice to string conversion
    bool result = ffi_generate_slice_to_string(g_generator, REG_RDI, REG_RAX);
    TEST_ASSERT(result, "Slice to string conversion failed");
    
    TEST_SUCCESS();
}

// =============================================================================
// TEST FUNCTION ARRAY
// =============================================================================

test_function_t ffi_string_slice_tests[] = {
    test_string_concatenation,
    test_slice_length_access,
    test_slice_bounds_checking,
    test_slice_to_ffi_conversion,
    test_slice_creation,
    test_slice_indexing,
    test_slice_subslicing,
    test_slice_iteration,
    test_string_to_slice_conversion,
    test_slice_to_string_conversion,
};

const size_t ffi_string_slice_test_count = sizeof(ffi_string_slice_tests) / sizeof(ffi_string_slice_tests[0]);

// =============================================================================
// TEST SUITE RUNNER
// =============================================================================

int run_ffi_string_slice_tests(void) {
    printf("Running FFI String and Slice Tests...\n");
    printf("=====================================\n\n");
    
    if (!setup_test_suite()) {
        return 1;
    }
    
    // Enable string optimization and bounds checking
    g_generator->config.optimize_string_operations = true;
    g_generator->config.enable_bounds_checking = true;
    
    for (size_t i = 0; i < ffi_string_slice_test_count; i++) {
        run_test(ffi_string_slice_tests[i]);
    }
    
    teardown_test_suite();
    
    return (g_tests_passed == g_tests_run) ? 0 : 1;
} 
