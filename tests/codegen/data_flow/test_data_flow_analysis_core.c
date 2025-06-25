/*
 * Core Data Flow Analysis and Instruction Buffer Management
 * Main data flow analysis context and instruction buffer utilities
 *
 * Part of test_data_flow_analysis_minimal.c split (585 lines -> 6 focused modules)
 * Provides fundamental data flow analysis infrastructure
 */

#include "test_data_flow_analysis_common.h"

// ============================================================================
// CORE DATA FLOW ANALYSIS FUNCTIONS
// ============================================================================

DataFlowAnalysis *data_flow_analysis_create(void) {
    DataFlowAnalysis *dfa = malloc(sizeof(DataFlowAnalysis));
    if (dfa) {
        dfa->initialized = true;
        dfa->cfg = NULL;
    }
    return dfa;
}

void data_flow_analysis_destroy(DataFlowAnalysis *dfa) {
    free(dfa);
}

// ============================================================================
// INSTRUCTION BUFFER MANAGEMENT FUNCTIONS
// ============================================================================

InstructionBuffer *instruction_buffer_create(size_t capacity) {
    InstructionBuffer *buffer = malloc(sizeof(InstructionBuffer));
    if (buffer) {
        buffer->instructions = malloc(capacity * sizeof(Instruction));
        buffer->count = 0;
        buffer->capacity = capacity;
    }
    return buffer;
}

void instruction_buffer_destroy(InstructionBuffer *buffer) {
    if (buffer) {
        free(buffer->instructions);
        free(buffer);
    }
}

bool instruction_buffer_add(InstructionBuffer *buffer, const Instruction *instr) {
    if (!buffer || !instr || buffer->count >= buffer->capacity)
        return false;
    buffer->instructions[buffer->count++] = *instr;
    return true;
}
