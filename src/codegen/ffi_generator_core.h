/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator Core
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_GENERATOR_CORE_H
#define ASTHRA_FFI_GENERATOR_CORE_H

#include "ffi_types.h"
#include "ffi_contexts.h"
#include "../parser/ast.h"
#include "../analysis/semantic_analyzer.h"
#include "code_generator.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// C17 static assertions for FFI assembly generation
_Static_assert(sizeof(void*) == 8, "System V AMD64 ABI requires 64-bit pointers");
_Static_assert(__STDC_VERSION__ >= 201710L, "C17 standard required for FFI assembly generation");

// Forward declaration
typedef struct FFIAssemblyGenerator FFIAssemblyGenerator;

// =============================================================================
// ENHANCED FFI ASSEMBLY GENERATOR
// =============================================================================

struct FFIAssemblyGenerator {
    // Base code generator
    CodeGenerator *base_generator;
    
    // FFI-specific contexts
    FFICallContext *current_ffi_call;
    PatternMatchContext *current_pattern_match;
    StringOperationContext *current_string_op;
    SliceOperationContext *current_slice_op;
    SecurityContext *current_security_op;
    ConcurrencyContext *current_concurrency_op;
    
    // Enhanced configuration
    struct {
        bool enable_bounds_checking;
        bool enable_security_features;
        bool enable_concurrency;
        bool optimize_string_operations;
        bool optimize_pattern_matching;
        size_t max_variadic_args;
        bool pic_mode;  // Position Independent Code
    } config;
    
    // Runtime function names
    struct {
        char *gc_alloc;
        char *gc_free;
        char *slice_bounds_check;
        char *string_concat;
        char *string_interpolate;
        char *result_create_ok;
        char *result_create_err;
        char *spawn_task;
        char *secure_zero;
    } runtime_functions;
    
    // Statistics
    atomic_uint_fast32_t ffi_calls_generated;
    atomic_uint_fast32_t pattern_matches_generated;
    atomic_uint_fast32_t string_operations_generated;
    atomic_uint_fast32_t slice_operations_generated;
    atomic_uint_fast32_t security_operations_generated;
    atomic_uint_fast32_t spawn_statements_generated;
};

// =============================================================================
// CORE FFI ASSEMBLY GENERATOR FUNCTIONS
// =============================================================================

/**
 * Create FFI assembly generator
 */
FFIAssemblyGenerator *ffi_assembly_generator_create(TargetArchitecture arch, 
                                                   CallingConvention conv);

/**
 * Destroy FFI assembly generator
 */
void ffi_assembly_generator_destroy(FFIAssemblyGenerator *generator);

/**
 * Generate assembly for complete program with Asthra features
 */
bool ffi_generate_program(FFIAssemblyGenerator *generator, ASTNode *program);

// =============================================================================
// ENHANCED EXPRESSION GENERATION
// =============================================================================

/**
 * Generate postfix expressions with disambiguation
 */
bool ffi_generate_postfix_expression(FFIAssemblyGenerator *generator,
                                    ASTNode *postfix_expr, Register result_reg);

/**
 * Generate Result<T,E> construction
 */
bool ffi_generate_result_construction(FFIAssemblyGenerator *generator,
                                    bool is_ok, Register value_reg,
                                    struct TypeInfo *result_type, Register result_reg);

// =============================================================================
// UNSAFE BLOCK GENERATION
// =============================================================================

/**
 * Generate unsafe block with GC interaction management
 */
bool ffi_generate_unsafe_block(FFIAssemblyGenerator *generator, ASTNode *unsafe_block);

/**
 * Generate GC interaction barriers
 */
bool ffi_generate_gc_barriers(FFIAssemblyGenerator *generator,
                             bool entering_unsafe, bool exiting_unsafe);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_GENERATOR_CORE_H 
