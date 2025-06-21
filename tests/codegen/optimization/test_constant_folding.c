/**
 * Asthra Programming Language Compiler
 * Optimization Passes Testing - Constant Folding Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_optimization_passes_common.h"


// =============================================================================
// CONSTANT FOLDING TESTS
// =============================================================================

bool test_constant_folding(void) {
    Optimizer* opt = test_optimizer_create(OPTIMIZATION_LEVEL_O2);
    if (!opt) return false;
    
    InstructionBuffer* buffer = test_instruction_buffer_create(5);
    if (!buffer) {
        test_optimizer_destroy(opt);
        return false;
    }
    
    // Create instructions with constant operations
    Instruction instructions[5] = {
        {.opcode = OP_MOV, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 5}},
        {.opcode = OP_MOV, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 3}},
        {.opcode = OP_ADD, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 1}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 2}},
        {.opcode = OP_MUL, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 2}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 4}},
        {.opcode = OP_SUB, .operand1 = {.type = OPERAND_IMMEDIATE, .value = 10}, .operand2 = {.type = OPERAND_IMMEDIATE, .value = 3}}
    };
    
    for (int i = 0; i < 5; i++) {
        if (!test_instruction_buffer_add(buffer, &instructions[i])) {
            test_instruction_buffer_destroy(buffer);
            test_optimizer_destroy(opt);
            return false;
        }
    }
    
    OptimizationResult result = test_optimizer_constant_folding(opt, buffer);
    bool test_passed = result.success && result.optimizations_applied > 0 && result.constant_folds >= 1;
    
    test_instruction_buffer_destroy(buffer);
    test_optimizer_destroy(opt);
    return test_passed;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE EXECUTION
// =============================================================================

#ifndef OPTIMIZATION_PASSES_COLLECTIVE_BUILD
int main(void) {
    printf("Constant Folding Test\n");
    printf("====================\n");
    
    bool result = test_constant_folding();
    printf("Test result: %s\n", result ? "PASSED" : "FAILED");
    
    return result ? 0 : 1;
}
#endif
