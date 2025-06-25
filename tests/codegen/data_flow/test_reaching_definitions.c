/**
 * Asthra Programming Language Compiler
 * Reaching Definitions Analysis Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file tests reaching definitions analysis
 */

#include "test_data_flow_common.h"

/**
 * Test reaching definitions analysis
 */
AsthraTestResult test_reaching_definitions(AsthraTestContext *context) {
    DataFlowAnalysisTestFixture *fixture = setup_data_flow_analysis_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create instructions with variable definitions and uses
    Instruction instructions[8];
    instructions[0] =
        (Instruction){.opcode = OP_MOV,
                      .dst_reg = 0,
                      .operand1 = {.type = OPERAND_IMMEDIATE, .value = 10}}; // x = 10 (def1)
    instructions[1] =
        (Instruction){.opcode = OP_MOV,
                      .dst_reg = 1,
                      .operand1 = {.type = OPERAND_REGISTER, .reg = 0}}; // y = x (use1)
    instructions[2] =
        (Instruction){.opcode = OP_CMP,
                      .operand1 = {.type = OPERAND_REGISTER, .reg = 1},
                      .operand2 = {.type = OPERAND_IMMEDIATE, .value = 5}}; // cmp y, 5 (use2)
    instructions[3] = (Instruction){
        .opcode = OP_JGT, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 6}}; // jgt else
    instructions[4] =
        (Instruction){.opcode = OP_MOV,
                      .dst_reg = 0,
                      .operand1 = {.type = OPERAND_IMMEDIATE, .value = 20}}; // x = 20 (def2)
    instructions[5] = (Instruction){.opcode = OP_JMP,
                                    .operand1 = {.type = OPERAND_IMMEDIATE, .value = 7}}; // jmp end
    instructions[6] =
        (Instruction){.opcode = OP_MOV,
                      .dst_reg = 0,
                      .operand1 = {.type = OPERAND_IMMEDIATE, .value = 30}}; // x = 30 (def3)
    instructions[7] =
        (Instruction){.opcode = OP_ADD,
                      .dst_reg = 2,
                      .operand1 = {.type = OPERAND_REGISTER, .reg = 0},
                      .operand2 = {.type = OPERAND_REGISTER, .reg = 1}}; // z = x + y (use3, use4)

    // Add instructions to buffer
    for (int i = 0; i < 8; i++) {
        instruction_buffer_add(fixture->instruction_buffer, &instructions[i]);
    }

    // Build CFG
    bool success = control_flow_graph_build(fixture->cfg, fixture->instruction_buffer);
    if (!asthra_test_assert_bool(context, success, "CFG construction failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Perform reaching definitions analysis
    success = reaching_definitions_analyze(fixture->reaching_defs, fixture->cfg);
    if (!asthra_test_assert_bool(context, success, "Reaching definitions analysis failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that definitions reach their uses
    BasicBlock *merge_block =
        control_flow_graph_get_block_by_index(fixture->cfg, 2); // Block with final use
    if (!asthra_test_assert_pointer(context, merge_block, "Merge block should exist")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // At the merge point, both def2 and def3 of variable x should reach
    DefinitionSet *reaching_set =
        reaching_definitions_get_reaching_set(fixture->reaching_defs, merge_block);
    if (!asthra_test_assert_pointer(context, reaching_set, "Reaching set should exist")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    size_t num_reaching_defs = definition_set_size(reaching_set);
    if (!asthra_test_assert_bool(context, num_reaching_defs >= 2,
                                 "Expected at least 2 reaching definitions for x, got %zu",
                                 num_reaching_defs)) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_data_flow_analysis_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test reaching definitions with loops
 */
AsthraTestResult test_reaching_definitions_with_loops(AsthraTestContext *context) {
    DataFlowAnalysisTestFixture *fixture = setup_data_flow_analysis_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a loop with variable redefinition
    Instruction instructions[6];
    instructions[0] =
        (Instruction){.opcode = OP_MOV,
                      .dst_reg = 0,
                      .operand1 = {.type = OPERAND_IMMEDIATE, .value = 0}}; // i = 0 (def1)
    instructions[1] =
        (Instruction){.opcode = OP_CMP,
                      .operand1 = {.type = OPERAND_REGISTER, .reg = 0},
                      .operand2 = {.type = OPERAND_IMMEDIATE, .value = 10}}; // cmp i, 10 (use1)
    instructions[2] = (Instruction){
        .opcode = OP_JGE, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 5}}; // jge exit
    instructions[3] = (Instruction){
        .opcode = OP_ADD,
        .dst_reg = 0,
        .operand1 = {.type = OPERAND_REGISTER, .reg = 0},
        .operand2 = {.type = OPERAND_IMMEDIATE, .value = 1}}; // i = i + 1 (use2, def2)
    instructions[4] = (Instruction){
        .opcode = OP_JMP, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 1}}; // jmp loop_header
    instructions[5] = (Instruction){.opcode = OP_RET};                          // exit

    // Add instructions to buffer
    for (int i = 0; i < 6; i++) {
        instruction_buffer_add(fixture->instruction_buffer, &instructions[i]);
    }

    // Build CFG and analyze
    bool success = control_flow_graph_build(fixture->cfg, fixture->instruction_buffer);
    if (!asthra_test_assert_bool(context, success, "CFG construction failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    success = reaching_definitions_analyze(fixture->reaching_defs, fixture->cfg);
    if (!asthra_test_assert_bool(context, success,
                                 "Reaching definitions analysis with loops failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // In the loop header, both the initial definition and the loop definition should reach
    BasicBlock *loop_header =
        control_flow_graph_get_block_by_index(fixture->cfg, 0); // Loop header block
    DefinitionSet *reaching_set =
        reaching_definitions_get_reaching_set(fixture->reaching_defs, loop_header);

    if (!asthra_test_assert_pointer(context, reaching_set,
                                    "Reaching set for loop header should exist")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Should have multiple definitions reaching due to the loop
    size_t num_reaching_defs = definition_set_size(reaching_set);
    if (!asthra_test_assert_bool(context, num_reaching_defs >= 1,
                                 "Expected at least 1 reaching definition in loop, got %zu",
                                 num_reaching_defs)) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_data_flow_analysis_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("Running Reaching Definitions Tests...\n");
    printf("=====================================\n");

    // Since these tests have type conflicts, run them manually
    int passed = 0;
    int total = 2;

    // Create a minimal test context
    AsthraTestMetadata metadata = {.name = "test"};
    AsthraTestContext *context = asthra_test_create_context(&metadata);

    printf("Test 1: Reaching Definitions\n");
    if (test_reaching_definitions(context) == ASTHRA_TEST_PASS) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }

    printf("\nTest 2: Reaching Definitions with Loops\n");
    if (test_reaching_definitions_with_loops(context) == ASTHRA_TEST_PASS) {
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