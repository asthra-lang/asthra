/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Tests - Optimization and Validation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for optimization features and assembly validation
 */

#include "test_ffi_assembly_common.h"

// =============================================================================
// OPTIMIZATION TESTS
// =============================================================================

static bool test_zero_cost_abstractions(void) {
    printf("Testing zero-cost abstractions optimization...\n");

    // Optimize zero-cost abstractions
    bool result = ffi_optimize_zero_cost_abstractions(g_generator);
    TEST_ASSERT(result, "Zero-cost abstractions optimization failed");

    TEST_SUCCESS();
}

static bool test_dead_code_elimination(void) {
    printf("Testing dead code elimination...\n");

    // Generate some dead code and optimize
    bool result = ffi_optimize_dead_code_elimination(g_generator);
    TEST_ASSERT(result, "Dead code elimination failed");

    TEST_SUCCESS();
}

static bool test_register_allocation_optimization(void) {
    printf("Testing register allocation optimization...\n");

    // Optimize register allocation
    bool result = ffi_optimize_register_allocation(g_generator);
    TEST_ASSERT(result, "Register allocation optimization failed");

    TEST_SUCCESS();
}

static bool test_instruction_scheduling(void) {
    printf("Testing instruction scheduling...\n");

    // Optimize instruction scheduling
    bool result = ffi_optimize_instruction_scheduling(g_generator);
    TEST_ASSERT(result, "Instruction scheduling optimization failed");

    TEST_SUCCESS();
}

static bool test_loop_optimization(void) {
    printf("Testing loop optimization...\n");

    // Create a simple loop for optimization
    ASTNode *loop_block = ast_create_node(AST_BLOCK, (SourceLocation){0, 0, 0});
    loop_block->data.block.statements = ast_node_list_create(1);

    // Add a simple statement to the loop
    ASTNode *stmt = create_test_call_expr("loop_body", NULL);
    ast_node_list_add(&loop_block->data.block.statements, stmt);

    // Optimize the loop
    bool result = ffi_optimize_loop(g_generator, loop_block);
    TEST_ASSERT(result, "Loop optimization failed");

    // Cleanup
    ast_free_node(loop_block);

    TEST_SUCCESS();
}

static bool test_constant_folding(void) {
    printf("Testing constant folding...\n");

    // Create constant expression
    ASTNode *left = create_test_integer_literal(10);
    ASTNode *right = create_test_integer_literal(20);

    ASTNode *binary_expr = ast_create_node(AST_BINARY_EXPR, (SourceLocation){0, 0, 0});
    binary_expr->data.binary_expr.left = left;
    binary_expr->data.binary_expr.right = right;
    binary_expr->data.binary_expr.operator= BINARY_OP_ADD;

    // Optimize constant folding
    bool result = ffi_optimize_constant_folding(g_generator, binary_expr);
    TEST_ASSERT(result, "Constant folding optimization failed");

    // Cleanup
    ast_free_node(binary_expr);

    TEST_SUCCESS();
}

static bool test_inlining_optimization(void) {
    printf("Testing function inlining...\n");

    // Create a function call for inlining
    ASTNodeList *args = ast_node_list_create(1);
    ast_node_list_add(&args, create_test_integer_literal(42));

    ASTNode *call_expr = create_test_call_expr("small_function", args);

    // Optimize inlining
    bool result = ffi_optimize_inline_function(g_generator, call_expr);
    TEST_ASSERT(result, "Function inlining optimization failed");

    // Cleanup
    ast_free_node(call_expr);

    TEST_SUCCESS();
}

// =============================================================================
// ENHANCED EXPRESSION TESTS
// =============================================================================

static bool test_postfix_expression_generation(void) {
    printf("Testing postfix expression generation...\n");

    // Create postfix expression
    ASTNode *base = create_test_identifier("object");
    ASTNodeList *suffixes = ast_node_list_create(1);
    ast_node_list_add(&suffixes, create_test_identifier("field"));

    ASTNode *postfix_expr = ast_create_node(AST_POSTFIX_EXPR, (SourceLocation){0, 0, 0});
    postfix_expr->data.postfix_expr.base = base;
    postfix_expr->data.postfix_expr.suffixes = suffixes;

    // Generate postfix expression
    bool result = ffi_generate_postfix_expression(g_generator, postfix_expr, ASTHRA_REG_RAX);
    TEST_ASSERT(result, "Postfix expression generation failed");

    // Cleanup - only free the main node, it will handle the list
    ast_free_node(postfix_expr);

    TEST_SUCCESS();
}

