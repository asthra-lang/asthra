/**
 * Asthra Programming Language Compiler
 * Code Generation Types and Basic Definitions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CODE_GENERATOR_TYPES_H
#define ASTHRA_CODE_GENERATOR_TYPES_H

#include "../parser/ast.h"
#include "../analysis/semantic_analyzer.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// C17 static assertions for code generation assumptions
_Static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t must be at least 32-bit for instruction buffers");
_Static_assert(__STDC_VERSION__ >= 201710L, "C17 standard required for code generation modernization");
_Static_assert(sizeof(void*) == 8, "System V AMD64 ABI requires 64-bit pointers");

// Forward declarations
typedef struct CodeGenerator CodeGenerator;
typedef struct InstructionBuffer InstructionBuffer;
typedef struct AssemblyInstruction AssemblyInstruction;
typedef struct CodeGenStatistics CodeGenStatistics;
typedef struct RegisterAllocator RegisterAllocator;
typedef struct LabelManager LabelManager;

// =============================================================================
// TARGET ARCHITECTURE AND ABI DEFINITIONS
// =============================================================================

// Target architecture
typedef enum {
    TARGET_ARCH_X86_64,
    TARGET_ARCH_AARCH64,
    TARGET_ARCH_WASM32,
    TARGET_ARCH_COUNT
} TargetArchitecture;

// Calling conventions
typedef enum {
    CALLING_CONV_SYSTEM_V_AMD64,
    CALLING_CONV_MS_X64,
    CALLING_CONV_AARCH64_AAPCS,
    CALLING_CONV_WASM_C,
    CALLING_CONV_COUNT
} CallingConvention;

// C17 static assertions for ABI assumptions
_Static_assert(TARGET_ARCH_COUNT <= 8, "Target architectures must fit in 3 bits");
_Static_assert(CALLING_CONV_COUNT <= 8, "Calling conventions must fit in 3 bits");

// System V AMD64 ABI register definitions
typedef enum {
    // Integer registers
    REG_RAX = 0, REG_RCX, REG_RDX, REG_RBX, REG_RSP, REG_RBP, REG_RSI, REG_RDI,
    REG_R8, REG_R9, REG_R10, REG_R11, REG_R12, REG_R13, REG_R14, REG_R15,
    
    // XMM registers for floating point
    REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM4, REG_XMM5, REG_XMM6, REG_XMM7,
    REG_XMM8, REG_XMM9, REG_XMM10, REG_XMM11, REG_XMM12, REG_XMM13, REG_XMM14, REG_XMM15,
    
    REG_COUNT,
    REG_NONE = -1,
    REG_IMMEDIATE = -2  // Special value for immediate operands
} Register;

// C17 static assertion for register enum
_Static_assert(REG_COUNT <= 64, "Registers must fit in 64-bit bitmask for allocation tracking");

// System V AMD64 parameter passing registers
static const Register SYSV_INT_PARAM_REGS[] = {REG_RDI, REG_RSI, REG_RDX, REG_RCX, REG_R8, REG_R9};
static const Register SYSV_FLOAT_PARAM_REGS[] = {REG_XMM0, REG_XMM1, REG_XMM2, REG_XMM3, REG_XMM4, REG_XMM5, REG_XMM6, REG_XMM7};

#define SYSV_INT_PARAM_COUNT (sizeof(SYSV_INT_PARAM_REGS) / sizeof(Register))
#define SYSV_FLOAT_PARAM_COUNT (sizeof(SYSV_FLOAT_PARAM_REGS) / sizeof(Register))

// =============================================================================
// ERROR HANDLING
// =============================================================================

// Code generation error codes
typedef enum {
    CODEGEN_ERROR_NONE = 0,
    CODEGEN_ERROR_OUT_OF_MEMORY,
    CODEGEN_ERROR_INVALID_INSTRUCTION,
    CODEGEN_ERROR_REGISTER_ALLOCATION_FAILED,
    CODEGEN_ERROR_LABEL_NOT_FOUND,
    CODEGEN_ERROR_UNSUPPORTED_OPERATION,
    CODEGEN_ERROR_ABI_VIOLATION,
    CODEGEN_ERROR_STACK_OVERFLOW,
    CODEGEN_ERROR_COUNT
} CodeGenErrorCode;

// =============================================================================
// CODE GENERATION STATISTICS WITH ATOMIC OPERATIONS
// =============================================================================

struct CodeGenStatistics {
    // Instruction statistics
    atomic_uint_fast64_t instructions_generated;
    atomic_uint_fast64_t basic_blocks_generated;
    atomic_uint_fast64_t functions_generated;
    
    // Memory statistics
    atomic_uint_fast64_t bytes_generated;
    atomic_uint_fast64_t stack_bytes_allocated;
    atomic_uint_fast64_t heap_allocations;
    
    // Optimization statistics
    atomic_uint_fast32_t optimizations_applied;
    atomic_uint_fast32_t dead_code_eliminated;
    atomic_uint_fast32_t constant_folded;
    
    // Register allocation statistics
    atomic_uint_fast32_t registers_allocated;
    atomic_uint_fast32_t spills_generated;
    atomic_uint_fast32_t max_register_pressure;
    
    // Performance metrics
    atomic_uint_fast64_t generation_time_ns;
    atomic_uint_fast32_t cache_hits;
    atomic_uint_fast32_t cache_misses;
};

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CODE_GENERATOR_TYPES_H 
