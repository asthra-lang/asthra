/*
 * Reaching Definitions Analysis for Data Flow Analysis
 * Reaching definitions creation, analysis, and definition set management
 *
 * Part of test_data_flow_analysis_minimal.c split (585 lines -> 6 focused modules)
 * Provides reaching definitions analysis infrastructure
 */

#include "test_data_flow_analysis_common.h"

// ============================================================================
// REACHING DEFINITIONS ANALYSIS FUNCTIONS
// ============================================================================

ReachingDefinitions *reaching_definitions_create(void) {
    ReachingDefinitions *rd = malloc(sizeof(ReachingDefinitions));
    if (rd) {
        rd->initialized = true;
        rd->block_sets = NULL;
        rd->num_blocks = 0;
    }
    return rd;
}

void reaching_definitions_destroy(ReachingDefinitions *rd) {
    if (rd) {
        if (rd->block_sets) {
            for (size_t i = 0; i < rd->num_blocks; i++) {
                if (rd->block_sets[i]) {
                    free(rd->block_sets[i]->definitions);
                    free(rd->block_sets[i]);
                }
            }
            free(rd->block_sets);
        }
        free(rd);
    }
}

bool reaching_definitions_analyze(ReachingDefinitions *rd, ControlFlowGraph *cfg) {
    if (!rd || !cfg)
        return false;

    // Simulate reaching definitions analysis
    rd->num_blocks = cfg->block_count;
    rd->block_sets = malloc(rd->num_blocks * sizeof(DefinitionSet *));

    for (size_t i = 0; i < rd->num_blocks; i++) {
        rd->block_sets[i] = malloc(sizeof(DefinitionSet));
        rd->block_sets[i]->capacity = 4;
        rd->block_sets[i]->definitions = malloc(rd->block_sets[i]->capacity * sizeof(int));
        rd->block_sets[i]->count = 2; // Simulate 2 reaching definitions per block
        rd->block_sets[i]->definitions[0] = (int)i * 2;
        rd->block_sets[i]->definitions[1] = (int)i * 2 + 1;
    }

    return true;
}

DefinitionSet *reaching_definitions_get_reaching_set(ReachingDefinitions *rd, BasicBlock *block) {
    if (!rd || !block || block->id < 0 || (size_t)block->id >= rd->num_blocks)
        return NULL;
    return rd->block_sets[block->id];
}

// ============================================================================
// DEFINITION SET UTILITY FUNCTIONS
// ============================================================================

size_t definition_set_size(DefinitionSet *set) {
    return set ? set->count : 0;
}
