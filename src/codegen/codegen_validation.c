/**
 * Asthra Programming Language Compiler
 * Code Generator - Validation and Debugging
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Functions for validating generated instructions and debugging
 */

#include "code_generator.h"
#include <stdbool.h>

bool code_generator_validate_instructions(const CodeGenerator *generator) {
    if (!generator || !generator->instruction_buffer) {
        return false;
    }
    
    InstructionBuffer *buffer = generator->instruction_buffer;
    
    // Basic validation checks
    for (size_t i = 0; i < buffer->count; i++) {
        AssemblyInstruction *inst = buffer->instructions[i];
        if (!inst) {
            return false;  // Null instruction
        }
        
        // Validate instruction type
        if (inst->type > INST_COUNT) {
            return false;  // Invalid instruction type
        }
        
        // Special handling for comment instructions
        if (inst->type == INST_COUNT) {
            // This is a comment instruction - validate it has a comment and no operands
            if (!inst->comment || inst->operand_count != 0) {
                return false;  // Invalid comment instruction
            }
            continue;  // Skip operand validation for comments
        }
        
        // Validate operand count
        if (inst->operand_count > 3) {  // Most x86-64 instructions have at most 3 operands
            return false;  // Too many operands
        }
        
        // Validate each operand
        for (size_t j = 0; j < inst->operand_count; j++) {
            AssemblyOperand *operand = &inst->operands[j];
            
            switch (operand->type) {
                case OPERAND_REGISTER:
                    if (operand->data.reg >= ASTHRA_REG_COUNT || operand->data.reg < 0) {
                        return false;  // Invalid register
                    }
                    break;
                    
                case OPERAND_IMMEDIATE:
                    // Immediate values are generally valid (no specific constraints)
                    break;
                    
                case OPERAND_MEMORY:
                    // Validate memory operand registers
                    if (operand->data.memory.base >= ASTHRA_REG_COUNT || 
                        (operand->data.memory.index != ASTHRA_REG_NONE && operand->data.memory.index >= ASTHRA_REG_COUNT)) {
                        return false;  // Invalid memory operand register
                    }
                    // Validate scale factor
                    if (operand->data.memory.scale != 1 && operand->data.memory.scale != 2 && 
                        operand->data.memory.scale != 4 && operand->data.memory.scale != 8) {
                        return false;  // Invalid scale factor
                    }
                    break;
                    
                case OPERAND_LABEL:
                    if (!operand->data.label) {
                        return false;  // Null label
                    }
                    break;
                    
                default:
                    return false;  // Invalid operand type
            }
        }
    }
    
    return true;  // All validations passed
}