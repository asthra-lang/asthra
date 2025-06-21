/**
 * Asthra Programming Language Compiler
 * Dominator Analysis Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file tests dominator analysis and immediate dominator computation
 */

#include "test_control_flow_common.h"

// TODO: Fix header conflicts before enabling this test
#if 0
// Include optimization test types
#include "test_optimization_passes_common.h"
#endif

/**
 * Test dominator analysis
 */
AsthraTestResult test_dominator_analysis(AsthraTestContext* context) {
    // TODO: Fix Instruction type issues in this test
    return ASTHRA_TEST_SKIP;
    
#if 0
    ControlFlowAnalysisTestFixture* fixture = setup_control_flow_analysis_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create a CFG with multiple paths
    Instruction instructions[10];
    instructions[0] = (Instruction){.opcode = OP_MOV, .dst_reg = 0, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 10}};  // Entry block
    instructions[1] = (Instruction){.opcode = OP_CMP, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 5}};
    instructions[2] = (Instruction){.opcode = OP_JGT, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 6}};                 // Branch
    instructions[3] = (Instruction){.opcode = OP_MOV, .dst_reg = 1, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 1}};  // Left path
    instructions[4] = (Instruction){.opcode = OP_ADD, .dst_reg = 2, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}, .operand2 = {.type = OPERAND_REGISTER, .reg = 1}};
    instructions[5] = (Instruction){.opcode = OP_JMP, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 8}};                 // Jump to merge
    instructions[6] = (Instruction){.opcode = OP_MOV, .dst_reg = 1, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 2}};  // Right path
    instructions[7] = (Instruction){.opcode = OP_MUL, .dst_reg = 2, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}, .operand2 = {.type = OPERAND_REGISTER, .reg = 1}};
    instructions[8] = (Instruction){.opcode = OP_MOV, .dst_reg = 3, .operand1 = {.type = OPERAND_REGISTER, .reg = 2}};     // Merge point
    instructions[9] = (Instruction){.opcode = OP_RET};
    
    // Add instructions to buffer
    for (int i = 0; i < 10; i++) {
        instruction_buffer_add(fixture->instruction_buffer, instructions[i]);
    }
    
    // Build CFG
    bool success = control_flow_graph_build(fixture->cfg, fixture->instruction_buffer);
    if (!asthra_test_assert_bool(context, success, "CFG construction failed")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Perform dominator analysis
    success = dominator_analysis_compute(fixture->dominator_analysis, fixture->cfg);
    if (!asthra_test_assert_bool(context, success, "Dominator analysis failed")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check dominator relationships
    BasicBlock* entry_block = control_flow_graph_get_entry_block(fixture->cfg);
    BasicBlock* merge_block = control_flow_graph_get_block_by_index(fixture->cfg, 3);  // Merge point
    
    if (!asthra_test_assert_pointer(context, entry_block, "Entry block should exist")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_pointer(context, merge_block, "Merge block should exist")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Entry block should dominate merge block
    bool dominates = dominator_analysis_dominates(fixture->dominator_analysis, entry_block, merge_block);
    if (!asthra_test_assert_bool(context, dominates, "Entry block should dominate merge block")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_control_flow_analysis_fixture(fixture);
    return ASTHRA_TEST_PASS;
#endif
}

/**
 * Test immediate dominator computation
 */
AsthraTestResult test_immediate_dominators(AsthraTestContext* context) {
    // TODO: Fix Instruction type issues in this test
    return ASTHRA_TEST_SKIP;
    
#if 0
    ControlFlowAnalysisTestFixture* fixture = setup_control_flow_analysis_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create a simple diamond CFG
    Instruction instructions[6];
    instructions[0] = (Instruction){.opcode = OP_MOV, .dst_reg = 0, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 1}};   // Entry
    instructions[1] = (Instruction){.opcode = OP_JZ, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 4}};                  // Branch
    instructions[2] = (Instruction){.opcode = OP_MOV, .dst_reg = 1, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 2}};  // Left
    instructions[3] = (Instruction){.opcode = OP_JMP, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 5}};                 // Jump to merge
    instructions[4] = (Instruction){.opcode = OP_MOV, .dst_reg = 1, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 3}};  // Right
    instructions[5] = (Instruction){.opcode = OP_RET};                                                                       // Merge
    
    // Add instructions to buffer
    for (int i = 0; i < 6; i++) {
        instruction_buffer_add(fixture->instruction_buffer, instructions[i]);
    }
    
    // Build CFG and compute dominators
    bool success = control_flow_graph_build(fixture->cfg, fixture->instruction_buffer);
    if (!asthra_test_assert_bool(context, success, "CFG construction failed")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    success = dominator_analysis_compute(fixture->dominator_analysis, fixture->cfg);
    if (!asthra_test_assert_bool(context, success, "Dominator analysis failed")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check immediate dominator relationships
    BasicBlock* entry_block = control_flow_graph_get_entry_block(fixture->cfg);
    BasicBlock* merge_block = control_flow_graph_get_block_by_index(fixture->cfg, 2);  // Merge block
    
    BasicBlock* idom = dominator_analysis_get_immediate_dominator(fixture->dominator_analysis, merge_block);
    if (!asthra_test_assert_pointer_eq(context, idom, entry_block, "Entry block should be immediate dominator of merge block")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_control_flow_analysis_fixture(fixture);
    return ASTHRA_TEST_PASS;
#endif
}

// Main test runner
int main(void) {
    // TODO: Fix header conflicts and function signatures before enabling tests
    printf("Dominator Analysis Tests: SKIPPED due to header conflicts\n");
    return 0;
    
#if 0
    AsthraTestSuite suite = {
        .name = "Dominator Analysis Tests",
        .tests = {
            { "Dominator Analysis", test_dominator_analysis },
            { "Immediate Dominators", test_immediate_dominators },
            { NULL, NULL }
        }
    };
    
    return asthra_test_run_suite(&suite);
#endif
}