/**
 * Asthra Programming Language Compiler
 * Data Flow Analysis Framework
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Data flow analysis algorithms and structures for optimization.
 */

#ifndef ASTHRA_OPTIMIZER_DATAFLOW_H
#define ASTHRA_OPTIMIZER_DATAFLOW_H

#include "optimizer_types.h"
#include "optimizer_bitvector.h"
#include "optimizer_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DATA FLOW ANALYSIS STRUCTURE
// =============================================================================

// Data flow analysis results with atomic reference counting
struct DataFlowAnalysis {
    DataFlowAnalysisType type;
    ControlFlowGraph *cfg;
    
    // Analysis results
    BitVector **in_sets;   // Input sets for each basic block
    BitVector **out_sets;  // Output sets for each basic block
    BitVector **gen_sets;  // Generated sets for each basic block
    BitVector **kill_sets; // Killed sets for each basic block
    
    // C17 atomic reference counting for thread-safe sharing
    atomic_uint_fast32_t ref_count;
    
    // Analysis statistics with memory ordering
    atomic_uint_fast32_t iterations_performed;
    atomic_uint_fast64_t analysis_time_ns;
    
    // Thread safety
    pthread_rwlock_t rwlock;
};

// =============================================================================
// DATA FLOW ANALYSIS OPERATIONS
// =============================================================================

DataFlowAnalysis *dataflow_create(DataFlowAnalysisType type, ControlFlowGraph *cfg);
void dataflow_destroy(DataFlowAnalysis *analysis);

// Reference counting with atomic operations
void dataflow_retain(DataFlowAnalysis *analysis);
void dataflow_release(DataFlowAnalysis *analysis);

// Analysis execution
bool dataflow_analyze(DataFlowAnalysis *analysis);
bool dataflow_is_converged(DataFlowAnalysis *analysis);
uint32_t dataflow_get_iteration_count(DataFlowAnalysis *analysis);

// Specific analyses
DataFlowAnalysis *dataflow_reaching_definitions(ControlFlowGraph *cfg);
DataFlowAnalysis *dataflow_live_variables(ControlFlowGraph *cfg);
DataFlowAnalysis *dataflow_available_expressions(ControlFlowGraph *cfg);

// Type-safe data flow analysis dispatch
#define analyze_data_flow(cfg, type) _Generic((type), \
    DataFlowAnalysisType: analyze_data_flow_impl, \
    default: analyze_data_flow_impl \
)(cfg, type)

// Implementation function (not for direct use)
DataFlowAnalysis *analyze_data_flow_impl(ControlFlowGraph *cfg, DataFlowAnalysisType type);

// Verification and debugging
bool optimizer_verify_dataflow_results(DataFlowAnalysis *analysis);
void optimizer_print_dataflow_results(const DataFlowAnalysis *analysis);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OPTIMIZER_DATAFLOW_H 
