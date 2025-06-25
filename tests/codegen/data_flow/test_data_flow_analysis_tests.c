/*
 * Data Flow Analysis Test Functions
 * Individual test functions for reaching definitions, live variables, and use-def chains
 *
 * Part of test_data_flow_analysis_minimal.c split (585 lines -> 6 focused modules)
 * Provides comprehensive data flow analysis test coverage
 */

#include "test_data_flow_analysis_common.h"

// ============================================================================
// DATA FLOW ANALYSIS TEST FUNCTIONS
// ============================================================================

DEFINE_TEST(test_reaching_definitions) {
    DataFlowAnalysis *dfa = data_flow_analysis_create();
    if (!dfa)
        return false;

    InstructionBuffer *buffer = instruction_buffer_create(8);
    if (!buffer) {
        data_flow_analysis_destroy(dfa);
        return false;
    }

    ControlFlowGraph *cfg = control_flow_graph_create();
    if (!cfg) {
        instruction_buffer_destroy(buffer);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    ReachingDefinitions *rd = reaching_definitions_create();
    if (!rd) {
        control_flow_graph_destroy(cfg);
        instruction_buffer_destroy(buffer);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    // Create instructions with variable definitions and uses
    Instruction instructions[8] = {
        {.opcode = OP_MOV,
         .dst_reg = 0,
         .operand1 = {.type = OPERAND_IMMEDIATE, .value = 10},
         .is_definition = true},
        {.opcode = OP_MOV,
         .dst_reg = 1,
         .operand1 = {.type = OPERAND_REGISTER, .reg = 0},
         .is_use = true},
        {.opcode = OP_CMP,
         .operand1 = {.type = OPERAND_REGISTER, .reg = 1},
         .operand2 = {.type = OPERAND_IMMEDIATE, .value = 5},
         .is_use = true},
        {.opcode = OP_JGT, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 6}},
        {.opcode = OP_MOV,
         .dst_reg = 0,
         .operand1 = {.type = OPERAND_IMMEDIATE, .value = 20},
         .is_definition = true},
        {.opcode = OP_JMP, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 7}},
        {.opcode = OP_MOV,
         .dst_reg = 0,
         .operand1 = {.type = OPERAND_IMMEDIATE, .value = 30},
         .is_definition = true},
        {.opcode = OP_ADD,
         .dst_reg = 2,
         .operand1 = {.type = OPERAND_REGISTER, .reg = 0},
         .operand2 = {.type = OPERAND_REGISTER, .reg = 1},
         .is_use = true}};

    for (int i = 0; i < 8; i++) {
        if (!instruction_buffer_add(buffer, &instructions[i])) {
            reaching_definitions_destroy(rd);
            control_flow_graph_destroy(cfg);
            instruction_buffer_destroy(buffer);
            data_flow_analysis_destroy(dfa);
            return false;
        }
    }

    bool success = control_flow_graph_build(cfg, buffer);
    if (!success) {
        reaching_definitions_destroy(rd);
        control_flow_graph_destroy(cfg);
        instruction_buffer_destroy(buffer);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    success = reaching_definitions_analyze(rd, cfg);
    if (!success) {
        reaching_definitions_destroy(rd);
        control_flow_graph_destroy(cfg);
        instruction_buffer_destroy(buffer);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    // Check that definitions reach their uses
    BasicBlock *merge_block = control_flow_graph_get_block_by_index(cfg, 2);
    if (!merge_block) {
        reaching_definitions_destroy(rd);
        control_flow_graph_destroy(cfg);
        instruction_buffer_destroy(buffer);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    DefinitionSet *reaching_set = reaching_definitions_get_reaching_set(rd, merge_block);
    if (!reaching_set) {
        reaching_definitions_destroy(rd);
        control_flow_graph_destroy(cfg);
        instruction_buffer_destroy(buffer);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    size_t num_reaching_defs = definition_set_size(reaching_set);
    bool test_passed = (num_reaching_defs >= 2);

    reaching_definitions_destroy(rd);
    control_flow_graph_destroy(cfg);
    instruction_buffer_destroy(buffer);
    data_flow_analysis_destroy(dfa);
    return test_passed;
}

DEFINE_TEST(test_live_variable_analysis) {
    DataFlowAnalysis *dfa = data_flow_analysis_create();
    if (!dfa)
        return false;

    ControlFlowGraph *cfg = control_flow_graph_create();
    if (!cfg) {
        data_flow_analysis_destroy(dfa);
        return false;
    }

    LiveVariableAnalysis *lva = live_variable_analysis_create();
    if (!lva) {
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    InstructionBuffer *buffer = instruction_buffer_create(4);
    if (!buffer) {
        live_variable_analysis_destroy(lva);
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    bool success = control_flow_graph_build(cfg, buffer);
    if (!success) {
        instruction_buffer_destroy(buffer);
        live_variable_analysis_destroy(lva);
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    success = live_variable_analysis_analyze(lva, cfg);
    if (!success) {
        instruction_buffer_destroy(buffer);
        live_variable_analysis_destroy(lva);
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    // Check live variables
    BasicBlock *block = control_flow_graph_get_block_by_index(cfg, 0);
    if (!block) {
        instruction_buffer_destroy(buffer);
        live_variable_analysis_destroy(lva);
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    bool var0_live = live_variable_analysis_is_live(lva, 0, block);
    bool var1_live = live_variable_analysis_is_live(lva, 1, block);
    bool var2_dead = !live_variable_analysis_is_live(lva, 2, block);

    bool test_passed = var0_live && var1_live && var2_dead;

    instruction_buffer_destroy(buffer);
    live_variable_analysis_destroy(lva);
    control_flow_graph_destroy(cfg);
    data_flow_analysis_destroy(dfa);
    return test_passed;
}

DEFINE_TEST(test_use_def_chains) {
    DataFlowAnalysis *dfa = data_flow_analysis_create();
    if (!dfa)
        return false;

    ControlFlowGraph *cfg = control_flow_graph_create();
    if (!cfg) {
        data_flow_analysis_destroy(dfa);
        return false;
    }

    ReachingDefinitions *rd = reaching_definitions_create();
    if (!rd) {
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    UseDefChains *udc = use_def_chains_create();
    if (!udc) {
        reaching_definitions_destroy(rd);
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    InstructionBuffer *buffer = instruction_buffer_create(4);
    if (!buffer) {
        use_def_chains_destroy(udc);
        reaching_definitions_destroy(rd);
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    bool success = control_flow_graph_build(cfg, buffer);
    if (!success) {
        instruction_buffer_destroy(buffer);
        use_def_chains_destroy(udc);
        reaching_definitions_destroy(rd);
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    success = reaching_definitions_analyze(rd, cfg);
    if (!success) {
        instruction_buffer_destroy(buffer);
        use_def_chains_destroy(udc);
        reaching_definitions_destroy(rd);
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    success = use_def_chains_build(udc, cfg, rd);
    if (!success) {
        instruction_buffer_destroy(buffer);
        use_def_chains_destroy(udc);
        reaching_definitions_destroy(rd);
        control_flow_graph_destroy(cfg);
        data_flow_analysis_destroy(dfa);
        return false;
    }

    size_t chain_count = use_def_chains_get_chain_count(udc);
    bool test_passed = (chain_count >= 3);

    instruction_buffer_destroy(buffer);
    use_def_chains_destroy(udc);
    reaching_definitions_destroy(rd);
    control_flow_graph_destroy(cfg);
    data_flow_analysis_destroy(dfa);
    return test_passed;
}
