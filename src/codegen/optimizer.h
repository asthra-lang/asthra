/**
 * Asthra Programming Language Compiler
 * Main Optimizer Header - Modular Split Version
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 3.2 Enhancements:
 * - Modular header split for better maintainability
 * - Backward compatibility with existing code
 * - Clear separation of concerns between optimization components
 * 
 * This header provides the complete optimizer interface by including
 * all modular optimizer headers. Code can include this single header
 * to access all optimizer functionality.
 */

#ifndef ASTHRA_OPTIMIZER_H
#define ASTHRA_OPTIMIZER_H

// Core types and forward declarations
#include "optimizer_types.h"

// Configuration and pass management
#include "optimizer_config.h"

// Bit vector operations for data flow analysis
#include "optimizer_bitvector.h"

// Control flow graph and basic blocks
#include "optimizer_cfg.h"

// Data flow analysis framework
#include "optimizer_dataflow.h"

// Optimization pass implementations
#include "optimizer_passes.h"

// Statistics and profiling
#include "optimizer_stats.h"

// Main optimizer core interface
#include "optimizer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BACKWARD COMPATIBILITY NOTES
// =============================================================================

/*
 * This modular split maintains 100% backward compatibility with existing code.
 * All types, functions, and macros that were previously available in the
 * monolithic optimizer.h are still available through this header.
 * 
 * Benefits of the split:
 * - Faster compilation times due to smaller header dependencies
 * - Better maintainability with logical separation of concerns
 * - Easier testing of individual optimizer components
 * - Cleaner dependency management
 * - Reduced coupling between different optimization subsystems
 * 
 * The split structure:
 * - optimizer_types.h: Core types, enums, forward declarations
 * - optimizer_config.h: Pass configuration and optimization levels
 * - optimizer_bitvector.h: Bit vector operations for data flow analysis
 * - optimizer_cfg.h: Control flow graph and basic block structures
 * - optimizer_dataflow.h: Data flow analysis framework
 * - optimizer_passes.h: All optimization pass declarations
 * - optimizer_stats.h: Statistics and profiling functionality
 * - optimizer_core.h: Main optimizer interface and structure
 */

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OPTIMIZER_H 