static bool test_array_access_optimization(void) {
    printf("Testing array access optimization...\n");

    // Create array access expression
    ASTNode *array = create_test_identifier("array");
    ASTNode *index = create_test_integer_literal(5);

    ASTNode *array_access = ast_create_node(AST_ARRAY_ACCESS, (SourceLocation){0, 0, 0});
    array_access->data.array_access.array = array;
    array_access->data.array_access.index = index;

    // Optimize array access
    bool result = ffi_optimize_array_access(g_generator, array_access);
    TEST_ASSERT(result, "Array access optimization failed");

    // Cleanup
    ast_free_node(array_access);

    TEST_SUCCESS();
}

static bool test_conditional_move_optimization(void) {
    printf("Testing conditional move optimization...\n");

    // Create conditional expression
    ASTNode *condition = create_test_identifier("flag");
    ASTNode *true_expr = create_test_integer_literal(1);
    ASTNode *false_expr = create_test_integer_literal(0);

    ASTNode *conditional = ast_create_node(AST_CONDITIONAL_EXPR, (SourceLocation){0, 0, 0});
    conditional->data.conditional_expr.condition = condition;
    conditional->data.conditional_expr.true_expr = true_expr;
    conditional->data.conditional_expr.false_expr = false_expr;

    // Optimize to conditional move
    bool result = ffi_optimize_conditional_move(g_generator, conditional, ASTHRA_REG_RAX);
    TEST_ASSERT(result, "Conditional move optimization failed");

    // Cleanup
    ast_free_node(conditional);

    TEST_SUCCESS();
}

// =============================================================================
// VALIDATION AND OUTPUT TESTS
// =============================================================================

static bool test_assembly_validation(void) {
    printf("Testing generated assembly validation...\n");

    // Validate generated assembly
    bool result = ffi_validate_generated_assembly(g_generator);
    TEST_ASSERT(result, "Generated assembly validation failed");

    TEST_SUCCESS();
}

static bool test_nasm_output(void) {
    printf("Testing NASM assembly output...\n");

    char output_buffer[8192];

    // Generate NASM output
    bool result = ffi_print_nasm_assembly(g_generator, output_buffer, sizeof(output_buffer));
    TEST_ASSERT(result, "NASM assembly output generation failed");
    TEST_ASSERT(strlen(output_buffer) > 0, "NASM output is empty");

    printf("Generated NASM assembly (first 200 chars):\n%.200s...\n", output_buffer);

    TEST_SUCCESS();
}

static bool test_gas_output(void) {
    printf("Testing GNU AS assembly output...\n");

    char output_buffer[8192];

    // Generate GNU AS output
    bool result = ffi_print_gas_assembly(g_generator, output_buffer, sizeof(output_buffer));
    TEST_ASSERT(result, "GNU AS assembly output generation failed");
    TEST_ASSERT(strlen(output_buffer) > 0, "GNU AS output is empty");

    printf("Generated GNU AS assembly (first 200 chars):\n%.200s...\n", output_buffer);

    TEST_SUCCESS();
}

static bool test_generation_statistics(void) {
    printf("Testing generation statistics...\n");

    size_t ffi_calls, pattern_matches, string_ops, slice_ops, security_ops, spawn_stmts;

    // Get statistics
    ffi_get_generation_statistics(g_generator, &ffi_calls, &pattern_matches, &string_ops,
                                  &slice_ops, &security_ops, &spawn_stmts);

    printf("Generation Statistics:\n");
    printf("  FFI calls: %zu\n", ffi_calls);
    printf("  Pattern matches: %zu\n", pattern_matches);
    printf("  String operations: %zu\n", string_ops);
    printf("  Slice operations: %zu\n", slice_ops);
    printf("  Security operations: %zu\n", security_ops);
    printf("  Spawn statements: %zu\n", spawn_stmts);

    TEST_ASSERT(ffi_calls > 0 || pattern_matches > 0 || string_ops > 0 || slice_ops > 0 ||
                    security_ops > 0 || spawn_stmts > 0,
                "No operations were generated");

    TEST_SUCCESS();
}

