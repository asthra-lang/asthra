/**
 * Asthra Programming Language Compiler
 * Optimizer Core Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core optimizer functionality including creation, destruction, and configuration.
 */

#include "optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <limits.h>

// C17 static assertions for implementation assumptions
_Static_assert(sizeof(atomic_uint_fast64_t) >= sizeof(uint64_t), "Atomic types must be at least 64-bit");
_Static_assert(sizeof(pthread_mutex_t) > 0, "pthread_mutex_t must be available");

// =============================================================================
// INTERNAL HELPER FUNCTIONS AND CONSTANTS
// =============================================================================

// Default optimization pass configurations with C17 designated initializers
static const OptimizationPassConfig DEFAULT_PASS_CONFIGS[] = {
    [OPT_PASS_DEAD_CODE_ELIMINATION] = {
        .type = OPT_PASS_DEAD_CODE_ELIMINATION,
        .enabled = true,
        .priority = 1,
        .max_iterations = 10,
        .config.dead_code = {
            .remove_unreachable_code = true,
            .remove_unused_variables = true,
            .remove_empty_blocks = true
        }
    },
    [OPT_PASS_CONSTANT_FOLDING] = {
        .type = OPT_PASS_CONSTANT_FOLDING,
        .enabled = true,
        .priority = 2,
        .max_iterations = 5,
        .config.constant_folding = {
            .fold_arithmetic = true,
            .fold_comparisons = true,
            .fold_logical = true,
            .fold_bitwise = true
        }
    },
    [OPT_PASS_CONSTANT_PROPAGATION] = {
        .type = OPT_PASS_CONSTANT_PROPAGATION,
        .enabled = true,
        .priority = 3,
        .max_iterations = 8,
        .config.propagation = {
            .propagate_constants = true,
            .propagate_copies = true,
            .max_propagation_distance = 100
        }
    }
};

// Optimization level configurations
static const struct {
    uint32_t enabled_passes;
    size_t max_iterations;
    bool aggressive_mode;
} OPTIMIZATION_LEVEL_CONFIGS[] = {
    [OPT_LEVEL_NONE] = {
        .enabled_passes = 0,
        .max_iterations = 0,
        .aggressive_mode = false
    },
    [OPT_LEVEL_BASIC] = {
        .enabled_passes = (1U << OPT_PASS_DEAD_CODE_ELIMINATION) | 
                         (1U << OPT_PASS_CONSTANT_FOLDING),
        .max_iterations = 3,
        .aggressive_mode = false
    },
    [OPT_LEVEL_STANDARD] = {
        .enabled_passes = (1U << OPT_PASS_DEAD_CODE_ELIMINATION) |
                         (1U << OPT_PASS_CONSTANT_FOLDING) |
                         (1U << OPT_PASS_CONSTANT_PROPAGATION) |
                         (1U << OPT_PASS_COPY_PROPAGATION) |
                         (1U << OPT_PASS_COMMON_SUBEXPRESSION_ELIMINATION),
        .max_iterations = 10,
        .aggressive_mode = false
    },
    [OPT_LEVEL_AGGRESSIVE] = {
        .enabled_passes = 0xFFFFFFFF,  // All passes enabled
        .max_iterations = 20,
        .aggressive_mode = true
    }
};

// =============================================================================
// OPTIMIZER CREATION AND DESTRUCTION
// =============================================================================

Optimizer *optimizer_create(OptimizationLevel level) {
    Optimizer *optimizer = calloc(1, sizeof(Optimizer));
    if (!optimizer) {
        return NULL;
    }
    
    // C17 designated initializer for configuration
    *optimizer = (Optimizer) {
        .level = level,
        .enabled_passes = OPTIMIZATION_LEVEL_CONFIGS[level].enabled_passes,
        .debug_mode = false,
        .verify_optimizations = true,
        .cfg = NULL,
        .pass_configs = NULL,
        .pass_count = OPT_PASS_COUNT,
        .analyses = NULL,
        .analysis_count = 0,
        .config = {
            .max_iterations = OPTIMIZATION_LEVEL_CONFIGS[level].max_iterations,
            .max_inlining_depth = 5,
            .max_unroll_factor = 4,
            .preserve_debug_info = true,
            .aggressive_dead_code_elimination = OPTIMIZATION_LEVEL_CONFIGS[level].aggressive_mode,
            .enable_interprocedural_optimization = false
        }
    };
    
    // Initialize pass configurations
    optimizer->pass_configs = malloc(sizeof(OptimizationPassConfig) * OPT_PASS_COUNT);
    if (!optimizer->pass_configs) {
        free(optimizer);
        return NULL;
    }
    
    // Initialize all pass configurations to zero first
    memset(optimizer->pass_configs, 0, sizeof(OptimizationPassConfig) * OPT_PASS_COUNT);
    
    // Copy default configurations for the passes that are defined
    size_t num_default_configs = sizeof(DEFAULT_PASS_CONFIGS) / sizeof(DEFAULT_PASS_CONFIGS[0]);
    for (size_t i = 0; i < num_default_configs; i++) {
        if (DEFAULT_PASS_CONFIGS[i].type < OPT_PASS_COUNT) {
            optimizer->pass_configs[DEFAULT_PASS_CONFIGS[i].type] = DEFAULT_PASS_CONFIGS[i];
        }
    }
    
    // Initialize atomic statistics
    atomic_store(&optimizer->stats.passes_executed, 0);
    atomic_store(&optimizer->stats.total_optimization_time_ns, 0);
    atomic_store(&optimizer->stats.iterations_performed, 0);
    atomic_store(&optimizer->stats.instructions_eliminated, 0);
    atomic_store(&optimizer->stats.constants_folded, 0);
    atomic_store(&optimizer->stats.copies_propagated, 0);
    
    return optimizer;
}

