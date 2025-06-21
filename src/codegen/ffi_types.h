/**
 * Asthra Programming Language Compiler
 * FFI Types and Enums
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_TYPES_H
#define ASTHRA_FFI_TYPES_H

#include "../parser/ast.h"
#include "code_generator.h"
#include "../analysis/type_info.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Special register constant for no register
#define REGISTER_NONE ((Register)0xFFFFFFFF)

// FFI calling conventions
typedef enum {
    FFI_CONV_C,              // Standard C calling convention
    FFI_CONV_STDCALL,        // Windows stdcall
    FFI_CONV_FASTCALL,       // Fast calling convention
    FFI_CONV_VECTORCALL,     // Vector calling convention
    FFI_CONV_SYSV_AMD64,     // System V AMD64 ABI
    FFI_CONV_MS_X64,         // Microsoft x64 calling convention
    FFI_CONV_COUNT
} FFICallingConvention;

// FFI parameter marshaling types
typedef enum {
    FFI_MARSHAL_DIRECT,      // Direct value passing
    FFI_MARSHAL_POINTER,     // Pass by pointer
    FFI_MARSHAL_SLICE,       // Slice to pointer+length
    FFI_MARSHAL_STRING,      // String to C string
    FFI_MARSHAL_RESULT,      // Result type marshaling
    FFI_MARSHAL_VARIADIC,    // Variadic argument
    FFI_MARSHAL_COUNT
} FFIMarshalingType;

// Pattern matching strategies
typedef enum {
    PATTERN_STRATEGY_JUMP_TABLE,    // Efficient jump table for dense patterns
    PATTERN_STRATEGY_BINARY_SEARCH, // Binary search for sparse patterns
    PATTERN_STRATEGY_LINEAR,        // Linear comparison chain
    PATTERN_STRATEGY_COUNT
} PatternMatchStrategy;

// String operation types
typedef enum {
    STRING_OP_CONCATENATION,     // + operator
    STRING_OP_INTERPOLATION,     // {} syntax
    STRING_OP_LENGTH,            // .len access
    STRING_OP_SLICE,             // substring operation
    STRING_OP_COMPARISON,        // == != < > <= >=
    STRING_OP_COUNT
} StringOperationType;

// Slice operation types
typedef enum {
    SLICE_OP_CREATE,             // Create slice from array
    SLICE_OP_LENGTH_ACCESS,      // .len property access
    SLICE_OP_INDEX_ACCESS,       // [index] access
    SLICE_OP_SUBSLICE,          // [start:end] operation
    SLICE_OP_BOUNDS_CHECK,      // Runtime bounds checking
    SLICE_OP_TO_FFI,            // Convert to C pointer+length
    SLICE_OP_COUNT
} SliceOperationType;

// Security operation types
typedef enum {
    SECURITY_OP_CONSTANT_TIME,   // Constant-time operations
    SECURITY_OP_VOLATILE_MEMORY, // Volatile memory access
    SECURITY_OP_SECURE_ZERO,     // Secure memory zeroing
    SECURITY_OP_MEMORY_BARRIER,  // Memory barriers
    SECURITY_OP_COUNT
} SecurityOperationType;

// FFI ownership transfer types (renamed to avoid conflict with ast_types.h)
typedef enum {
    FFI_OWNERSHIP_NONE,       // No ownership transfer
    FFI_OWNERSHIP_COPY,       // Copy the data
    FFI_OWNERSHIP_MOVE,       // Move ownership
    FFI_OWNERSHIP_BORROW,     // Temporary borrow
    FFI_OWNERSHIP_COUNT
} FFIOwnershipTransferType;

// Concurrency operation types
typedef enum {
    // Tier 1 (Core & Simple) concurrency operations
    CONCURRENCY_OP_SPAWN,        // spawn statement
    CONCURRENCY_OP_SPAWN_WITH_HANDLE, // spawn_with_handle statement
    CONCURRENCY_OP_AWAIT,        // await expression
    // Note: Tier 3 operations moved to stdlib:
    // CONCURRENCY_OP_CHANNEL_SEND, CONCURRENCY_OP_CHANNEL_RECV,
    // CONCURRENCY_OP_SELECT, CONCURRENCY_OP_WORKER_POOL
    CONCURRENCY_OP_SYNC,         // synchronization primitive
    CONCURRENCY_OP_ATOMIC,       // atomic operation
    CONCURRENCY_OP_COUNT
} ConcurrencyOperationType;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_TYPES_H 
