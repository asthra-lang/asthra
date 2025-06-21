/**
 * Asthra Programming Language Compiler
 * Optimizer Data Flow Analysis Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Data flow analysis implementation for optimization.
 */

#include "optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// =============================================================================
// DATA FLOW ANALYSIS OPERATIONS
// =============================================================================

DataFlowAnalysis *dataflow_create(DataFlowAnalysisType type, ControlFlowGraph *cfg) {
    if (!cfg) return NULL;
    
    DataFlowAnalysis *analysis = malloc(sizeof(DataFlowAnalysis));
    if (!analysis) return NULL;
    
    // C17 designated initializer
    *analysis = (DataFlowAnalysis) {
        .type = type,
        .cfg = cfg,
        .in_sets = NULL,
        .out_sets = NULL,
        .gen_sets = NULL,
        .kill_sets = NULL,
        .ref_count = 1,
        .iterations_performed = 0,
        .analysis_time_ns = 0
    };
    
    if (pthread_rwlock_init(&analysis->rwlock, NULL) != 0) {
        free(analysis);
        return NULL;
    }
    
    // Allocate bit vector arrays
    size_t block_count = cfg->block_count;
    analysis->in_sets = calloc(block_count, sizeof(BitVector*));
    analysis->out_sets = calloc(block_count, sizeof(BitVector*));
    analysis->gen_sets = calloc(block_count, sizeof(BitVector*));
    analysis->kill_sets = calloc(block_count, sizeof(BitVector*));
    
    if (!analysis->in_sets || !analysis->out_sets || 
        !analysis->gen_sets || !analysis->kill_sets) {
        dataflow_destroy(analysis);
        return NULL;
    }
    
    // Initialize bit vectors
    for (size_t i = 0; i < block_count; i++) {
        analysis->in_sets[i] = bitvector_create(256);
        analysis->out_sets[i] = bitvector_create(256);
        analysis->gen_sets[i] = bitvector_create(256);
        analysis->kill_sets[i] = bitvector_create(256);
        
        if (!analysis->in_sets[i] || !analysis->out_sets[i] ||
            !analysis->gen_sets[i] || !analysis->kill_sets[i]) {
            dataflow_destroy(analysis);
            return NULL;
        }
    }
    
    return analysis;
}

void dataflow_destroy(DataFlowAnalysis *analysis) {
    if (!analysis) return;
    
    pthread_rwlock_wrlock(&analysis->rwlock);
    
    if (analysis->cfg) {
        size_t block_count = analysis->cfg->block_count;
        
        if (analysis->in_sets) {
            for (size_t i = 0; i < block_count; i++) {
                bitvector_destroy(analysis->in_sets[i]);
            }
            free(analysis->in_sets);
        }
        
        if (analysis->out_sets) {
            for (size_t i = 0; i < block_count; i++) {
                bitvector_destroy(analysis->out_sets[i]);
            }
            free(analysis->out_sets);
        }
        
        if (analysis->gen_sets) {
            for (size_t i = 0; i < block_count; i++) {
                bitvector_destroy(analysis->gen_sets[i]);
            }
            free(analysis->gen_sets);
        }
        
        if (analysis->kill_sets) {
            for (size_t i = 0; i < block_count; i++) {
                bitvector_destroy(analysis->kill_sets[i]);
            }
            free(analysis->kill_sets);
        }
    }
    
    pthread_rwlock_unlock(&analysis->rwlock);
    pthread_rwlock_destroy(&analysis->rwlock);
    free(analysis);
}

void dataflow_retain(DataFlowAnalysis *analysis) {
    if (!analysis) return;
    atomic_fetch_add(&analysis->ref_count, 1);
}

void dataflow_release(DataFlowAnalysis *analysis) {
    if (!analysis) return;
    
    uint32_t old_count = atomic_fetch_sub(&analysis->ref_count, 1);
    if (old_count == 1) {
        dataflow_destroy(analysis);
    }
}

