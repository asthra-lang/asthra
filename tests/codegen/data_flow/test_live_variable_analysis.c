/**
 * Asthra Programming Language Compiler
 * Live Variable Analysis Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file tests live variable analysis
 */

#include "test_data_flow_common.h"

/**
 * Test live variable analysis
 */
AsthraTestResult test_live_variable_analysis(AsthraTestContext* context) {
    DataFlowAnalysisTestFixture* fixture = setup_data_flow_analysis_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create instructions with variable liveness patterns
    Instruction instructions[7];
    instructions[0] = (Instruction){.opcode = OP_MOV, .dst_reg = 0, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 10}};  // x = 10
    instructions[1] = (Instruction){.opcode = OP_MOV, .dst_reg = 1, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 20}};  // y = 20
    instructions[2] = (Instruction){.opcode = OP_ADD, .dst_reg = 2, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}, .operand2 = {.type = OPERAND_REGISTER, .reg = 1}};  // z = x + y (x, y live here)
    instructions[3] = (Instruction){.opcode = OP_MOV, .dst_reg = 0, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 30}};  // x = 30 (old x dies, new x born)
    instructions[4] = (Instruction){.opcode = OP_MUL, .dst_reg = 3, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}, .operand2 = {.type = OPERAND_REGISTER, .reg = 2}};  // w = x * z (x, z live here)
    instructions[5] = (Instruction){.opcode = OP_ADD, .dst_reg = 4, .operand1 = {.type = OPERAND_REGISTER, .reg = 3}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 5}};  // result = w + 5 (w live here)
    instructions[6] = (Instruction){.opcode = OP_RET};                                                                       // return (result live here)
    
    // Add instructions to buffer
    for (int i = 0; i < 7; i++) {
        instruction_buffer_add(fixture->instruction_buffer, &instructions[i]);
    }
    
    // Build CFG
    bool success = control_flow_graph_build(fixture->cfg, fixture->instruction_buffer);
    if (!asthra_test_assert_bool(context, success, "CFG construction failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Perform live variable analysis
    success = live_variable_analysis_analyze(fixture->live_vars, fixture->cfg);
    if (!asthra_test_assert_bool(context, success, "Live variable analysis failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check liveness at specific program points
    BasicBlock* entry_block = control_flow_graph_get_entry_block(fixture->cfg);
    if (!asthra_test_assert_pointer(context, entry_block, "Entry block should exist")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // At instruction 2 (z = x + y), both x and y should be live
    LivenessSet* live_set = live_variable_analysis_get_live_in(fixture->live_vars, entry_block, 2);
    if (!asthra_test_assert_pointer(context, live_set, "Live set should exist")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    bool x_live = liveness_set_contains(live_set, 0);  // Register 0 (x)
    bool y_live = liveness_set_contains(live_set, 1);  // Register 1 (y)
    
    if (!asthra_test_assert_bool(context, x_live, "Variable x should be live before z = x + y")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, y_live, "Variable y should be live before z = x + y")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_data_flow_analysis_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test live variable analysis with control flow
 */
AsthraTestResult test_live_variables_with_control_flow(AsthraTestContext* context) {
    DataFlowAnalysisTestFixture* fixture = setup_data_flow_analysis_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create a diamond CFG with different variable usage patterns
    Instruction instructions[8];
    instructions[0] = (Instruction){.opcode = OP_MOV, .dst_reg = 0, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 10}};  // x = 10
    instructions[1] = (Instruction){.opcode = OP_MOV, .dst_reg = 1, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 20}};  // y = 20
    instructions[2] = (Instruction){.opcode = OP_JZ, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 5}};                  // jz else
    instructions[3] = (Instruction){.opcode = OP_ADD, .dst_reg = 2, .operand1 = {.type = OPERAND_REGISTER, .reg = 0}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 5}};  // z = x + 5 (x used)
    instructions[4] = (Instruction){.opcode = OP_JMP, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 6}};                 // jmp merge
    instructions[5] = (Instruction){.opcode = OP_ADD, .dst_reg = 2, .operand1 = {.type = OPERAND_REGISTER, .reg = 1}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 10}};  // z = y + 10 (y used)
    instructions[6] = (Instruction){.opcode = OP_MUL, .dst_reg = 3, .operand1 = {.type = OPERAND_REGISTER, .reg = 2}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 2}};  // w = z * 2 (z used)
    instructions[7] = (Instruction){.opcode = OP_RET};
    
    // Add instructions to buffer
    for (int i = 0; i < 8; i++) {
        instruction_buffer_add(fixture->instruction_buffer, &instructions[i]);
    }
    
    // Build CFG and analyze
    bool success = control_flow_graph_build(fixture->cfg, fixture->instruction_buffer);
    if (!asthra_test_assert_bool(context, success, "CFG construction failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    success = live_variable_analysis_analyze(fixture->live_vars, fixture->cfg);
    if (!asthra_test_assert_bool(context, success, "Live variable analysis with control flow failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // At the branch point, both x and y should be live since they're used in different branches
    BasicBlock* entry_block = control_flow_graph_get_entry_block(fixture->cfg);
    LivenessSet* live_set = live_variable_analysis_get_live_out(fixture->live_vars, entry_block);
    
    if (!asthra_test_assert_pointer(context, live_set, "Live out set should exist")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    bool x_live = liveness_set_contains(live_set, 0);  // Register 0 (x)
    bool y_live = liveness_set_contains(live_set, 1);  // Register 1 (y)
    
    if (!asthra_test_assert_bool(context, x_live, "Variable x should be live at branch point")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, y_live, "Variable y should be live at branch point")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_data_flow_analysis_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("Running Live Variable Analysis Tests...\n");
    printf("=====================================\n");
    
    // Since these tests have type conflicts, run them manually
    int passed = 0;
    int total = 2;
    
    // Create a minimal test context
    AsthraTestMetadata metadata = { .name = "test" };
    AsthraTestContext* context = asthra_test_create_context(&metadata);
    
    printf("Test 1: Live Variable Analysis\n");
    if (test_live_variable_analysis(context) == ASTHRA_TEST_PASS) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }
    
    printf("\nTest 2: Live Variables with Control Flow\n");
    if (test_live_variables_with_control_flow(context) == ASTHRA_TEST_PASS) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }
    
    asthra_test_destroy_context(context);
    
    printf("\n=====================================\n");
    printf("Results: %d/%d tests passed\n", passed, total);
    
    return (passed == total) ? 0 : 1;
}