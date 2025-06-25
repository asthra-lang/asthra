/*
 * Control Flow Graph Management for Data Flow Analysis
 * Control flow graph creation, building, and basic block management
 *
 * Part of test_data_flow_analysis_minimal.c split (585 lines -> 6 focused modules)
 * Provides control flow graph infrastructure for data flow analysis
 */

#include "test_data_flow_analysis_common.h"

// ============================================================================
// CONTROL FLOW GRAPH MANAGEMENT FUNCTIONS
// ============================================================================

ControlFlowGraph *control_flow_graph_create(void) {
    ControlFlowGraph *cfg = malloc(sizeof(ControlFlowGraph));
    if (cfg) {
        cfg->blocks = NULL;
        cfg->block_count = 0;
    }
    return cfg;
}

void control_flow_graph_destroy(ControlFlowGraph *cfg) {
    if (cfg) {
        if (cfg->blocks) {
            for (size_t i = 0; i < cfg->block_count; i++) {
                free(cfg->blocks[i]);
            }
            free(cfg->blocks);
        }
        free(cfg);
    }
}

bool control_flow_graph_build(ControlFlowGraph *cfg, InstructionBuffer *buffer) {
    if (!cfg || !buffer)
        return false;

    // Simple simulation: create basic blocks
    cfg->block_count = 3; // Simulated block count for data flow tests
    cfg->blocks = malloc(cfg->block_count * sizeof(BasicBlock *));

    for (size_t i = 0; i < cfg->block_count; i++) {
        cfg->blocks[i] = malloc(sizeof(BasicBlock));
        cfg->blocks[i]->id = (int)i;
        cfg->blocks[i]->start_instruction = i * 2;
        cfg->blocks[i]->end_instruction = (i * 2) + 1;
    }

    return true;
}

BasicBlock *control_flow_graph_get_block_by_index(ControlFlowGraph *cfg, size_t index) {
    if (!cfg || index >= cfg->block_count)
        return NULL;
    return cfg->blocks[index];
}
