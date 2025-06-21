/**
 * Asthra Programming Language Compiler
 * FFI Context Structures
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_CONTEXTS_H
#define ASTHRA_FFI_CONTEXTS_H

#include "ffi_types.h"
#include "../parser/ast.h"
#include "code_generator.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct FFICallContext FFICallContext;
typedef struct PatternMatchContext PatternMatchContext;
typedef struct StringOperationContext StringOperationContext;
typedef struct SliceOperationContext SliceOperationContext;
typedef struct SecurityContext SecurityContext;
typedef struct ConcurrencyContext ConcurrencyContext;

// FFI call context for managing complex calls
struct FFICallContext {
    char *function_name;
    char *extern_library;
    FFICallingConvention calling_conv;
    
    // Parameter information
    struct {
        FFIMarshalingType marshal_type;
        FFIOwnershipTransferType transfer_type;
        Register allocated_reg;
        size_t stack_offset;
        bool is_variadic;
    } *parameters;
    size_t parameter_count;
    
    // Return value handling
    FFIMarshalingType return_marshal_type;
    FFIOwnershipTransferType return_transfer_type;
    Register return_reg;
    
    // Stack management
    size_t total_stack_size;
    size_t alignment_padding;
    
    // Cleanup tracking
    bool needs_cleanup;
    char **cleanup_labels;
    size_t cleanup_count;
};

// Pattern match context
struct PatternMatchContext {
    PatternMatchStrategy strategy;
    
    // Match expression information
    Register match_value_reg;
    struct TypeInfo *match_type;
    
    // Pattern arms
    struct {
        char *pattern_label;
        char *body_label;
        char *end_label;
        bool is_default;
        
        // For Result<T,E> patterns
        bool is_result_ok;
        bool is_result_err;
        char *binding_name;
        Register binding_reg;
    } *arms;
    size_t arm_count;
    
    // Jump table (if applicable)
    char **jump_table_labels;
    size_t jump_table_size;
    
    // Optimization flags
    bool is_exhaustive;
    bool has_default_arm;
    bool can_use_jump_table;
};

// String operation context
struct StringOperationContext {
    StringOperationType operation;
    
    // Operand registers
    Register left_reg;
    Register right_reg;
    Register result_reg;
    
    // For interpolation
    struct {
        char *template_string;
        Register *expression_regs;
        size_t expression_count;
    } interpolation;
    
    // Memory management
    bool needs_allocation;
    bool transfer_ownership;
    char *cleanup_label;
    
    // Optimization flags
    bool is_constant_time;
    bool bounds_checked;
};

// Slice operation context
struct SliceOperationContext {
    SliceOperationType operation;
    
    // Slice information
    Register slice_ptr_reg;      // Pointer to slice data
    Register slice_len_reg;      // Length register
    Register slice_cap_reg;      // Capacity register (for mutable slices)
    
    // Operation-specific registers
    Register index_reg;          // For index access
    Register start_reg;          // For subslicing
    Register end_reg;            // For subslicing
    Register result_reg;         // Result register
    
    // Type information
    size_t element_size;
    bool is_mutable;
    bool bounds_checking_enabled;
    
    // FFI conversion
    Register ffi_ptr_reg;        // C pointer for FFI
    Register ffi_len_reg;        // C length for FFI
    
    // Error handling
    char *bounds_error_label;
    char *success_label;
};

// Security context for secure operations
struct SecurityContext {
    SecurityOperationType operation;
    
    // Memory regions
    Register memory_reg;
    size_t memory_size;
    
    // Constant-time flags
    bool avoid_branches;
    bool avoid_memory_access_patterns;
    bool use_cmov_instructions;
    
    // Volatile memory flags
    bool prevent_optimization;
    bool force_memory_access;
    
    // Cleanup requirements
    bool secure_cleanup_required;
    char *cleanup_label;
};

// Tier 1 concurrency context (Core & Simple)
struct ConcurrencyContext {
    ConcurrencyOperationType operation;
    
    // Function information for spawn operations
    char *function_name;
    Register *argument_regs;
    size_t argument_count;
    
    // Task handle management
    Register task_handle_reg;
    char *handle_var_name;
    bool needs_handle_storage;
    
    // Note: Tier 3 fields moved to stdlib:
    // Channel operations, Select operation context, Worker pool context,
    // Channel/Select function pointers
    
    // Runtime scheduler interface (Tier 1)
    char *scheduler_spawn_function;
    char *scheduler_await_function;
    
    // Synchronization
    bool needs_memory_barrier;
    bool needs_atomic_operations;
};

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_CONTEXTS_H 
