/*
 * Live Variable Analysis for Data Flow Analysis
 * Live variable analysis creation, computation, and liveness queries
 *
 * Part of test_data_flow_analysis_minimal.c split (585 lines -> 6 focused modules)
 * Provides live variable analysis infrastructure
 */

#include "test_data_flow_analysis_common.h"

// ============================================================================
// LIVE VARIABLE ANALYSIS FUNCTIONS
// ============================================================================

LiveVariableAnalysis *live_variable_analysis_create(void) {
    LiveVariableAnalysis *lva = malloc(sizeof(LiveVariableAnalysis));
    if (lva) {
        lva->initialized = true;
        lva->live_vars = NULL;
        lva->num_vars = 0;
    }
    return lva;
}

void live_variable_analysis_destroy(LiveVariableAnalysis *lva) {
    if (lva) {
        free(lva->live_vars);
        free(lva);
    }
}

bool live_variable_analysis_analyze(LiveVariableAnalysis *lva, ControlFlowGraph *cfg) {
    if (!lva || !cfg)
        return false;

    // Simulate live variable analysis
    lva->num_vars = 4; // Simulate 4 variables
    lva->live_vars = malloc(lva->num_vars * sizeof(int));

    // Simulate liveness: variables 0 and 1 are live, 2 and 3 are dead
    lva->live_vars[0] = 1; // Variable 0 is live
    lva->live_vars[1] = 1; // Variable 1 is live
    lva->live_vars[2] = 0; // Variable 2 is dead
    lva->live_vars[3] = 0; // Variable 3 is dead

    return true;
}

bool live_variable_analysis_is_live(LiveVariableAnalysis *lva, int var_id, BasicBlock *block) {
    if (!lva || !block || var_id < 0 || (size_t)var_id >= lva->num_vars)
        return false;

    // Simple simulation: return liveness based on pre-computed values
    return lva->live_vars[var_id] != 0;
}
