/**
 * Asthra Programming Language Compiler
 * Optimizer Configuration Structures
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Configuration structures and pass setup for optimization system.
 */

#ifndef ASTHRA_OPTIMIZER_CONFIG_H
#define ASTHRA_OPTIMIZER_CONFIG_H

#include "optimizer_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// OPTIMIZATION PASS CONFIGURATION
// =============================================================================

// Optimization pass configuration with C17 designated initializers
struct OptimizationPassConfig {
    OptimizationPassType type;
    bool enabled;
    uint32_t priority;  // Lower numbers run first
    uint32_t max_iterations;
    
    // Pass-specific configuration
    union {
        struct {
            bool remove_unreachable_code;
            bool remove_unused_variables;
            bool remove_empty_blocks;
        } dead_code;
        
        struct {
            bool fold_arithmetic;
            bool fold_comparisons;
            bool fold_logical;
            bool fold_bitwise;
        } constant_folding;
        
        struct {
            bool propagate_constants;
            bool propagate_copies;
            size_t max_propagation_distance;
        } propagation;
        
        struct {
            bool enable_global_cse;
            bool enable_local_cse;
            size_t hash_table_size;
        } cse;
    } config;
};

// =============================================================================
// CONFIGURATION FUNCTIONS
// =============================================================================

// Configuration
bool optimizer_set_level(Optimizer *optimizer, OptimizationLevel level);
bool optimizer_enable_pass(Optimizer *optimizer, OptimizationPassType pass);
bool optimizer_disable_pass(Optimizer *optimizer, OptimizationPassType pass);
bool optimizer_configure_pass(Optimizer *optimizer, OptimizationPassType pass, 
                             const OptimizationPassConfig *config);

// Type-safe optimization pass configuration
#define configure_optimization_pass(type, config) _Generic((type), \
    OptimizationPassType: configure_optimization_pass_impl, \
    default: configure_optimization_pass_impl \
)(type, config)

// Implementation function (not for direct use)
bool configure_optimization_pass_impl(OptimizationPassType type, const OptimizationPassConfig *config);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OPTIMIZER_CONFIG_H 
