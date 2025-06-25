/*
 * Use-Def Chains Analysis for Data Flow Analysis
 * Use-def chains creation, building, and chain queries
 *
 * Part of test_data_flow_analysis_minimal.c split (585 lines -> 6 focused modules)
 * Provides use-def chains analysis infrastructure
 */

#include "test_data_flow_analysis_common.h"

// ============================================================================
// USE-DEF CHAINS ANALYSIS FUNCTIONS
// ============================================================================

UseDefChains *use_def_chains_create(void) {
    UseDefChains *udc = malloc(sizeof(UseDefChains));
    if (udc) {
        udc->initialized = true;
        udc->chains = NULL;
        udc->num_chains = 0;
    }
    return udc;
}

void use_def_chains_destroy(UseDefChains *udc) {
    if (udc) {
        free(udc->chains);
        free(udc);
    }
}

bool use_def_chains_build(UseDefChains *udc, ControlFlowGraph *cfg, ReachingDefinitions *rd) {
    if (!udc || !cfg || !rd)
        return false;

    // Simulate use-def chain construction
    udc->num_chains = 3;
    udc->chains = malloc(udc->num_chains * sizeof(int));
    udc->chains[0] = 1; // Use at instruction 1 -> def at instruction 0
    udc->chains[1] = 3; // Use at instruction 3 -> def at instruction 2
    udc->chains[2] = 5; // Use at instruction 5 -> def at instruction 4

    return true;
}

size_t use_def_chains_get_chain_count(UseDefChains *udc) {
    return udc ? udc->num_chains : 0;
}
