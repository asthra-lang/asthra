/**
 * Asthra Programming Language Compiler
 * Optimization Passes Testing - Strength Reduction Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_optimization_passes_common.h"

// =============================================================================
// STRENGTH REDUCTION TESTS
// =============================================================================

bool test_strength_reduction(void) {
    Optimizer *opt = test_optimizer_create(OPTIMIZATION_LEVEL_O2);
    if (!opt)
        return false;

    InstructionBuffer *buffer = test_instruction_buffer_create(3);
    if (!buffer) {
        test_optimizer_destroy(opt);
        return false;
    }

    // Create instructions with strength reduction opportunities
    Instruction instructions[3] = {{.opcode = OP_MUL,
                                    .dst_reg = 0,
                                    .operand1 = {.type = OPERAND_REGISTER, .reg = 1},
                                    .operand2 = {.type = OPERAND_IMMEDIATE, .value = 2}},
                                   {.opcode = OP_MUL,
                                    .dst_reg = 1,
                                    .operand1 = {.type = OPERAND_REGISTER, .reg = 2},
                                    .operand2 = {.type = OPERAND_IMMEDIATE, .value = 4}},
                                   {.opcode = OP_MUL,
                                    .dst_reg = 2,
                                    .operand1 = {.type = OPERAND_REGISTER, .reg = 3},
                                    .operand2 = {.type = OPERAND_IMMEDIATE, .value = 8}}};

    for (int i = 0; i < 3; i++) {
        if (!test_instruction_buffer_add(buffer, &instructions[i])) {
            test_instruction_buffer_destroy(buffer);
            test_optimizer_destroy(opt);
            return false;
        }
    }

    OptimizationResult result = test_optimizer_strength_reduction(opt, buffer);
    bool test_passed = result.success && result.strength_reductions >= 3;

    test_instruction_buffer_destroy(buffer);
    test_optimizer_destroy(opt);
    return test_passed;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE EXECUTION
// =============================================================================

#ifndef OPTIMIZATION_PASSES_COLLECTIVE_BUILD
int main(void) {
    printf("Strength Reduction Test\n");
    printf("======================\n");

    bool result = test_strength_reduction();
    printf("Test result: %s\n", result ? "PASSED" : "FAILED");

    return result ? 0 : 1;
}
#endif