bool dataflow_analyze(DataFlowAnalysis *analysis) {
    if (!analysis || !analysis->cfg) return false;
    
    pthread_rwlock_wrlock(&analysis->rwlock);
    
    // TODO: Implement iterative data flow analysis
    // This would perform the fixed-point iteration for the specific analysis type
    
    bool converged = false;
    uint32_t iterations = 0;
    const uint32_t max_iterations = 100;
    
    while (!converged && iterations < max_iterations) {
        converged = true;
        
        // Iterate over all basic blocks
        for (size_t i = 0; i < analysis->cfg->block_count; i++) {
            BasicBlock *block = analysis->cfg->blocks[i];
            if (!block) continue;
            
            // Save old out set for convergence check
            BitVector *old_out = bitvector_clone(analysis->out_sets[i]);
            if (!old_out) {
                pthread_rwlock_unlock(&analysis->rwlock);
                return false;
            }
            
            // Compute new in set based on analysis type
            switch (analysis->type) {
                case DATAFLOW_REACHING_DEFINITIONS:
                    // IN[B] = Union of OUT[P] for all predecessors P of B
                    bitvector_clear_all(analysis->in_sets[i]);
                    for (size_t j = 0; j < block->predecessor_count; j++) {
                        // Find predecessor index
                        for (size_t k = 0; k < analysis->cfg->block_count; k++) {
                            if (analysis->cfg->blocks[k] == block->predecessors[j]) {
                                bitvector_union(analysis->in_sets[i], analysis->out_sets[k]);
                                break;
                            }
                        }
                    }
                    break;
                    
                case DATAFLOW_LIVE_VARIABLES:
                    // IN[B] = USE[B] Union (OUT[B] - DEF[B])
                    bitvector_clear_all(analysis->in_sets[i]);
                    BitVector *temp = bitvector_clone(analysis->out_sets[i]);
                    if (temp) {
                        bitvector_difference(temp, analysis->kill_sets[i]);  // OUT - DEF
                        bitvector_union(analysis->in_sets[i], analysis->gen_sets[i]);  // USE
                        bitvector_union(analysis->in_sets[i], temp);  // USE Union (OUT - DEF)
                        bitvector_destroy(temp);
                    }
                    break;
                    
                default: {
                    // Generic forward analysis: OUT[B] = GEN[B] Union (IN[B] - KILL[B])
                    BitVector *temp2 = bitvector_clone(analysis->in_sets[i]);
                    if (temp2) {
                        bitvector_difference(temp2, analysis->kill_sets[i]);  // IN - KILL
                        bitvector_clear_all(analysis->out_sets[i]);
                        bitvector_union(analysis->out_sets[i], analysis->gen_sets[i]);  // GEN
                        bitvector_union(analysis->out_sets[i], temp2);  // GEN Union (IN - KILL)
                        bitvector_destroy(temp2);
                    }
                    break;
                }
            }
            
            // Compute new out set based on analysis type
            if (analysis->type == DATAFLOW_LIVE_VARIABLES) {
                // OUT[B] = Union of IN[S] for all successors S of B
                bitvector_clear_all(analysis->out_sets[i]);
                for (size_t j = 0; j < block->successor_count; j++) {
                    // Find successor index
                    for (size_t k = 0; k < analysis->cfg->block_count; k++) {
                        if (analysis->cfg->blocks[k] == block->successors[j]) {
                            bitvector_union(analysis->out_sets[i], analysis->in_sets[k]);
                            break;
                        }
                    }
                }
            }
            
            // Check for convergence
            if (!bitvector_equals(old_out, analysis->out_sets[i])) {
                converged = false;
            }
            
            bitvector_destroy(old_out);
        }
        
        iterations++;
    }
    
    atomic_store(&analysis->iterations_performed, iterations);
    
    pthread_rwlock_unlock(&analysis->rwlock);
    return converged;
}

bool dataflow_is_converged(DataFlowAnalysis *analysis) {
    if (!analysis) return false;
    
    // Simple check - if we've performed at least one iteration
    return atomic_load(&analysis->iterations_performed) > 0;
}

uint32_t dataflow_get_iteration_count(DataFlowAnalysis *analysis) {
    if (!analysis) return 0;
    
    return atomic_load(&analysis->iterations_performed);
}

DataFlowAnalysis *dataflow_reaching_definitions(ControlFlowGraph *cfg) {
    return dataflow_create(DATAFLOW_REACHING_DEFINITIONS, cfg);
}

DataFlowAnalysis *dataflow_live_variables(ControlFlowGraph *cfg) {
    return dataflow_create(DATAFLOW_LIVE_VARIABLES, cfg);
}

DataFlowAnalysis *dataflow_available_expressions(ControlFlowGraph *cfg) {
    return dataflow_create(DATAFLOW_AVAILABLE_EXPRESSIONS, cfg);
}

// Implementation function for _Generic macro
DataFlowAnalysis *analyze_data_flow_impl(ControlFlowGraph *cfg, DataFlowAnalysisType type) {
    return dataflow_create(type, cfg);
} 
