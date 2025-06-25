/**
 * Asthra Programming Language Compiler
 * Use-Def Chains Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file tests use-def chain construction and analysis
 */

#include "test_data_flow_common.h"

/**
 * Test use-def chain construction
 */
AsthraTestResult test_use_def_chains(AsthraTestContext *context) {
    DataFlowAnalysisTestFixture *fixture = setup_data_flow_analysis_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create instructions with clear def-use relationships
    Instruction instructions[6];
    instructions[0] =
        (Instruction){.opcode = OP_MOV,
                      .dst_reg = 0,
                      .operand1 = {.type = OPERAND_IMMEDIATE, .value = 42}}; // x = 42 (def1)
    instructions[1] =
        (Instruction){.opcode = OP_ADD,
                      .dst_reg = 1,
                      .operand1 = {.type = OPERAND_REGISTER, .reg = 0},
                      .operand2 = {.type = OPERAND_IMMEDIATE, .value = 8}}; // y = x + 8 (use1)
    instructions[2] =
        (Instruction){.opcode = OP_MOV,
                      .dst_reg = 0,
                      .operand1 = {.type = OPERAND_IMMEDIATE, .value = 100}}; // x = 100 (def2)
    instructions[3] =
        (Instruction){.opcode = OP_MUL,
                      .dst_reg = 2,
                      .operand1 = {.type = OPERAND_REGISTER, .reg = 0},
                      .operand2 = {.type = OPERAND_REGISTER, .reg = 1}}; // z = x * y (use2, use3)
    instructions[4] =
        (Instruction){.opcode = OP_SUB,
                      .dst_reg = 3,
                      .operand1 = {.type = OPERAND_REGISTER, .reg = 2},
                      .operand2 = {.type = OPERAND_IMMEDIATE, .value = 10}}; // w = z - 10 (use4)
    instructions[5] = (Instruction){.opcode = OP_RET};

    // Add instructions to buffer
    for (int i = 0; i < 6; i++) {
        instruction_buffer_add(fixture->instruction_buffer, &instructions[i]);
    }

    // Build CFG
    bool success = control_flow_graph_build(fixture->cfg, fixture->instruction_buffer);
    if (!asthra_test_assert_bool(context, success, "CFG construction failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Build use-def chains
    success = use_def_chains_build(fixture->use_def_chains, fixture->cfg);
    if (!asthra_test_assert_bool(context, success, "Use-def chain construction failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check specific use-def relationships
    // use1 (instruction 1, operand 1) should link to def1 (instruction 0)
    DefList *def_list =
        use_def_chains_get_defs_for_use(fixture->use_def_chains, 1, 0); // Instruction 1, operand 0
    if (!asthra_test_assert_pointer(context, def_list, "Def list for use1 should exist")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    size_t num_defs = def_list_size(def_list);
    if (!asthra_test_assert_bool(context, num_defs == 1,
                                 "Use1 should have exactly 1 definition, got %zu", num_defs)) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    Definition *def = def_list_get(def_list, 0);
    if (!asthra_test_assert_bool(context, def->instruction_index == 0,
                                 "Use1 should link to instruction 0, got %d",
                                 def->instruction_index)) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // use2 (instruction 3, operand 1) should link to def2 (instruction 2)
    def_list =
        use_def_chains_get_defs_for_use(fixture->use_def_chains, 3, 0); // Instruction 3, operand 0
    if (!asthra_test_assert_pointer(context, def_list, "Def list for use2 should exist")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    num_defs = def_list_size(def_list);
    if (!asthra_test_assert_bool(context, num_defs == 1,
                                 "Use2 should have exactly 1 definition, got %zu", num_defs)) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    def = def_list_get(def_list, 0);
    if (!asthra_test_assert_bool(context, def->instruction_index == 2,
                                 "Use2 should link to instruction 2, got %d",
                                 def->instruction_index)) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_data_flow_analysis_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test use-def chains with multiple definitions
 */
AsthraTestResult test_use_def_chains_multiple_defs(AsthraTestContext *context) {
    DataFlowAnalysisTestFixture *fixture = setup_data_flow_analysis_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a diamond CFG where a variable has multiple reaching definitions
    Instruction instructions[7];
    instructions[0] =
        (Instruction){.opcode = OP_MOV,
                      .dst_reg = 0,
                      .operand1 = {.type = OPERAND_IMMEDIATE, .value = 1}}; // x = 1 (def1)
    instructions[1] = (Instruction){.opcode = OP_JZ,
                                    .operand1 = {.type = OPERAND_IMMEDIATE, .value = 4}}; // jz else
    instructions[2] =
        (Instruction){.opcode = OP_MOV,
                      .dst_reg = 0,
                      .operand1 = {.type = OPERAND_IMMEDIATE, .value = 10}}; // x = 10 (def2)
    instructions[3] = (Instruction){
        .opcode = OP_JMP, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 5}}; // jmp merge
    instructions[4] =
        (Instruction){.opcode = OP_MOV,
                      .dst_reg = 0,
                      .operand1 = {.type = OPERAND_IMMEDIATE, .value = 20}}; // x = 20 (def3)
    instructions[5] = (Instruction){
        .opcode = OP_ADD,
        .dst_reg = 1,
        .operand1 = {.type = OPERAND_REGISTER, .reg = 0},
        .operand2 = {.type = OPERAND_IMMEDIATE, .value = 5}}; // y = x + 5 (use1 - multiple defs)
    instructions[6] = (Instruction){.opcode = OP_RET};

    // Add instructions to buffer
    for (int i = 0; i < 7; i++) {
        instruction_buffer_add(fixture->instruction_buffer, &instructions[i]);
    }

    // Build CFG and use-def chains
    bool success = control_flow_graph_build(fixture->cfg, fixture->instruction_buffer);
    if (!asthra_test_assert_bool(context, success, "CFG construction failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    success = use_def_chains_build(fixture->use_def_chains, fixture->cfg);
    if (!asthra_test_assert_bool(context, success, "Use-def chain construction failed")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // The use at instruction 5 should have multiple reaching definitions
    DefList *def_list =
        use_def_chains_get_defs_for_use(fixture->use_def_chains, 5, 0); // Instruction 5, operand 0
    if (!asthra_test_assert_pointer(context, def_list,
                                    "Def list for use with multiple defs should exist")) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    size_t num_defs = def_list_size(def_list);
    if (!asthra_test_assert_bool(context, num_defs >= 2,
                                 "Use should have multiple definitions, got %zu", num_defs)) {
        cleanup_data_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_data_flow_analysis_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("Running Use-Def Chains Tests...\n");
    printf("=====================================\n");

    // Since these tests have type conflicts, run them manually
    int passed = 0;
    int total = 2;

    // Create a minimal test context
    AsthraTestMetadata metadata = {.name = "test"};
    AsthraTestContext *context = asthra_test_create_context(&metadata);

    printf("Test 1: Use-Def Chains\n");
    if (test_use_def_chains(context) == ASTHRA_TEST_PASS) {
        printf("✓ PASSED\n");
        passed++;
    } else {
        printf("✗ FAILED\n");
    }

    printf("\nTest 2: Use-Def Chains Multiple Defs\n");
    if (test_use_def_chains_multiple_defs(context) == ASTHRA_TEST_PASS) {
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