/**
 * Asthra Programming Language Compiler
 * Optimization Pass Implementations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Declarations for all optimization passes including dead code elimination,
 * constant folding, loop optimizations, and more.
 */

#ifndef ASTHRA_OPTIMIZER_PASSES_H
#define ASTHRA_OPTIMIZER_PASSES_H

#include "optimizer_types.h"
#include "optimizer_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
struct FFIAssemblyGenerator;
struct PatternMatchContext;
struct LoopInfo;
struct CodeGenerator;

// =============================================================================
// OPTIMIZATION PASS IMPLEMENTATIONS
// =============================================================================

// Dead code elimination
bool opt_dead_code_elimination(Optimizer *optimizer, ControlFlowGraph *cfg);
bool opt_remove_unreachable_blocks(Optimizer *optimizer, ControlFlowGraph *cfg);
bool opt_remove_unused_variables(Optimizer *optimizer, ControlFlowGraph *cfg);

// Constant folding and propagation
bool opt_constant_folding(Optimizer *optimizer, ControlFlowGraph *cfg);
bool opt_constant_propagation(Optimizer *optimizer, ControlFlowGraph *cfg);
bool opt_copy_propagation(Optimizer *optimizer, ControlFlowGraph *cfg);

// Common subexpression elimination
bool opt_common_subexpression_elimination(Optimizer *optimizer, ControlFlowGraph *cfg);
bool opt_local_cse(Optimizer *optimizer, BasicBlock *block);
bool opt_global_cse(Optimizer *optimizer, ControlFlowGraph *cfg);

// Loop optimizations
bool opt_loop_invariant_code_motion(Optimizer *optimizer, ControlFlowGraph *cfg);
bool opt_strength_reduction(Optimizer *optimizer, ControlFlowGraph *cfg);
bool opt_loop_unrolling(Optimizer *optimizer, ControlFlowGraph *cfg);

// Advanced control flow optimizations
bool optimize_match_expression(Optimizer *optimizer, ASTNode *match_stmt,
                              struct FFIAssemblyGenerator *generator,
                              struct PatternMatchContext *context);
bool optimize_for_loop(Optimizer *optimizer, ASTNode *for_stmt, 
                      struct CodeGenerator *generator);
bool optimize_loop_unrolling(Optimizer *optimizer, ASTNode *for_stmt,
                            struct CodeGenerator *generator);
bool optimize_loop_invariant_code_motion(Optimizer *optimizer, ControlFlowGraph *cfg,
                                       struct LoopInfo *loop);
bool optimize_loop_strength_reduction(Optimizer *optimizer, ControlFlowGraph *cfg,
                                    struct LoopInfo *loop);

// Register and instruction optimizations
bool opt_register_coalescing(Optimizer *optimizer, ControlFlowGraph *cfg);
bool opt_instruction_scheduling(Optimizer *optimizer, ControlFlowGraph *cfg);
bool opt_peephole_optimization(Optimizer *optimizer, ControlFlowGraph *cfg);

// Function optimizations
bool opt_function_inlining(Optimizer *optimizer, ASTNode *program);
bool opt_tail_call_optimization(Optimizer *optimizer, ControlFlowGraph *cfg);

// =============================================================================
// C17 GENERIC MACROS FOR TYPE-SAFE OPTIMIZATION OPERATIONS
// =============================================================================

// Type-safe optimization dispatch using C17 _Generic
#define optimize_instruction(optimizer, instruction) _Generic((instruction), \
    AssemblyInstruction*: optimize_instruction_impl, \
    const AssemblyInstruction*: optimize_instruction_const_impl \
)(optimizer, instruction)

#define optimize_basic_block(optimizer, block) _Generic((block), \
    BasicBlock*: optimize_basic_block_impl, \
    const BasicBlock*: optimize_basic_block_const_impl \
)(optimizer, block)

// =============================================================================
// IMPLEMENTATION FUNCTIONS FOR _Generic MACROS
// =============================================================================

// Implementation functions (not for direct use)
bool optimize_instruction_impl(Optimizer *optimizer, AssemblyInstruction *instruction);
bool optimize_instruction_const_impl(Optimizer *optimizer, const AssemblyInstruction *instruction);

bool optimize_basic_block_impl(Optimizer *optimizer, BasicBlock *block);
bool optimize_basic_block_const_impl(Optimizer *optimizer, const BasicBlock *block);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OPTIMIZER_PASSES_H 
