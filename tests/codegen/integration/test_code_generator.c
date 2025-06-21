/**
 * Asthra Programming Language Compiler
 * Code Generator Test
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "code_generator.h"
#include "ast.h"

// Test code generator creation and destruction
static bool test_code_generator_creation(void) {
    printf("Testing code generator creation and destruction...\n");
    
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!generator) {
        printf("FAIL: Failed to create code generator\n");
        return false;
    }
    
    // Check that all components were created
    if (!generator->instruction_buffer) {
        printf("FAIL: Instruction buffer not created\n");
        code_generator_destroy(generator);
        return false;
    }
    
    if (!generator->register_allocator) {
        printf("FAIL: Register allocator not created\n");
        code_generator_destroy(generator);
        return false;
    }
    
    if (!generator->label_manager) {
        printf("FAIL: Label manager not created\n");
        code_generator_destroy(generator);
        return false;
    }
    
    if (!generator->symbol_table) {
        printf("FAIL: Symbol table not created\n");
        code_generator_destroy(generator);
        return false;
    }
    
    // Clean up
    code_generator_destroy(generator);
    printf("PASS: Code generator created and destroyed successfully\n");
    return true;
}

// Test instruction buffer
static bool test_instruction_buffer(void) {
    printf("Testing instruction buffer...\n");
    
    InstructionBuffer *buffer = instruction_buffer_create(10);
    if (!buffer) {
        printf("FAIL: Failed to create instruction buffer\n");
        return false;
    }
    
    // Create some test instructions
    AssemblyInstruction *mov = create_mov_instruction(ASTHRA_REG_RAX, ASTHRA_REG_RBX);
    AssemblyInstruction *ret = create_ret_instruction();
    
    if (!mov || !ret) {
        printf("FAIL: Failed to create test instructions\n");
        instruction_buffer_destroy(buffer);
        return false;
    }
    
    // Add instructions to buffer
    if (!instruction_buffer_add(buffer, mov)) {
        printf("FAIL: Failed to add MOV instruction to buffer\n");
        instruction_buffer_destroy(buffer);
        return false;
    }
    
    if (!instruction_buffer_add(buffer, ret)) {
        printf("FAIL: Failed to add RET instruction to buffer\n");
        instruction_buffer_destroy(buffer);
        return false;
    }
    
    // Check buffer count
    if (buffer->count != 2) {
        printf("FAIL: Instruction buffer count incorrect. Expected 2, got %zu\n", buffer->count);
        instruction_buffer_destroy(buffer);
        return false;
    }
    
    // Clean up
    instruction_buffer_destroy(buffer);
    printf("PASS: Instruction buffer tests passed\n");
    return true;
}

// Test register allocator
static bool test_register_allocator(void) {
    printf("Testing register allocator...\n");
    
    RegisterAllocator *allocator = register_allocator_create();
    if (!allocator) {
        printf("FAIL: Failed to create register allocator\n");
        return false;
    }
    
    // Allocate a register
    Register reg1 = register_allocate(allocator, true);
    if (reg1 == ASTHRA_REG_NONE) {
        printf("FAIL: Failed to allocate register\n");
        register_allocator_destroy(allocator);
        return false;
    }
    
    // Check that the register is allocated
    if (!register_is_allocated(allocator, reg1)) {
        printf("FAIL: Register not marked as allocated\n");
        register_allocator_destroy(allocator);
        return false;
    }
    
    // Free the register
    register_free(allocator, reg1);
    
    // Check that the register is no longer allocated
    if (register_is_allocated(allocator, reg1)) {
        printf("FAIL: Register still marked as allocated after freeing\n");
        register_allocator_destroy(allocator);
        return false;
    }
    
    // Clean up
    register_allocator_destroy(allocator);
    printf("PASS: Register allocator tests passed\n");
    return true;
}

// Test label manager
static bool test_label_manager(void) {
    printf("Testing label manager...\n");
    
    LabelManager *manager = label_manager_create(16);
    if (!manager) {
        printf("FAIL: Failed to create label manager\n");
        return false;
    }
    
    // Create a label
    char *label = label_manager_create_label(manager, LABEL_FUNCTION, "test_func");
    if (!label) {
        printf("FAIL: Failed to create label\n");
        label_manager_destroy(manager);
        return false;
    }
    
    // Define the label
    if (!label_manager_define_label(manager, label, 100)) {
        printf("FAIL: Failed to define label\n");
        free(label);
        label_manager_destroy(manager);
        return false;
    }
    
    // Check that the label is defined
    if (!label_manager_is_defined(manager, label)) {
        printf("FAIL: Label not marked as defined\n");
        free(label);
        label_manager_destroy(manager);
        return false;
    }
    
    // Clean up
    free(label);
    label_manager_destroy(manager);
    printf("PASS: Label manager tests passed\n");
    return true;
}

// Test instruction creation
static bool test_instruction_creation(void) {
    printf("Testing instruction creation...\n");
    
    // Test MOV instruction
    AssemblyInstruction *mov = create_mov_instruction(ASTHRA_REG_RAX, ASTHRA_REG_RBX);
    if (!mov) {
        printf("FAIL: Failed to create MOV instruction\n");
        return false;
    }
    
    if (mov->type != INST_MOV) {
        printf("FAIL: MOV instruction has incorrect type\n");
        free(mov);
        return false;
    }
    
    if (mov->operand_count != 2) {
        printf("FAIL: MOV instruction has incorrect operand count\n");
        free(mov);
        return false;
    }
    
    if (mov->operands[0].type != OPERAND_REGISTER || mov->operands[0].data.reg != ASTHRA_REG_RAX) {
        printf("FAIL: MOV instruction has incorrect destination operand\n");
        free(mov);
        return false;
    }
    
    if (mov->operands[1].type != OPERAND_REGISTER || mov->operands[1].data.reg != ASTHRA_REG_RBX) {
        printf("FAIL: MOV instruction has incorrect source operand\n");
        free(mov);
        return false;
    }
    
    free(mov);
    
    // Test MOV immediate instruction
    AssemblyInstruction *mov_imm = create_mov_immediate(ASTHRA_REG_RAX, 42);
    if (!mov_imm) {
        printf("FAIL: Failed to create MOV immediate instruction\n");
        return false;
    }
    
    if (mov_imm->type != INST_MOV) {
        printf("FAIL: MOV immediate instruction has incorrect type\n");
        free(mov_imm);
        return false;
    }
    
    if (mov_imm->operand_count != 2) {
        printf("FAIL: MOV immediate instruction has incorrect operand count\n");
        free(mov_imm);
        return false;
    }
    
    if (mov_imm->operands[0].type != OPERAND_REGISTER || mov_imm->operands[0].data.reg != ASTHRA_REG_RAX) {
        printf("FAIL: MOV immediate instruction has incorrect destination operand\n");
        free(mov_imm);
        return false;
    }
    
    if (mov_imm->operands[1].type != OPERAND_IMMEDIATE || mov_imm->operands[1].data.immediate != 42) {
        printf("FAIL: MOV immediate instruction has incorrect source operand\n");
        free(mov_imm);
        return false;
    }
    
    free(mov_imm);
    
    // Test RET instruction
    AssemblyInstruction *ret = create_ret_instruction();
    if (!ret) {
        printf("FAIL: Failed to create RET instruction\n");
        return false;
    }
    
    if (ret->type != INST_RET) {
        printf("FAIL: RET instruction has incorrect type\n");
        free(ret);
        return false;
    }
    
    if (ret->operand_count != 0) {
        printf("FAIL: RET instruction has incorrect operand count\n");
        free(ret);
        return false;
    }
    
    free(ret);
    
    printf("PASS: Instruction creation tests passed\n");
    return true;
}

int main(void) {
    printf("=== Asthra Code Generator Tests ===\n");
    
    bool all_passed = true;
    
    // Run tests
    all_passed &= test_code_generator_creation();
    all_passed &= test_instruction_buffer();
    all_passed &= test_register_allocator();
    all_passed &= test_label_manager();
    all_passed &= test_instruction_creation();
    
    // Summary
    if (all_passed) {
        printf("\nAll tests PASSED!\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
} 
