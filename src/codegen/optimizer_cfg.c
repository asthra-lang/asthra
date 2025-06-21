/**
 * Asthra Programming Language Compiler
 * Optimizer Control Flow Graph Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Control Flow Graph construction and manipulation.
 */

#include "optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// =============================================================================
// CONTROL FLOW GRAPH OPERATIONS
// =============================================================================

ControlFlowGraph *cfg_create(void) {
    ControlFlowGraph *cfg = malloc(sizeof(ControlFlowGraph));
    if (!cfg) return NULL;
    
    cfg->blocks = calloc(16, sizeof(BasicBlock*));
    if (!cfg->blocks) {
        free(cfg);
        return NULL;
    }
    
    // C17 designated initializer
    *cfg = (ControlFlowGraph) {
        .blocks = cfg->blocks,
        .block_count = 0,
        .block_capacity = 16,
        .entry_block = NULL,
        .exit_block = NULL,
        .next_block_id = 1,
        .is_reducible = true,
        .loop_count = 0
    };
    
    if (pthread_mutex_init(&cfg->mutex, NULL) != 0) {
        free(cfg->blocks);
        free(cfg);
        return NULL;
    }
    
    return cfg;
}

void cfg_destroy(ControlFlowGraph *cfg) {
    if (!cfg) return;
    
    pthread_mutex_lock(&cfg->mutex);
    
    // Destroy all basic blocks
    for (size_t i = 0; i < cfg->block_count; i++) {
        BasicBlock *block = cfg->blocks[i];
        if (block) {
            free(block->label);
            free(block->instructions);
            free(block->predecessors);
            free(block->successors);
            bitvector_destroy(block->def_set);
            bitvector_destroy(block->use_set);
            bitvector_destroy(block->live_in);
            bitvector_destroy(block->live_out);
            free(block);
        }
    }
    
    free(cfg->blocks);
    pthread_mutex_unlock(&cfg->mutex);
    pthread_mutex_destroy(&cfg->mutex);
    free(cfg);
}

BasicBlock *cfg_create_basic_block(ControlFlowGraph *cfg, const char *label) {
    if (!cfg) return NULL;
    
    pthread_mutex_lock(&cfg->mutex);
    
    // Resize blocks array if necessary
    if (cfg->block_count >= cfg->block_capacity) {
        size_t new_capacity = cfg->block_capacity * 2;
        BasicBlock **new_blocks = realloc(cfg->blocks, new_capacity * sizeof(BasicBlock*));
        if (!new_blocks) {
            pthread_mutex_unlock(&cfg->mutex);
            return NULL;
        }
        cfg->blocks = new_blocks;
        cfg->block_capacity = new_capacity;
    }
    
    BasicBlock *block = calloc(1, sizeof(BasicBlock));
    if (!block) {
        pthread_mutex_unlock(&cfg->mutex);
        return NULL;
    }
    
    // C17 designated initializer
    *block = (BasicBlock) {
        .id = atomic_fetch_add(&cfg->next_block_id, 1),
        .label = label ? strdup(label) : NULL,
        .instructions = calloc(8, sizeof(AssemblyInstruction*)),
        .instruction_count = 0,
        .instruction_capacity = 8,
        .predecessors = calloc(4, sizeof(BasicBlock*)),
        .successors = calloc(4, sizeof(BasicBlock*)),
        .predecessor_count = 0,
        .successor_count = 0,
        .def_set = bitvector_create(256),  // Assume max 256 variables
        .use_set = bitvector_create(256),
        .live_in = bitvector_create(256),
        .live_out = bitvector_create(256),
        .flags = {
            .is_loop_header = false,
            .is_loop_exit = false,
            .is_unreachable = false,
            .has_side_effects = false,
            .is_critical_edge = false
        }
    };
    
    if (!block->instructions || !block->predecessors || !block->successors ||
        !block->def_set || !block->use_set || !block->live_in || !block->live_out) {
        // Cleanup on failure
        free(block->label);
        free(block->instructions);
        free(block->predecessors);
        free(block->successors);
        bitvector_destroy(block->def_set);
        bitvector_destroy(block->use_set);
        bitvector_destroy(block->live_in);
        bitvector_destroy(block->live_out);
        free(block);
        pthread_mutex_unlock(&cfg->mutex);
        return NULL;
    }
    
    cfg->blocks[cfg->block_count++] = block;
    
    pthread_mutex_unlock(&cfg->mutex);
    return block;
}

