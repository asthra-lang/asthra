/**
 * Asthra Programming Language Compiler
 * Optimization Passes Testing - Peephole Optimization Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_optimization_passes_common.h"

// =============================================================================
// PEEPHOLE OPTIMIZATION TESTS
// =============================================================================

bool test_peephole_optimization(void) {
    Optimizer *opt = test_optimizer_create(OPTIMIZATION_LEVEL_O2);
    if (!opt)
        return false;

    InstructionBuffer *buffer = test_instruction_buffer_create(4);
    if (!buffer) {
        test_optimizer_destroy(opt);
        return false;
    }

    // Create instructions with peephole optimization opportunities
    Instruction instructions[4] = {
        {.opcode = OP_MOV, .dst_reg = 0, .operand1 = {.type = OPERAND_REGISTER, .reg = 1}},
        {.opcode = OP_MOV,
         .dst_reg = 1,
         .operand1 = {.type = OPERAND_REGISTER, .reg = 0}}, // Redundant
        {.opcode = OP_ADD,
         .dst_reg = 2,
         .operand1 = {.type = OPERAND_REGISTER, .reg = 2},
         .operand2 = {.type = OPERAND_IMMEDIATE, .value = 0}}, // Add 0
        {.opcode = OP_MOV, .dst_reg = 3, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 42}}};

    for (int i = 0; i < 4; i++) {
        if (!test_instruction_buffer_add(buffer, &instructions[i])) {
            test_instruction_buffer_destroy(buffer);
            test_optimizer_destroy(opt);
            return false;
        }
    }

    OptimizationResult result = test_optimizer_peephole_optimization(opt, buffer);
    bool test_passed = result.success && result.optimizations_applied >= 1;

    test_instruction_buffer_destroy(buffer);
    test_optimizer_destroy(opt);
    return test_passed;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE EXECUTION
// =============================================================================

#ifndef OPTIMIZATION_PASSES_COLLECTIVE_BUILD
int main(void) {
    printf("Peephole Optimization Test\n");
    printf("=========================\n");

    bool result = test_peephole_optimization();
    printf("Test result: %s\n", result ? "PASSED" : "FAILED");

    return result ? 0 : 1;
}
#endif
