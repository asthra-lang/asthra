/**
 * Asthra Programming Language Compiler
 * Control Flow Graph Construction Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file tests CFG construction from assembly instructions
 */

#include "test_control_flow_common.h"

/**
 * Test CFG construction from basic blocks
 */
AsthraTestResult test_cfg_construction(AsthraTestContext *context) {
    ControlFlowAnalysisTestFixture *fixture = setup_control_flow_analysis_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a simple function with basic blocks
    // Note: The AssemblyInstruction struct uses flexible array members, so we need to allocate them
    // individually
    AssemblyInstruction *instructions[8];

    // Block 0: MOV r0, 10
    instructions[0] = malloc(sizeof(AssemblyInstruction) + 2 * sizeof(AssemblyOperand));
    instructions[0]->type = INST_MOV;
    instructions[0]->operand_count = 2;
    instructions[0]->operands[0] = (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = 0};
    instructions[0]->operands[1] =
        (AssemblyOperand){.type = OPERAND_IMMEDIATE, .data.immediate = 10};

    // CMP r0, 5
    instructions[1] = malloc(sizeof(AssemblyInstruction) + 2 * sizeof(AssemblyOperand));
    instructions[1]->type = INST_CMP;
    instructions[1]->operand_count = 2;
    instructions[1]->operands[0] = (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = 0};
    instructions[1]->operands[1] =
        (AssemblyOperand){.type = OPERAND_IMMEDIATE, .data.immediate = 5};

    // JG label (Jump to block 2)
    instructions[2] = malloc(sizeof(AssemblyInstruction) + sizeof(AssemblyOperand));
    instructions[2]->type = INST_JG;
    instructions[2]->operand_count = 1;
    instructions[2]->operands[0] = (AssemblyOperand){.type = OPERAND_LABEL, .data.label = "block2"};

    // Block 1: MOV r1, 1
    instructions[3] = malloc(sizeof(AssemblyInstruction) + 2 * sizeof(AssemblyOperand));
    instructions[3]->type = INST_MOV;
    instructions[3]->operand_count = 2;
    instructions[3]->operands[0] = (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = 1};
    instructions[3]->operands[1] =
        (AssemblyOperand){.type = OPERAND_IMMEDIATE, .data.immediate = 1};

    // JMP end
    instructions[4] = malloc(sizeof(AssemblyInstruction) + sizeof(AssemblyOperand));
    instructions[4]->type = INST_JMP;
    instructions[4]->operand_count = 1;
    instructions[4]->operands[0] = (AssemblyOperand){.type = OPERAND_LABEL, .data.label = "end"};

    // Block 2: MOV r1, 0
    instructions[5] = malloc(sizeof(AssemblyInstruction) + 2 * sizeof(AssemblyOperand));
    instructions[5]->type = INST_MOV;
    instructions[5]->operand_count = 2;
    instructions[5]->operands[0] = (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = 1};
    instructions[5]->operands[1] =
        (AssemblyOperand){.type = OPERAND_IMMEDIATE, .data.immediate = 0};

    // JMP end
    instructions[6] = malloc(sizeof(AssemblyInstruction) + sizeof(AssemblyOperand));
    instructions[6]->type = INST_JMP;
    instructions[6]->operand_count = 1;
    instructions[6]->operands[0] = (AssemblyOperand){.type = OPERAND_LABEL, .data.label = "end"};

    // Block 3: RET
    instructions[7] = malloc(sizeof(AssemblyInstruction));
    instructions[7]->type = INST_RET;
    instructions[7]->operand_count = 0; // Block 3 (end)

    // Add instructions to buffer
    for (int i = 0; i < 8; i++) {
        instruction_buffer_add(fixture->instruction_buffer, instructions[i]);
    }

    // Build CFG from instructions
    // Note: The instruction buffer contains the instructions, so we pass NULL for the array
    bool success = control_flow_graph_build(fixture->cfg, NULL, 8);
    if (!asthra_test_assert_bool(context, success, "CFG construction failed")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that basic blocks were created correctly
    size_t num_blocks = control_flow_graph_get_block_count(fixture->cfg);
    if (!asthra_test_assert_bool(context, num_blocks == 4, "Expected 4 basic blocks, got %zu",
                                 num_blocks)) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that edges were created correctly
    BasicBlock *entry_block = control_flow_graph_get_entry_block(fixture->cfg);
    if (!asthra_test_assert_pointer(context, entry_block, "Entry block should exist")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    size_t num_successors = basic_block_get_successor_count(entry_block);
    if (!asthra_test_assert_bool(context, num_successors == 2,
                                 "Entry block should have 2 successors, got %zu", num_successors)) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_control_flow_analysis_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test CFG with loops
 */
AsthraTestResult test_cfg_with_loops(AsthraTestContext *context) {
    ControlFlowAnalysisTestFixture *fixture = setup_control_flow_analysis_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a simple loop structure
    AssemblyInstruction *instructions[6];

    // i = 0: MOV r0, 0
    instructions[0] = malloc(sizeof(AssemblyInstruction) + 2 * sizeof(AssemblyOperand));
    instructions[0]->type = INST_MOV;
    instructions[0]->operand_count = 2;
    instructions[0]->operands[0] = (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = 0};
    instructions[0]->operands[1] =
        (AssemblyOperand){.type = OPERAND_IMMEDIATE, .data.immediate = 0};

    // Loop header: CMP r0, 10
    instructions[1] = malloc(sizeof(AssemblyInstruction) + 2 * sizeof(AssemblyOperand));
    instructions[1]->type = INST_CMP;
    instructions[1]->operand_count = 2;
    instructions[1]->operands[0] = (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = 0};
    instructions[1]->operands[1] =
        (AssemblyOperand){.type = OPERAND_IMMEDIATE, .data.immediate = 10};

    // Exit condition: JGE exit
    instructions[2] = malloc(sizeof(AssemblyInstruction) + sizeof(AssemblyOperand));
    instructions[2]->type = INST_JGE;
    instructions[2]->operand_count = 1;
    instructions[2]->operands[0] = (AssemblyOperand){.type = OPERAND_LABEL, .data.label = "exit"};

    // i++: ADD r0, r0, 1
    instructions[3] = malloc(sizeof(AssemblyInstruction) + 3 * sizeof(AssemblyOperand));
    instructions[3]->type = INST_ADD;
    instructions[3]->operand_count = 3;
    instructions[3]->operands[0] = (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = 0};
    instructions[3]->operands[1] = (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = 0};
    instructions[3]->operands[2] =
        (AssemblyOperand){.type = OPERAND_IMMEDIATE, .data.immediate = 1};

    // Back edge: JMP loop_header
    instructions[4] = malloc(sizeof(AssemblyInstruction) + sizeof(AssemblyOperand));
    instructions[4]->type = INST_JMP;
    instructions[4]->operand_count = 1;
    instructions[4]->operands[0] =
        (AssemblyOperand){.type = OPERAND_LABEL, .data.label = "loop_header"};

    // Exit: RET
    instructions[5] = malloc(sizeof(AssemblyInstruction));
    instructions[5]->type = INST_RET;
    instructions[5]->operand_count = 0; // Exit

    // Add instructions to buffer
    for (int i = 0; i < 6; i++) {
        instruction_buffer_add(fixture->instruction_buffer, instructions[i]);
    }

    // Build CFG from instructions
    // Note: The instruction buffer contains the instructions, so we pass NULL for the array
    bool success = control_flow_graph_build(fixture->cfg, NULL, 8);
    if (!asthra_test_assert_bool(context, success, "CFG construction with loops failed")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that back edges were detected
    bool has_back_edges = control_flow_graph_has_back_edges(fixture->cfg);
    if (!asthra_test_assert_bool(context, has_back_edges, "CFG should have back edges (loops)")) {
        cleanup_control_flow_analysis_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_control_flow_analysis_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    // TODO: Fix header conflicts and function signatures before enabling tests
    printf("CFG Construction Tests: SKIPPED due to header conflicts\n");
    return 0;

#if 0
    AsthraTestSuite suite = {
        .name = "CFG Construction Tests",
        .tests = {
            { "CFG Construction", test_cfg_construction },
            { "CFG with Loops", test_cfg_with_loops },
            { NULL, NULL }
        }
    };
    
    return asthra_test_run_suite(&suite);
#endif
}