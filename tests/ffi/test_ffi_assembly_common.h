/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Tests - Common Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Common definitions and utilities for FFI assembly generator tests
 */

#ifndef ASTHRA_TEST_FFI_ASSEMBLY_COMMON_H
#define ASTHRA_TEST_FFI_ASSEMBLY_COMMON_H

#include "ffi_assembly_generator.h"
#include "ast.h"
#include "code_generator.h"
#include "../framework/semantic_test_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

// Test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s - %s\n", __func__, message); \
            return false; \
        } \
    } while(0)

#define TEST_SUCCESS() \
    do { \
        printf("PASS: %s\n", __func__); \
        return true; \
    } while(0)

// Test function type
typedef bool (*test_function_t)(void);

// Global test state
extern FFIAssemblyGenerator *g_generator;
extern size_t g_tests_run;
extern size_t g_tests_passed;

// Memory leak debug flag
extern bool g_ignore_leaks;

// =============================================================================
// MOCK IMPLEMENTATIONS FOR MISSING DEPENDENCIES
// =============================================================================

// Mock semantic analyzer for testing
typedef struct MockSemanticAnalyzer {
    int dummy;
} MockSemanticAnalyzer;

// Mock type info for testing
typedef struct MockTypeInfo {
    int base_type;
    char *name;
} MockTypeInfo;

// Mock implementations of missing functions
MockSemanticAnalyzer *mock_semantic_analyzer_create(void);
void mock_semantic_analyzer_destroy(MockSemanticAnalyzer *analyzer);

// Helper function to set up mock variables in the symbol table
bool setup_mock_variables(FFIAssemblyGenerator *generator);

// =============================================================================
// TEST HELPER FUNCTIONS
// =============================================================================

// AST Node creation helpers
ASTNode *create_test_identifier(const char *name);
ASTNode *create_test_integer_literal(int64_t value);
ASTNode *create_test_string_literal(const char *value);
ASTNode *create_test_call_expr(const char *func_name, ASTNodeList *args);
ASTNode *create_test_spawn_stmt(const char *function_name, ASTNodeList *args);
ASTNode *create_test_match_stmt(ASTNode *expression, ASTNodeList *arms);
ASTNode *create_test_unsafe_block(ASTNode *block);

// Custom cleanup for test identifiers with TypeInfo
void free_test_identifier(ASTNode *node);

// Additional mock FFI function declarations for extended tests
// These functions should return true for successful mocking in tests

// String operations
bool ffi_generate_string_length(FFIAssemblyGenerator *gen, int src_reg, int dest_reg);
bool ffi_generate_string_comparison(FFIAssemblyGenerator *gen, int left_reg, int right_reg, int result_reg);
bool ffi_generate_string_to_slice(FFIAssemblyGenerator *gen, int src_reg, int dest_reg);
bool ffi_generate_slice_to_string(FFIAssemblyGenerator *gen, int src_reg, int dest_reg);

// Slice operations
bool ffi_generate_slice_index_access(FFIAssemblyGenerator *gen, int slice_reg, int index_reg, int result_reg, bool bounds_check);
bool ffi_generate_slice_subslice(FFIAssemblyGenerator *gen, int slice_reg, int start_reg, int end_reg, int result_reg);
bool ffi_generate_slice_iteration_setup(FFIAssemblyGenerator *gen, int slice_reg, int iter_reg, int end_reg);

// Security operations
bool ffi_generate_stack_canary_setup(FFIAssemblyGenerator *gen);
bool ffi_generate_stack_canary_check(FFIAssemblyGenerator *gen, const char *error_handler);
bool ffi_generate_pic_code(FFIAssemblyGenerator *gen, bool enable);

// Concurrency operations
bool ffi_generate_mutex_lock(FFIAssemblyGenerator *gen, int mutex_reg);
bool ffi_generate_mutex_unlock(FFIAssemblyGenerator *gen, int mutex_reg);
bool ffi_generate_atomic_cas(FFIAssemblyGenerator *gen, int target_reg, int expected_reg, int desired_reg, int result_reg);
bool ffi_generate_atomic_increment(FFIAssemblyGenerator *gen, int target_reg, int result_reg);
bool ffi_generate_tls_access(FFIAssemblyGenerator *gen, const char *var_name, int result_reg);
bool ffi_generate_memory_fence(FFIAssemblyGenerator *gen, int fence_type);

// Optimization operations
bool ffi_optimize_dead_code_elimination(FFIAssemblyGenerator *gen);
bool ffi_optimize_register_allocation(FFIAssemblyGenerator *gen);
bool ffi_optimize_instruction_scheduling(FFIAssemblyGenerator *gen);
bool ffi_optimize_loop(FFIAssemblyGenerator *gen, ASTNode *loop_block);
bool ffi_optimize_constant_folding(FFIAssemblyGenerator *gen, ASTNode *expr);
bool ffi_optimize_inline_function(FFIAssemblyGenerator *gen, ASTNode *call_expr);
bool ffi_optimize_array_access(FFIAssemblyGenerator *gen, ASTNode *array_access);
bool ffi_optimize_conditional_move(FFIAssemblyGenerator *gen, ASTNode *conditional, int result_reg);

// Assembly output and validation
bool ffi_print_gas_assembly(FFIAssemblyGenerator *gen, char *buffer, size_t buffer_size);
size_t ffi_get_code_size(FFIAssemblyGenerator *gen);
bool ffi_optimize_code_size(FFIAssemblyGenerator *gen);
bool ffi_enable_profiling(FFIAssemblyGenerator *gen, bool enable);

// Profiling data structure
typedef struct {
    size_t instruction_count;
    size_t register_spills;
} FFIProfilingData;

bool ffi_get_profiling_data(FFIAssemblyGenerator *gen, FFIProfilingData *data);

// Cross-platform support
bool ffi_set_calling_convention(FFIAssemblyGenerator *gen, int calling_convention);

// Fence types for memory operations
#define FENCE_ACQUIRE 1
#define FENCE_RELEASE 2
#define FENCE_SEQ_CST 3

// Test runner function
void run_test(test_function_t test_func);

// Test suite setup and teardown
bool setup_test_suite(void);
void teardown_test_suite(void);

#endif // ASTHRA_TEST_FFI_ASSEMBLY_COMMON_H 
