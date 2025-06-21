/**
 * Asthra Programming Language Compiler
 * Never Type Optimization Support
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Optimization support for Never type, including dead code elimination
 * after Never-returning functions and branch prediction hints.
 */

#ifndef ASTHRA_OPTIMIZER_NEVER_H
#define ASTHRA_OPTIMIZER_NEVER_H

#include "optimizer_types.h"
#include "optimizer_cfg.h"
#include "../analysis/semantic_types_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// NEVER TYPE DETECTION
// =============================================================================

/**
 * Check if a function returns Never type
 * @param func_type The function type descriptor
 * @return true if the function returns Never, false otherwise
 */
bool optimizer_is_never_returning_function(const TypeDescriptor *func_type);

/**
 * Check if a basic block contains a call to a Never-returning function
 * @param block The basic block to check
 * @return true if block contains Never-returning call, false otherwise
 */
bool optimizer_block_has_never_call(const BasicBlock *block);

// =============================================================================
// NEVER TYPE OPTIMIZATIONS
// =============================================================================

/**
 * Mark blocks as unreachable after Never-returning function calls
 * @param cfg The control flow graph to analyze
 * @return Number of blocks marked as unreachable
 */
size_t optimizer_mark_unreachable_after_never(ControlFlowGraph *cfg);

/**
 * Remove edges from blocks that call Never-returning functions
 * @param cfg The control flow graph to optimize
 * @return Number of edges removed
 */
size_t optimizer_remove_never_successor_edges(ControlFlowGraph *cfg);

/**
 * Add branch prediction hints for branches to Never-returning functions
 * @param cfg The control flow graph to optimize
 * @return Number of branch hints added
 */
size_t optimizer_add_never_branch_hints(ControlFlowGraph *cfg);

/**
 * Apply all Never-type optimizations to a control flow graph
 * @param cfg The control flow graph to optimize
 * @param stats Statistics structure to update (optional)
 * @return true if optimizations were successful, false on error
 */
bool optimizer_apply_never_optimizations(ControlFlowGraph *cfg, OptimizationStatistics *stats);

// =============================================================================
// STATISTICS TRACKING
// =============================================================================

/**
 * Update optimization statistics for Never type optimizations
 * @param stats The statistics structure to update
 * @param never_optimizations Number of Never-specific optimizations
 * @param unreachable_blocks Number of unreachable blocks removed
 * @param branch_hints Number of branch hints added
 */
void optimizer_update_never_statistics(OptimizationStatistics *stats,
                                     size_t never_optimizations,
                                     size_t unreachable_blocks,
                                     size_t branch_hints);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OPTIMIZER_NEVER_H