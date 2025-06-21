/**
 * Asthra Programming Language Compiler
 * Optimizer Core Interface and Main Structure
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Main optimizer interface and core functionality.
 */

#ifndef ASTHRA_OPTIMIZER_CORE_H
#define ASTHRA_OPTIMIZER_CORE_H

#include "optimizer_types.h"
#include "optimizer_config.h"
#include "optimizer_cfg.h"
#include "optimizer_dataflow.h"
#include "optimizer_stats.h"

// External dependencies
#include "code_generator.h"
#include "../analysis/semantic_analyzer.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MAIN OPTIMIZER STRUCTURE
// =============================================================================

struct Optimizer {
    // Target configuration
    OptimizationLevel level;
    uint32_t enabled_passes;  // Bitmask of enabled passes
    bool debug_mode;
    bool verify_optimizations;
    
    // Core components
    CodeGenerator *code_generator;
    SemanticAnalyzer *semantic_analyzer;
    ControlFlowGraph *cfg;
    
    // Optimization passes
    OptimizationPassConfig *pass_configs;
    size_t pass_count;
    
    // Data flow analyses
    DataFlowAnalysis **analyses;
    size_t analysis_count;
    
    // Statistics
    OptimizationStatistics stats;
    
    // Configuration
    struct {
        size_t max_iterations;
        size_t max_inlining_depth;
        size_t max_unroll_factor;
        bool preserve_debug_info;
        bool aggressive_dead_code_elimination;
        bool enable_interprocedural_optimization;
    } config;
};

// =============================================================================
// OPTIMIZER INTERFACE
// =============================================================================

// Creation and destruction
Optimizer *optimizer_create(OptimizationLevel level);
void optimizer_destroy(Optimizer *optimizer);
void optimizer_reset(Optimizer *optimizer);

// Main optimization entry points
bool optimizer_optimize_program(Optimizer *optimizer, ASTNode *program);
bool optimizer_optimize_function(Optimizer *optimizer, ASTNode *function);
bool optimizer_optimize_instructions(Optimizer *optimizer, 
                                    AssemblyInstruction **instructions, 
                                    size_t instruction_count);

// =============================================================================
// OPTIMIZATION VERIFICATION AND DEBUGGING
// =============================================================================

// Verification
bool optimizer_verify_correctness(Optimizer *optimizer, ControlFlowGraph *original, 
                                 ControlFlowGraph *optimized);
void optimizer_dump_optimization_log(const Optimizer *optimizer, const char *filename);

// =============================================================================
// ERROR HANDLING
// =============================================================================

// Error reporting
void optimizer_report_error(Optimizer *optimizer, OptimizationErrorCode code, 
                           const char *format, ...);
const char *optimizer_error_name(OptimizationErrorCode code);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OPTIMIZER_CORE_H 
