/**
 * Asthra Programming Language Compiler
 * Never Type Optimization Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "optimizer_never.h"
#include "optimizer_stats.h"
#include "code_generator_instructions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// =============================================================================
// NEVER TYPE DETECTION
// =============================================================================

bool optimizer_is_never_returning_function(const TypeDescriptor *func_type) {
    if (!func_type || func_type->category != TYPE_FUNCTION) {
        return false;
    }
    
    // Check if return type is Never
    const TypeDescriptor *return_type = func_type->data.function.return_type;
    if (!return_type) {
        return false;
    }
    
    return (return_type->category == TYPE_PRIMITIVE && 
            return_type->data.primitive.primitive_kind == PRIMITIVE_NEVER);
}

bool optimizer_block_has_never_call(const BasicBlock *block) {
    if (!block || !block->instructions) {
        return false;
    }
    
    // Look for CALL instructions in the block
    for (size_t i = 0; i < block->instruction_count; i++) {
        AssemblyInstruction *inst = block->instructions[i];
        if (!inst) continue;
        
        if (inst->type == INST_CALL) {
            if (inst->operand_count > 0 && 
                inst->operands[0].type == OPERAND_LABEL &&
                inst->operands[0].data.label) {
                
                const char *function_name = inst->operands[0].data.label;
                
                // Check for known Never-returning functions
                if (strstr(function_name, "panic") != NULL ||
                    strstr(function_name, "abort") != NULL ||
                    strstr(function_name, "exit") != NULL ||
                    strstr(function_name, "terminate") != NULL ||
                    strstr(function_name, "unreachable") != NULL ||
                    // Check for Asthra-specific Never functions
                    strstr(function_name, "_never_") != NULL ||
                    // Function names ending with _never or _panic
                    (strlen(function_name) > 6 && 
                     (strcmp(function_name + strlen(function_name) - 6, "_never") == 0 ||
                      strcmp(function_name + strlen(function_name) - 6, "_panic") == 0))) {
                    return true;
                }
            }
        }
        
        // Also check for explicit RET instructions after Never calls
        // If we see a RET immediately after a call, it's likely a tail call to Never
        if (inst->type == INST_RET && i > 0) {
            AssemblyInstruction *prev = block->instructions[i - 1];
            if (prev && prev->type == INST_CALL) {
                // This could be a tail call optimization to a Never function
                // For now, we'll be conservative and not mark it as Never
                // unless we have better semantic information
            }
        }
    }
    
    return false;
}

// =============================================================================
// NEVER TYPE OPTIMIZATIONS
// =============================================================================

size_t optimizer_mark_unreachable_after_never(ControlFlowGraph *cfg) {
    if (!cfg || !cfg->blocks) {
        return 0;
    }
    
    size_t blocks_marked = 0;
    
    // First pass: identify blocks with Never calls
    for (size_t i = 0; i < cfg->block_count; i++) {
        BasicBlock *block = cfg->blocks[i];
        if (!block) continue;
        
        if (optimizer_block_has_never_call(block)) {
            // Mark all successors as unreachable
            for (size_t j = 0; j < block->successor_count; j++) {
                BasicBlock *successor = block->successors[j];
                if (successor && !successor->flags.is_unreachable) {
                    successor->flags.is_unreachable = true;
                    blocks_marked++;
                    
                    // Recursively mark successors of unreachable blocks
                    // (using a simple queue would be better for large CFGs)
                    for (size_t k = 0; k < successor->successor_count; k++) {
                        BasicBlock *next = successor->successors[k];
                        if (next && !next->flags.is_unreachable) {
                            next->flags.is_unreachable = true;
                            blocks_marked++;
                        }
                    }
                }
            }
        }
    }
    
    return blocks_marked;
}

size_t optimizer_remove_never_successor_edges(ControlFlowGraph *cfg) {
    if (!cfg || !cfg->blocks) {
        return 0;
    }
    
    size_t edges_removed = 0;
    
    for (size_t i = 0; i < cfg->block_count; i++) {
        BasicBlock *block = cfg->blocks[i];
        if (!block) continue;
        
        if (optimizer_block_has_never_call(block)) {
            // Remove all outgoing edges
            size_t original_count = block->successor_count;
            
            // Clear successors
            for (size_t j = 0; j < block->successor_count; j++) {
                BasicBlock *successor = block->successors[j];
                if (successor) {
                    // Remove this block from successor's predecessors
                    for (size_t k = 0; k < successor->predecessor_count; k++) {
                        if (successor->predecessors[k] == block) {
                            // Shift remaining predecessors
                            for (size_t m = k; m < successor->predecessor_count - 1; m++) {
                                successor->predecessors[m] = successor->predecessors[m + 1];
                            }
                            successor->predecessor_count--;
                            break;
                        }
                    }
                }
                block->successors[j] = NULL;
            }
            
            block->successor_count = 0;
            edges_removed += original_count;
        }
    }
    
    return edges_removed;
}

size_t optimizer_add_never_branch_hints(ControlFlowGraph *cfg) {
    if (!cfg || !cfg->blocks) {
        return 0;
    }
    
    size_t hints_added = 0;
    
    // Look for conditional branches that lead to Never-returning blocks
    for (size_t i = 0; i < cfg->block_count; i++) {
        BasicBlock *block = cfg->blocks[i];
        if (!block || !block->instructions) continue;
        
        // Check if this block has multiple successors (conditional branch)
        if (block->successor_count < 2) continue;
        
        // Look for conditional jump instructions
        for (size_t j = 0; j < block->instruction_count; j++) {
            AssemblyInstruction *inst = block->instructions[j];
            if (!inst) continue;
            
            // Check if it's a conditional jump
            if (inst->type >= INST_JE && inst->type <= INST_JBE) {
                // Check if the jump target leads to a Never-returning function
                for (size_t k = 0; k < block->successor_count; k++) {
                    BasicBlock *successor = block->successors[k];
                    if (successor && optimizer_block_has_never_call(successor)) {
                        // Add UNLIKELY hint for branches leading to Never-returning calls
                        inst->branch_hint = BRANCH_HINT_UNLIKELY;
                        hints_added++;
                        
                        // Add comment for debugging
                        if (!inst->comment) {
                            inst->comment = strdup("unlikely: leads to Never-returning function");
                        }
                        break;
                    }
                }
                
                // If no Never-returning successor found, check if all other successors are likely
                if (inst->branch_hint == BRANCH_HINT_NONE) {
                    bool all_others_normal = true;
                    for (size_t k = 0; k < block->successor_count; k++) {
                        BasicBlock *successor = block->successors[k];
                        if (successor && optimizer_block_has_never_call(successor)) {
                            all_others_normal = false;
                            break;
                        }
                    }
                    // If this is the normal path (no Never calls), mark as likely
                    if (all_others_normal && block->successor_count > 1) {
                        inst->branch_hint = BRANCH_HINT_LIKELY;
                        hints_added++;
                        
                        if (!inst->comment) {
                            inst->comment = strdup("likely: normal execution path");
                        }
                    }
                }
            }
        }
    }
    
    return hints_added;
}

// =============================================================================
// STATISTICS TRACKING
// =============================================================================

bool optimizer_apply_never_optimizations(ControlFlowGraph *cfg, OptimizationStatistics *stats) {
    if (!cfg) {
        return false;
    }
    
    size_t total_optimizations = 0;
    size_t unreachable_blocks = 0;
    size_t edges_removed = 0;
    size_t branch_hints = 0;
    
    // Apply Never type optimizations in sequence
    
    // 1. Mark unreachable blocks after Never calls
    unreachable_blocks = optimizer_mark_unreachable_after_never(cfg);
    if (unreachable_blocks > 0) {
        total_optimizations++;
    }
    
    // 2. Remove successor edges from Never-calling blocks
    edges_removed = optimizer_remove_never_successor_edges(cfg);
    if (edges_removed > 0) {
        total_optimizations++;
    }
    
    // 3. Add branch prediction hints for Never paths
    branch_hints = optimizer_add_never_branch_hints(cfg);
    if (branch_hints > 0) {
        total_optimizations++;
    }
    
    // Update statistics if provided
    if (stats) {
        optimizer_update_never_statistics(stats, total_optimizations, 
                                        unreachable_blocks, branch_hints);
    }
    
    // Return true if any optimizations were applied
    return total_optimizations > 0;
}

void optimizer_update_never_statistics(OptimizationStatistics *stats,
                                     size_t never_optimizations,
                                     size_t unreachable_blocks,
                                     size_t branch_hints) {
    if (!stats) return;
    
    // Update Never-specific counters
    atomic_fetch_add(&stats->never_optimizations, never_optimizations);
    atomic_fetch_add(&stats->unreachable_blocks_removed, unreachable_blocks);
    atomic_fetch_add(&stats->never_branch_hints_added, branch_hints);
}