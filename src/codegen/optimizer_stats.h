/**
 * Asthra Programming Language Compiler
 * Optimizer Statistics and Profiling
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Statistics collection and profiling for optimization passes.
 */

#ifndef ASTHRA_OPTIMIZER_STATS_H
#define ASTHRA_OPTIMIZER_STATS_H

#include "optimizer_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// OPTIMIZATION STATISTICS WITH ATOMIC OPERATIONS
// =============================================================================

struct OptimizationStatistics {
    // Pass execution statistics
    atomic_uint_fast64_t passes_executed;
    atomic_uint_fast64_t total_optimization_time_ns;
    atomic_uint_fast32_t iterations_performed;
    
    // Transformation statistics
    atomic_uint_fast32_t instructions_eliminated;
    atomic_uint_fast32_t constants_folded;
    atomic_uint_fast32_t copies_propagated;
    atomic_uint_fast32_t common_subexpressions_eliminated;
    atomic_uint_fast32_t dead_stores_eliminated;
    atomic_uint_fast32_t unreachable_blocks_removed;
    
    // Code size statistics
    atomic_uint_fast64_t original_instruction_count;
    atomic_uint_fast64_t optimized_instruction_count;
    atomic_uint_fast64_t code_size_reduction_bytes;
    
    // Performance statistics
    atomic_uint_fast32_t register_pressure_reduced;
    atomic_uint_fast32_t memory_accesses_eliminated;
    atomic_uint_fast32_t branches_eliminated;
    atomic_uint_fast32_t function_calls_inlined;
    
    // Loop optimization statistics
    atomic_uint_fast32_t loops_unrolled;
    atomic_uint_fast32_t loop_invariants_moved;
    atomic_uint_fast32_t strength_reductions;
    atomic_uint_fast32_t instructions_generated;  // From unrolling
    
    // Match expression optimization statistics
    atomic_uint_fast32_t match_jump_tables_created;
    atomic_uint_fast32_t match_binary_searches_created;
    atomic_uint_fast32_t match_patterns_optimized;
    
    // Never type optimization statistics
    atomic_uint_fast32_t never_optimizations;
    atomic_uint_fast32_t never_branch_hints_added;
    atomic_uint_fast32_t never_edges_removed;
    
    // Analysis statistics
    atomic_uint_fast32_t dataflow_analyses_performed;
    atomic_uint_fast64_t dataflow_analysis_time_ns;
    atomic_uint_fast32_t cfg_constructions;
};

// =============================================================================
// STATISTICS AND PROFILING
// =============================================================================

OptimizationStatistics optimizer_get_statistics(const Optimizer *optimizer);
void optimizer_reset_statistics(Optimizer *optimizer);
void optimizer_print_statistics(const Optimizer *optimizer);

// Performance profiling
void optimizer_start_profiling(Optimizer *optimizer);
void optimizer_stop_profiling(Optimizer *optimizer);
double optimizer_get_optimization_time(const Optimizer *optimizer);

// Pass-specific statistics
uint32_t optimizer_get_pass_execution_count(const Optimizer *optimizer, OptimizationPassType pass);
double optimizer_get_pass_execution_time(const Optimizer *optimizer, OptimizationPassType pass);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OPTIMIZER_STATS_H 
