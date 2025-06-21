/**
 * Asthra Programming Language Compiler
 * Optimizer Core Types and Forward Declarations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 3.2 Enhancements:
 * - C17 _Static_assert for type validation
 * - Atomic operations for thread-safe optimization statistics
 * - Forward declarations for modular header structure
 */

#ifndef ASTHRA_OPTIMIZER_TYPES_H
#define ASTHRA_OPTIMIZER_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// C17 static assertions for optimization assumptions
_Static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t must be at least 32-bit for optimization data structures");
_Static_assert(__STDC_VERSION__ >= 201710L, "C17 standard required for optimization modernization");
_Static_assert(sizeof(atomic_uint_fast64_t) >= sizeof(uint64_t), "Atomic types must support 64-bit counters");

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================

typedef struct Optimizer Optimizer;
typedef struct OptimizationPass OptimizationPass;
typedef struct DataFlowAnalysis DataFlowAnalysis;
typedef struct OptimizationStatistics OptimizationStatistics;
typedef struct BasicBlock BasicBlock;
typedef struct ControlFlowGraph ControlFlowGraph;
typedef struct BitVector BitVector;
typedef struct OptimizationPassConfig OptimizationPassConfig;

// External dependencies (from other modules)
typedef struct ASTNode ASTNode;
typedef struct AssemblyInstruction AssemblyInstruction;
typedef struct CodeGenerator CodeGenerator;
typedef struct SemanticAnalyzer SemanticAnalyzer;

// =============================================================================
// CORE ENUMERATIONS
// =============================================================================

// Optimization pass types
typedef enum {
    OPT_PASS_DEAD_CODE_ELIMINATION,
    OPT_PASS_CONSTANT_FOLDING,
    OPT_PASS_CONSTANT_PROPAGATION,
    OPT_PASS_COPY_PROPAGATION,
    OPT_PASS_COMMON_SUBEXPRESSION_ELIMINATION,
    OPT_PASS_LOOP_INVARIANT_CODE_MOTION,
    OPT_PASS_STRENGTH_REDUCTION,
    OPT_PASS_REGISTER_COALESCING,
    OPT_PASS_INSTRUCTION_SCHEDULING,
    OPT_PASS_PEEPHOLE_OPTIMIZATION,
    OPT_PASS_TAIL_CALL_OPTIMIZATION,
    OPT_PASS_INLINING,
    OPT_PASS_COUNT
} OptimizationPassType;

// C17 static assertion for optimization pass enum
_Static_assert(OPT_PASS_COUNT <= 32, "Optimization passes must fit in 32-bit bitmask");

// Optimization levels
typedef enum {
    OPT_LEVEL_NONE = 0,     // -O0: No optimization
    OPT_LEVEL_BASIC = 1,    // -O1: Basic optimizations
    OPT_LEVEL_STANDARD = 2, // -O2: Standard optimizations
    OPT_LEVEL_AGGRESSIVE = 3, // -O3: Aggressive optimizations
    OPT_LEVEL_SIZE = 4,     // -Os: Size optimizations
    OPT_LEVEL_COUNT
} OptimizationLevel;

// Data flow analysis types
typedef enum {
    DATAFLOW_REACHING_DEFINITIONS,
    DATAFLOW_LIVE_VARIABLES,
    DATAFLOW_AVAILABLE_EXPRESSIONS,
    DATAFLOW_VERY_BUSY_EXPRESSIONS,
    DATAFLOW_DOMINANCE,
    DATAFLOW_POST_DOMINANCE,
    DATAFLOW_COUNT
} DataFlowAnalysisType;

// Optimization error codes
typedef enum {
    OPT_ERROR_NONE = 0,
    OPT_ERROR_OUT_OF_MEMORY,
    OPT_ERROR_INVALID_CFG,
    OPT_ERROR_DATAFLOW_FAILED,
    OPT_ERROR_PASS_FAILED,
    OPT_ERROR_VERIFICATION_FAILED,
    OPT_ERROR_INFINITE_LOOP,
    OPT_ERROR_UNSUPPORTED_OPERATION,
    OPT_ERROR_COUNT
} OptimizationErrorCode;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OPTIMIZER_TYPES_H 