void optimizer_destroy(Optimizer *optimizer) {
    if (!optimizer) return;
    
    // Destroy CFG if owned
    if (optimizer->cfg) {
        cfg_destroy(optimizer->cfg);
    }
    
    // Destroy data flow analyses
    for (size_t i = 0; i < optimizer->analysis_count; i++) {
        if (optimizer->analyses[i]) {
            dataflow_release(optimizer->analyses[i]);
        }
    }
    free(optimizer->analyses);
    
    free(optimizer->pass_configs);
    free(optimizer);
}

void optimizer_reset(Optimizer *optimizer) {
    if (!optimizer) return;
    
    // Reset statistics using atomic operations with memory_order_relaxed
    atomic_store_explicit(&optimizer->stats.passes_executed, 0, memory_order_relaxed);
    atomic_store_explicit(&optimizer->stats.total_optimization_time_ns, 0, memory_order_relaxed);
    atomic_store_explicit(&optimizer->stats.iterations_performed, 0, memory_order_relaxed);
    atomic_store_explicit(&optimizer->stats.instructions_eliminated, 0, memory_order_relaxed);
    atomic_store_explicit(&optimizer->stats.constants_folded, 0, memory_order_relaxed);
    atomic_store_explicit(&optimizer->stats.copies_propagated, 0, memory_order_relaxed);
    
    // Reset analyses
    for (size_t i = 0; i < optimizer->analysis_count; i++) {
        if (optimizer->analyses[i]) {
            dataflow_release(optimizer->analyses[i]);
        }
    }
    optimizer->analysis_count = 0;
}

// =============================================================================
// MAIN OPTIMIZATION FUNCTIONS
// =============================================================================

bool optimizer_optimize_program(Optimizer *optimizer, ASTNode *program) {
    if (!optimizer || !program) return false;
    
    // TODO: Implement program-level optimization
    // This would involve interprocedural analysis and optimization
    
    (void)program;  // Suppress unused parameter warning
    return true;
}

bool optimizer_optimize_function(Optimizer *optimizer, ASTNode *function) {
    if (!optimizer || !function) return false;
    
    // TODO: Implement function-level optimization
    // This would build a CFG from the function and apply optimizations
    
    (void)function;  // Suppress unused parameter warning
    return true;
}

bool optimizer_optimize_instructions(Optimizer *optimizer, 
                                    AssemblyInstruction **instructions, 
                                    size_t instruction_count) {
    if (!optimizer || !instructions) return false;
    
    atomic_store(&optimizer->stats.original_instruction_count, instruction_count);
    
    bool changed = false;
    
    // Apply instruction-level optimizations
    for (size_t i = 0; i < instruction_count; i++) {
        if (optimize_instruction(optimizer, instructions[i])) {
            changed = true;
        }
    }
    
    // Count remaining instructions after optimization
    size_t optimized_count = 0;
    for (size_t i = 0; i < instruction_count; i++) {
        if (instructions[i] && instructions[i]->type != INST_COUNT) {
            optimized_count++;
        }
    }
    
    atomic_store(&optimizer->stats.optimized_instruction_count, optimized_count);
    
    return changed;
}

// =============================================================================
// CONFIGURATION FUNCTIONS
// =============================================================================

bool optimizer_set_level(Optimizer *optimizer, OptimizationLevel level) {
    if (!optimizer || level >= OPT_LEVEL_COUNT) return false;
    
    optimizer->level = level;
    optimizer->enabled_passes = OPTIMIZATION_LEVEL_CONFIGS[level].enabled_passes;
    optimizer->config.max_iterations = OPTIMIZATION_LEVEL_CONFIGS[level].max_iterations;
    optimizer->config.aggressive_dead_code_elimination = OPTIMIZATION_LEVEL_CONFIGS[level].aggressive_mode;
    
    return true;
}

bool optimizer_enable_pass(Optimizer *optimizer, OptimizationPassType pass) {
    if (!optimizer || pass >= OPT_PASS_COUNT) return false;
    
    optimizer->enabled_passes |= (1U << pass);
    if (pass < optimizer->pass_count) {
        optimizer->pass_configs[pass].enabled = true;
    }
    
    return true;
}

bool optimizer_disable_pass(Optimizer *optimizer, OptimizationPassType pass) {
    if (!optimizer || pass >= OPT_PASS_COUNT) return false;
    
    optimizer->enabled_passes &= ~(1U << pass);
    if (pass < optimizer->pass_count) {
        optimizer->pass_configs[pass].enabled = false;
    }
    
    return true;
} 
