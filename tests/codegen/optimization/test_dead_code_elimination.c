/**
 * Asthra Programming Language Compiler
 * Optimization Passes Testing - Dead Code Elimination Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_optimization_passes_common.h"

// =============================================================================
// DEAD CODE ELIMINATION TESTS
// =============================================================================

bool test_dead_code_elimination(void) {
    Optimizer *opt = test_optimizer_create(OPTIMIZATION_LEVEL_O2);
    if (!opt)
        return false;

    InstructionBuffer *buffer = test_instruction_buffer_create(6);
    if (!buffer) {
        test_optimizer_destroy(opt);
        return false;
    }

    // Create instructions with some dead code
    Instruction instructions[6] = {
        {.opcode = OP_MOV, .dst_reg = 0, .is_used = true},  // Live
        {.opcode = OP_ADD, .dst_reg = 1, .is_used = false}, // Dead
        {.opcode = OP_MOV, .dst_reg = 2, .is_used = true},  // Live
        {.opcode = OP_SUB, .dst_reg = 3, .is_used = false}, // Dead
        {.opcode = OP_MUL, .dst_reg = 4, .is_used = false}, // Dead
        {.opcode = OP_MOV, .dst_reg = 5, .is_used = true}   // Live
    };

    for (int i = 0; i < 6; i++) {
        if (!test_instruction_buffer_add(buffer, &instructions[i])) {
            test_instruction_buffer_destroy(buffer);
            test_optimizer_destroy(opt);
            return false;
        }
    }

    OptimizationResult result = test_optimizer_dead_code_elimination(opt, buffer);
    bool test_passed = result.success && result.dead_code_eliminated == 3 &&
                       test_instruction_buffer_size(buffer) == 3;

    test_instruction_buffer_destroy(buffer);
    test_optimizer_destroy(opt);
    return test_passed;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE EXECUTION
// =============================================================================

#ifndef OPTIMIZATION_PASSES_COLLECTIVE_BUILD
int main(void) {
    printf("Dead Code Elimination Test\n");
    printf("==========================\n");

    bool result = test_dead_code_elimination();
    printf("Test result: %s\n", result ? "PASSED" : "FAILED");

    return result ? 0 : 1;
}
#endif
