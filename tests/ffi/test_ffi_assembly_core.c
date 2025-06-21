/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Tests - Core FFI Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for core FFI call generation and parameter marshaling
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
    
    // Create test arguments (printf-style)
    ASTNodeList *args = ast_node_list_create(3);
    ast_node_list_add(&args, create_test_string_literal("Hello %s %d"));
    ast_node_list_add(&args, create_test_string_literal("World"));
    ast_node_list_add(&args, create_test_integer_literal(2024));
    
    // Create variadic call expression
    ASTNode *call_expr = create_test_call_expr("printf", args);
    
    // Generate variadic call (1 fixed argument, 2 variadic)
    bool result = ffi_generate_variadic_call(g_generator, call_expr, 1);
    TEST_ASSERT(result, "FFI variadic call generation failed");
    
    // Cleanup - ast_free_node will free the args list
    ast_free_node(call_expr);
    
    TEST_SUCCESS();
}

static bool test_ffi_parameter_marshaling(void) {
    printf("Testing FFI parameter marshaling...\n");
    
    // Test direct marshaling
    ASTNode *int_param = create_test_integer_literal(123);
    bool result = ffi_generate_parameter_marshaling(g_generator, int_param, 
                                                   FFI_MARSHAL_DIRECT, 
                                                   FFI_OWNERSHIP_NONE, ASTHRA_REG_RDI);
    TEST_ASSERT(result, "Direct parameter marshaling failed");
    
    // Test string marshaling
    ASTNode *string_param = create_test_string_literal("test string");
    result = ffi_generate_parameter_marshaling(g_generator, string_param,
                                              FFI_MARSHAL_STRING,
                                              FFI_OWNERSHIP_MOVE, ASTHRA_REG_RSI);
    TEST_ASSERT(result, "String parameter marshaling failed");
    
    // Cleanup
    ast_free_node(int_param);
    ast_free_node(string_param);
    
    TEST_SUCCESS();
}

static bool test_ownership_transfer(void) {
    printf("Testing ownership transfer...\n");
    
    // Test full ownership transfer
    bool result = ffi_generate_ownership_transfer(g_generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, FFI_OWNERSHIP_MOVE);
    TEST_ASSERT(result, "Full ownership transfer failed");
    
    // Test no ownership transfer
    result = ffi_generate_ownership_transfer(g_generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, FFI_OWNERSHIP_NONE);
    TEST_ASSERT(result, "No ownership transfer failed");
    
    TEST_SUCCESS();
}

static bool test_c_struct_layout_generation(void) {
    printf("Testing C-compatible struct layout generation...\n");
    
    // Create struct declaration
    ASTNode *struct_decl = ast_create_node(AST_STRUCT_DECL, (SourceLocation){0, 0, 0});
    struct_decl->data.struct_decl.name = strdup("TestStruct");
    struct_decl->data.struct_decl.fields = ast_node_list_create(0);
    
    // Generate C struct layout
    bool result = ffi_generate_c_struct_layout(g_generator, struct_decl, false);
    TEST_ASSERT(result, "C struct layout generation failed");
    
    // Test packed struct
    result = ffi_generate_c_struct_layout(g_generator, struct_decl, true);
    TEST_ASSERT(result, "Packed C struct layout generation failed");
    
    // Cleanup
    ast_free_node(struct_decl);
    
    TEST_SUCCESS();
}

// =============================================================================
// TEST FUNCTION ARRAY
// =============================================================================

test_function_t ffi_core_tests[] = {
    test_ffi_extern_call_generation,
    test_ffi_variadic_call_generation,
    test_ffi_parameter_marshaling,
    test_ownership_transfer,
    test_c_struct_layout_generation,
};

const size_t ffi_core_test_count = sizeof(ffi_core_tests) / sizeof(ffi_core_tests[0]);

// =============================================================================
// TEST SUITE RUNNER
// =============================================================================

int run_ffi_core_tests(void) {
    printf("Running FFI Core Tests...\n");
    printf("========================\n\n");
    
    if (!setup_test_suite()) {
        return 1;
    }
    
    // Enable all features for testing
    g_generator->config.enable_bounds_checking = true;
    g_generator->config.enable_security_features = true;
    g_generator->config.enable_concurrency = true;
    g_generator->config.optimize_string_operations = true;
    g_generator->config.optimize_pattern_matching = true;
    g_generator->config.pic_mode = true;
    
    for (size_t i = 0; i < ffi_core_test_count; i++) {
        run_test(ffi_core_tests[i]);
    }
    
    teardown_test_suite();
    
    return (g_tests_passed == g_tests_run) ? 0 : 1;
} 
