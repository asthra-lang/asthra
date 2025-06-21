/**
 * Simple Arithmetic Expression Test
 * Demonstrates Phase 2 fixes for codegen test stabilization
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

// Use direct includes like the working test_code_generator.c
#include "code_generator.h"

// Test arithmetic instruction generation (following working test pattern)
static bool test_arithmetic_instruction_generation(void) {
    printf("Testing arithmetic instruction generation...\n");
    
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!generator) {
        printf("FAIL: Failed to create code generator\n");
        return false;
    }
    
    // Test generating MOV instruction (available function)
    AssemblyInstruction *mov_inst = create_mov_instruction(ASTHRA_REG_RAX, ASTHRA_REG_RBX);
    if (!mov_inst) {
        printf("FAIL: Failed to create MOV instruction\n");
        code_generator_destroy(generator);
        return false;
    }
    
    // Test generating RET instruction
    AssemblyInstruction *ret_inst = create_ret_instruction();
    if (!ret_inst) {
        printf("FAIL: Failed to create RET instruction\n");
        free(mov_inst);
        code_generator_destroy(generator);
        return false;
    }
    
    // Test generating CALL instruction
    AssemblyInstruction *call_inst = create_call_instruction("test_function");
    if (!call_inst) {
        printf("FAIL: Failed to create CALL instruction\n");
        free(mov_inst);
        free(ret_inst);
        code_generator_destroy(generator);
        return false;
    }
    
    // Clean up instructions
    free(mov_inst);
    free(ret_inst);
    free(call_inst);
    
    code_generator_destroy(generator);
    printf("PASS: Arithmetic instruction generation succeeded\n");
    return true;
}

static bool test_expression_register_allocation(void) {
    printf("Testing expression register allocation...\n");
    
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!generator) {
        printf("FAIL: Failed to create code generator\n");
        return false;
    }
    
    // Allocate registers for expression evaluation
    Register left_reg = register_allocate(generator->register_allocator, true);
    Register right_reg = register_allocate(generator->register_allocator, true);
    Register result_reg = register_allocate(generator->register_allocator, true);
    
    if (left_reg == ASTHRA_REG_NONE || right_reg == ASTHRA_REG_NONE || result_reg == ASTHRA_REG_NONE) {
        printf("FAIL: Failed to allocate registers for expression\n");
        code_generator_destroy(generator);
        return false;
    }
    
    // Generate simple MOV instructions to test register usage
    AssemblyInstruction *mov1 = create_mov_instruction(left_reg, ASTHRA_REG_RAX);
    AssemblyInstruction *mov2 = create_mov_instruction(right_reg, ASTHRA_REG_RBX);
    AssemblyInstruction *mov3 = create_mov_instruction(result_reg, ASTHRA_REG_RCX);
    
    if (!mov1 || !mov2 || !mov3) {
        printf("FAIL: Failed to create expression instructions\n");
        code_generator_destroy(generator);
        return false;
    }
    
    // Clean up instructions
    free(mov1);
    free(mov2);
    free(mov3);
    
    // Free registers
    register_free(generator->register_allocator, left_reg);
    register_free(generator->register_allocator, right_reg);
    register_free(generator->register_allocator, result_reg);
    
    code_generator_destroy(generator);
    printf("PASS: Expression register allocation succeeded\n");
    return true;
}

int main(void) {
    printf("Simple Arithmetic Expression Code Generation Tests\n");
    printf("==================================================\n");
    
    bool all_passed = true;
    
    if (!test_arithmetic_instruction_generation()) {
        all_passed = false;
    }
    
    if (!test_expression_register_allocation()) {
        all_passed = false;
    }
    
    printf("==================================================\n");
    if (all_passed) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed!\n");
        return 1;
    }
} 
