/**
 * FFI Test Stubs - Types and Structures
 *
 * Contains type definitions and structures used across FFI test stubs
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_STUBS_TYPES_H
#define ASTHRA_FFI_STUBS_TYPES_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Include actual FFI infrastructure
#include "code_generator_core.h"
#include "ffi_assembly_generator.h"
#include "parser.h"
#include "semantic_analyzer.h"

// =============================================================================
// FFI GENERATOR TYPES
// =============================================================================

// Enhanced FFI Generator wrapper that uses the real infrastructure
typedef struct {
    FFIAssemblyGenerator *real_generator;
    CodeGenerator *base_generator;
    atomic_uint_fast32_t generation_count;
    atomic_uint_fast32_t successful_generations;
    atomic_uint_fast32_t failed_generations;
    bool initialized;
} EnhancedFFIGenerator;

// Legacy MinimalFFIGenerator typedef for backward compatibility
typedef EnhancedFFIGenerator MinimalFFIGenerator;

// Generation statistics
typedef struct {
    int total_generated;
    int successful_generations;
    int failed_generations;
} GenerationStatistics;

// =============================================================================
// FFI MEMORY MANAGEMENT TYPES
// =============================================================================

// Simple FFI memory manager (in-memory tracking)
typedef struct {
    atomic_size_t allocated_bytes;
    atomic_size_t freed_bytes;
    atomic_size_t current_usage;
    atomic_size_t peak_usage;
    atomic_uint_fast32_t allocations;
    atomic_uint_fast32_t frees;
    bool leak_detected;
} FFIMemoryManager;

// FFI memory statistics
typedef struct {
    size_t total_allocated_bytes;
    size_t total_freed_bytes;
    size_t current_memory_usage;
    size_t peak_memory_usage;
    uint32_t total_allocations;
    uint32_t total_frees;
    bool leak_detected;
} FFIMemoryStats;

// =============================================================================
// ENHANCED AST AND PARSER TYPES
// =============================================================================

// Enhanced AST stub that connects to real parser infrastructure
typedef struct EnhancedASTNode {
    ASTNodeType node_type;
    void *data;
    size_t data_size;
    struct EnhancedASTNode *left;
    struct EnhancedASTNode *right;
    struct EnhancedASTNode *parent;
    atomic_uint_fast32_t ref_count;
    bool is_managed;
} EnhancedASTNode;

// Enhanced Parser that uses real parser infrastructure (simplified for testing)
typedef struct EnhancedParser {
    bool initialized;
    atomic_uint_fast32_t parse_count;
    atomic_uint_fast32_t successful_parses;
    atomic_uint_fast32_t failed_parses;
    char last_error[256];
} EnhancedParser;

// =============================================================================
// FFI MARSHALING TYPES
// =============================================================================

// Enhanced FFI parameter marshaling using real FFI infrastructure
typedef struct {
    FFIAssemblyGenerator *ffi_generator;
    atomic_uint_fast32_t marshaling_operations;
    atomic_uint_fast32_t successful_marshaling;
    atomic_uint_fast32_t failed_marshaling;
    bool initialized;
} EnhancedFFIMarshaler;

// FFI marshaling statistics
typedef struct {
    uint32_t total_operations;
    uint32_t successful_operations;
    uint32_t failed_operations;
    double success_rate;
} FFIMarshalingStats;

// =============================================================================
// CODEGEN TYPES
// =============================================================================

// Enhanced CodeGen context that uses real code generator
typedef struct EnhancedCodegenContext {
    CodeGenerator *real_generator;
    FFIAssemblyGenerator *ffi_generator;
    bool initialized;
    atomic_uint_fast32_t instruction_count;
    atomic_uint_fast32_t function_calls_generated;
    atomic_uint_fast32_t returns_generated;
    atomic_uint_fast32_t failed_generations;
    char output_buffer[8192]; // Assembly output buffer
    size_t output_buffer_pos;
} EnhancedCodegenContext;

// Enhanced statistics
typedef struct {
    uint32_t total_instructions;
    uint32_t function_calls;
    uint32_t returns;
    uint32_t failed_operations;
    double success_rate;
} CodegenStats;

// Legacy compatibility typedef
typedef EnhancedCodegenContext CodegenContext;

#endif // ASTHRA_FFI_STUBS_TYPES_H