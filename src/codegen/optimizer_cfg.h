/**
 * Asthra Programming Language Compiler
 * Control Flow Graph and Basic Block Structures
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Control flow graph representation and basic block management for
 * optimization and analysis.
 */

#ifndef ASTHRA_OPTIMIZER_CFG_H
#define ASTHRA_OPTIMIZER_CFG_H

#include "optimizer_types.h"
#include "optimizer_bitvector.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BASIC BLOCK STRUCTURE
// =============================================================================

// Basic block representation
struct BasicBlock {
    uint32_t id;
    char *label;
    
    // Instructions in this block
    AssemblyInstruction **instructions;
    size_t instruction_count;
    size_t instruction_capacity;
    
    // Control flow
    BasicBlock **predecessors;
    BasicBlock **successors;
    size_t predecessor_count;
    size_t successor_count;
    
    // Data flow analysis sets
    BitVector *def_set;    // Definitions in this block
    BitVector *use_set;    // Uses in this block
    BitVector *live_in;    // Live variables at block entry
    BitVector *live_out;   // Live variables at block exit
    
    // Optimization flags
    struct {
        bool is_loop_header : 1;
        bool is_loop_exit : 1;
        bool is_unreachable : 1;
        bool has_side_effects : 1;
        bool is_critical_edge : 1;
        uint8_t reserved : 3;
    } flags;
};

// =============================================================================
// CONTROL FLOW GRAPH STRUCTURE
// =============================================================================

// Control flow graph with C17 atomic operations
struct ControlFlowGraph {
    BasicBlock **blocks;
    size_t block_count;
    size_t block_capacity;
    
    BasicBlock *entry_block;
    BasicBlock *exit_block;
    
    // Graph properties with atomic updates
    atomic_uint_fast32_t next_block_id;
    atomic_bool is_reducible;
    atomic_uint_fast32_t loop_count;
    
    // Thread safety
    pthread_mutex_t mutex;
};

// =============================================================================
// CONTROL FLOW GRAPH OPERATIONS
// =============================================================================

ControlFlowGraph *cfg_create(void);
void cfg_destroy(ControlFlowGraph *cfg);

// Basic block management
BasicBlock *cfg_create_basic_block(ControlFlowGraph *cfg, const char *label);
bool cfg_add_edge(ControlFlowGraph *cfg, BasicBlock *from, BasicBlock *to);
bool cfg_remove_edge(ControlFlowGraph *cfg, BasicBlock *from, BasicBlock *to);
bool cfg_remove_basic_block(ControlFlowGraph *cfg, BasicBlock *block);

// CFG construction from instructions
bool cfg_build_from_instructions(ControlFlowGraph *cfg, 
                                AssemblyInstruction **instructions, 
                                size_t instruction_count);

// CFG analysis
bool cfg_compute_dominance(ControlFlowGraph *cfg);
bool cfg_compute_post_dominance(ControlFlowGraph *cfg);
bool cfg_detect_loops(ControlFlowGraph *cfg);
bool cfg_is_reducible(ControlFlowGraph *cfg);

// Verification and debugging
bool optimizer_verify_cfg_integrity(ControlFlowGraph *cfg);
void optimizer_print_cfg(const ControlFlowGraph *cfg);
void optimizer_print_basic_block(const BasicBlock *block);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OPTIMIZER_CFG_H 