bool cfg_add_edge(ControlFlowGraph *cfg, BasicBlock *from, BasicBlock *to) {
    if (!cfg || !from || !to) return false;
    
    pthread_mutex_lock(&cfg->mutex);
    
    // Add to successors of 'from' block
    if (from->successor_count >= 4) {  // Simple limit for now
        pthread_mutex_unlock(&cfg->mutex);
        return false;
    }
    
    // Check if edge already exists
    for (size_t i = 0; i < from->successor_count; i++) {
        if (from->successors[i] == to) {
            pthread_mutex_unlock(&cfg->mutex);
            return true;  // Edge already exists
        }
    }
    
    from->successors[from->successor_count++] = to;
    
    // Add to predecessors of 'to' block
    if (to->predecessor_count >= 4) {  // Simple limit for now
        pthread_mutex_unlock(&cfg->mutex);
        return false;
    }
    
    to->predecessors[to->predecessor_count++] = from;
    
    pthread_mutex_unlock(&cfg->mutex);
    return true;
}

bool cfg_remove_edge(ControlFlowGraph *cfg, BasicBlock *from, BasicBlock *to) {
    if (!cfg || !from || !to) return false;
    
    pthread_mutex_lock(&cfg->mutex);
    
    // Remove from successors of 'from' block
    for (size_t i = 0; i < from->successor_count; i++) {
        if (from->successors[i] == to) {
            // Shift remaining successors
            for (size_t j = i; j < from->successor_count - 1; j++) {
                from->successors[j] = from->successors[j + 1];
            }
            from->successor_count--;
            break;
        }
    }
    
    // Remove from predecessors of 'to' block
    for (size_t i = 0; i < to->predecessor_count; i++) {
        if (to->predecessors[i] == from) {
            // Shift remaining predecessors
            for (size_t j = i; j < to->predecessor_count - 1; j++) {
                to->predecessors[j] = to->predecessors[j + 1];
            }
            to->predecessor_count--;
            break;
        }
    }
    
    pthread_mutex_unlock(&cfg->mutex);
    return true;
}

bool cfg_remove_basic_block(ControlFlowGraph *cfg, BasicBlock *block) {
    if (!cfg || !block) return false;
    
    pthread_mutex_lock(&cfg->mutex);
    
    // Remove all edges to and from this block
    for (size_t i = 0; i < block->successor_count; i++) {
        cfg_remove_edge(cfg, block, block->successors[i]);
    }
    
    for (size_t i = 0; i < block->predecessor_count; i++) {
        cfg_remove_edge(cfg, block->predecessors[i], block);
    }
    
    // Find and remove block from CFG
    for (size_t i = 0; i < cfg->block_count; i++) {
        if (cfg->blocks[i] == block) {
            // Shift remaining blocks
            for (size_t j = i; j < cfg->block_count - 1; j++) {
                cfg->blocks[j] = cfg->blocks[j + 1];
            }
            cfg->block_count--;
            break;
        }
    }
    
    // Clean up block
    free(block->label);
    free(block->instructions);
    free(block->predecessors);
    free(block->successors);
    bitvector_destroy(block->def_set);
    bitvector_destroy(block->use_set);
    bitvector_destroy(block->live_in);
    bitvector_destroy(block->live_out);
    free(block);
    
    pthread_mutex_unlock(&cfg->mutex);
    return true;
}

bool cfg_build_from_instructions(ControlFlowGraph *cfg, 
                                AssemblyInstruction **instructions, 
                                size_t instruction_count) {
    if (!cfg || !instructions) return false;
    
    // TODO: Implement CFG construction from instructions
    // This would analyze control flow instructions (jumps, branches, calls)
    // and create basic blocks accordingly
    
    (void)instruction_count;  // Suppress unused parameter warning
    return true;
}

bool cfg_compute_dominance(ControlFlowGraph *cfg) {
    if (!cfg) return false;
    
    // TODO: Implement dominance analysis
    // This would compute dominance relationships between basic blocks
    
    return true;
}

bool cfg_compute_post_dominance(ControlFlowGraph *cfg) {
    if (!cfg) return false;
    
    // TODO: Implement post-dominance analysis
    // This would compute post-dominance relationships between basic blocks
    
    return true;
}

bool cfg_detect_loops(ControlFlowGraph *cfg) {
    if (!cfg) return false;
    
    // TODO: Implement loop detection
    // This would identify natural loops in the CFG
    
    return true;
}

bool cfg_is_reducible(ControlFlowGraph *cfg) {
    if (!cfg) return false;
    
    // TODO: Implement reducibility test
    // This would determine if the CFG is reducible (structured)
    
    return atomic_load(&cfg->is_reducible);
} 