static bool test_code_size_optimization(void) {
    printf("Testing code size optimization...\n");

    size_t original_size = ffi_get_code_size(g_generator);

    // Optimize for size
    bool result = ffi_optimize_code_size(g_generator);
    TEST_ASSERT(result, "Code size optimization failed");

    size_t optimized_size = ffi_get_code_size(g_generator);
    printf("  Original size: %zu bytes, Optimized size: %zu bytes\n", original_size,
           optimized_size);

    TEST_SUCCESS();
}

static bool test_performance_profiling(void) {
    printf("Testing performance profiling...\n");

    // Enable profiling
    bool result = ffi_enable_profiling(g_generator, true);
    TEST_ASSERT(result, "Failed to enable profiling");

    // Get profiling data
    FFIProfilingData data;
    result = ffi_get_profiling_data(g_generator, &data);
    TEST_ASSERT(result, "Failed to get profiling data");

    printf("  Profiling data: %zu instructions, %zu register spills\n", data.instruction_count,
           data.register_spills);

    TEST_SUCCESS();
}

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

static bool test_complete_program_generation(void) {
    printf("Testing complete program generation...\n");

    // Create a simple program with multiple features
    ASTNode *program = ast_create_node(AST_PROGRAM, (SourceLocation){0, 0, 0});
    program->data.program.package_decl = NULL;
    program->data.program.imports = ast_node_list_create(0);
    program->data.program.declarations = ast_node_list_create(0);

    // Generate complete program
    bool result = ffi_generate_program(g_generator, program);
    TEST_ASSERT(result, "Complete program generation failed");

    // Cleanup
    ast_free_node(program);

    TEST_SUCCESS();
}

static bool test_cross_platform_compatibility(void) {
    printf("Testing cross-platform compatibility...\n");

    // Test different calling conventions
    bool result = ffi_set_calling_convention(g_generator, CALLING_CONV_SYSTEM_V_AMD64);
    TEST_ASSERT(result, "Failed to set System V calling convention");

    result = ffi_set_calling_convention(g_generator, CALLING_CONV_MS_X64);
    TEST_ASSERT(result, "Failed to set Microsoft x64 calling convention");

    TEST_SUCCESS();
}

// =============================================================================
// TEST FUNCTION ARRAY
// =============================================================================

test_function_t ffi_optimization_tests[] = {
    test_zero_cost_abstractions,
    test_dead_code_elimination,
    test_register_allocation_optimization,
    test_instruction_scheduling,
    test_loop_optimization,
    test_constant_folding,
    test_inlining_optimization,
    test_postfix_expression_generation,
    test_array_access_optimization,
    test_conditional_move_optimization,
    test_assembly_validation,
    test_nasm_output,
    test_gas_output,
    test_generation_statistics,
    test_code_size_optimization,
    test_performance_profiling,
    test_complete_program_generation,
    test_cross_platform_compatibility,
};

const size_t ffi_optimization_test_count =
    sizeof(ffi_optimization_tests) / sizeof(ffi_optimization_tests[0]);

// =============================================================================
// TEST SUITE RUNNER
// =============================================================================

int run_ffi_optimization_tests(void) {
    printf("Running FFI Optimization and Validation Tests...\n");
    printf("================================================\n\n");

    if (!setup_test_suite()) {
        return 1;
    }

    // Enable all optimization features
    g_generator->config.optimize_string_operations = true;
    g_generator->config.optimize_pattern_matching = true;
    g_generator->config.enable_bounds_checking = true;
    g_generator->config.enable_security_features = true;
    g_generator->config.enable_concurrency = true;
    g_generator->config.pic_mode = true;

    for (size_t i = 0; i < ffi_optimization_test_count; i++) {
        run_test(ffi_optimization_tests[i]);
    }

    teardown_test_suite();

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}
