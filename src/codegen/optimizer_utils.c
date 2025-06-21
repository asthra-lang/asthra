/**
 * Asthra Programming Language Compiler
 * Optimizer Utility Functions and Error Handling
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Utility functions and error handling for optimization.
 */

#include "optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

bool configure_optimization_pass_impl(OptimizationPassType type, const OptimizationPassConfig *config) {
    if (type >= OPT_PASS_COUNT || !config) return false;
    
    // TODO: Implement pass configuration
    // This would configure specific optimization passes
    
    return true;
}

bool optimizer_configure_pass(Optimizer *optimizer, OptimizationPassType pass, 
                             const OptimizationPassConfig *config) {
    if (!optimizer || pass >= OPT_PASS_COUNT || !config) return false;
    
    if (pass < optimizer->pass_count) {
        optimizer->pass_configs[pass] = *config;
        return true;
    }
    
    return false;
}

// =============================================================================
// ERROR HANDLING
// =============================================================================

void optimizer_report_error(Optimizer *optimizer, OptimizationErrorCode code, 
                           const char *format, ...) {
    if (!optimizer || !format) return;
    
    fprintf(stderr, "Optimization error (%d): ", code);
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n");
}

const char *optimizer_error_name(OptimizationErrorCode code) {
    switch (code) {
        case OPT_ERROR_NONE: return "NONE";
        case OPT_ERROR_OUT_OF_MEMORY: return "OUT_OF_MEMORY";
        case OPT_ERROR_INVALID_CFG: return "INVALID_CFG";
        case OPT_ERROR_DATAFLOW_FAILED: return "DATAFLOW_FAILED";
        case OPT_ERROR_PASS_FAILED: return "PASS_FAILED";
        case OPT_ERROR_VERIFICATION_FAILED: return "VERIFICATION_FAILED";
        case OPT_ERROR_INFINITE_LOOP: return "INFINITE_LOOP";
        case OPT_ERROR_UNSUPPORTED_OPERATION: return "UNSUPPORTED_OPERATION";
        default: return "UNKNOWN";
    }
} 
