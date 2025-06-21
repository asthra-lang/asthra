/**
 * Asthra Programming Language Compiler
 * Code Generator - Architecture-Specific Formatting
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Functions for formatting assembly instructions for different architectures
 */

#include "code_generator.h"
#include <stdio.h>
#include <string.h>

// Forward declarations for static functions
static int format_x86_64_instruction(const AssemblyInstruction *inst, 
                                    char *buffer, size_t buffer_size);
static int format_aarch64_instruction(const AssemblyInstruction *inst, 
                                     char *buffer, size_t buffer_size);
static const char* format_x86_64_operand(const AssemblyOperand *operand);
static const char* format_aarch64_operand(const AssemblyOperand *operand);
static const char* get_x86_64_register_name(Register reg);
static const char* get_aarch64_register_name(Register reg);

/**
 * Format a single instruction into assembly text based on target architecture
 */
int code_generator_format_instruction(const CodeGenerator *generator, 
                                     const AssemblyInstruction *inst,
                                     char *buffer, size_t buffer_size) {
    if (!generator || !inst || !buffer || buffer_size == 0) {
        return -1;
    }
    
    // Choose formatter based on target architecture
    switch (generator->target_arch) {
        case TARGET_ARCH_AARCH64:
            return format_aarch64_instruction(inst, buffer, buffer_size);
        case TARGET_ARCH_X86_64:
            return format_x86_64_instruction(inst, buffer, buffer_size);
        default:
            return snprintf(buffer, buffer_size, "    ; Unsupported architecture\n");
    }
}

/**
 * Format x86_64 assembly instruction
 */
static int format_x86_64_instruction(const AssemblyInstruction *inst, 
                                    char *buffer, size_t buffer_size) {
    if (!inst || !buffer) return -1;
    
    switch (inst->type) {
        case INST_MOV:
            if (inst->operand_count == 2) {
                return snprintf(buffer, buffer_size, "    movq %s, %s\n",
                               format_x86_64_operand(&inst->operands[1]),
                               format_x86_64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_PUSH:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    pushq %s\n",
                               format_x86_64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_POP:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    popq %s\n",
                               format_x86_64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_RET:
            return snprintf(buffer, buffer_size, "    retq\n");
            
        default:
            return snprintf(buffer, buffer_size, "    ; x86_64 instruction type %d\n", inst->type);
    }
    
    return snprintf(buffer, buffer_size, "    ; Invalid x86_64 instruction\n");
}

/**
 * Format ARM64/AArch64 assembly instruction
 */
static int format_aarch64_instruction(const AssemblyInstruction *inst, 
                                     char *buffer, size_t buffer_size) {
    if (!inst || !buffer) return -1;
    
    switch (inst->type) {
        case INST_MOV:
            if (inst->operand_count == 2) {
                return snprintf(buffer, buffer_size, "    mov %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]));
            }
            break;
            
        case INST_ADD:
            if (inst->operand_count == 2) {
                return snprintf(buffer, buffer_size, "    add %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]));
            }
            break;
            
        case INST_SUB:
            if (inst->operand_count == 2) {
                return snprintf(buffer, buffer_size, "    sub %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]));
            }
            break;
            
        case INST_PUSH:
            // ARM64 doesn't have push instruction, use str with pre-decrement
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    str %s, [sp, #-16]!\n",
                               format_aarch64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_POP:
            // ARM64 doesn't have pop instruction, use ldr with post-increment
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    ldr %s, [sp], #16\n",
                               format_aarch64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_RET:
            return snprintf(buffer, buffer_size, "    ret\n");
            
        default:
            return snprintf(buffer, buffer_size, "    ; ARM64 instruction type %d\n", inst->type);
    }
    
    return snprintf(buffer, buffer_size, "    ; Invalid ARM64 instruction\n");
}

/**
 * Format x86_64 operand 
 */
static const char* format_x86_64_operand(const AssemblyOperand *operand) {
    static char operand_buffer[64];
    
    if (!operand) return "<?>";
    
    switch (operand->type) {
        case OPERAND_REGISTER:
            return get_x86_64_register_name(operand->data.reg);
        case OPERAND_IMMEDIATE:
            snprintf(operand_buffer, sizeof(operand_buffer), "$%lld", 
                    (long long)operand->data.immediate);
            return operand_buffer;
        case OPERAND_MEMORY:
            snprintf(operand_buffer, sizeof(operand_buffer), "%d(%%%s)", 
                    operand->data.memory.displacement,
                    get_x86_64_register_name(operand->data.memory.base));
            return operand_buffer;
        default:
            return "<?>";
    }
}

/**
 * Format ARM64 operand
 */
static const char* format_aarch64_operand(const AssemblyOperand *operand) {
    static char operand_buffer[64];
    
    if (!operand) return "<?>";
    
    switch (operand->type) {
        case OPERAND_REGISTER:
            return get_aarch64_register_name(operand->data.reg);
        case OPERAND_IMMEDIATE:
            snprintf(operand_buffer, sizeof(operand_buffer), "#%lld", 
                    (long long)operand->data.immediate);
            return operand_buffer;
        case OPERAND_MEMORY:
            snprintf(operand_buffer, sizeof(operand_buffer), "[%s, #%d]", 
                    get_aarch64_register_name(operand->data.memory.base),
                    operand->data.memory.displacement);
            return operand_buffer;
        default:
            return "<?>";
    }
}

/**
 * Get x86_64 register name
 */
static const char* get_x86_64_register_name(Register reg) {
    static const char* x86_64_reg_names[] = {
        "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    };
    
    if (reg < sizeof(x86_64_reg_names) / sizeof(x86_64_reg_names[0])) {
        return x86_64_reg_names[reg];
    }
    return "?";
}

/**
 * Get ARM64 register name  
 */
static const char* get_aarch64_register_name(Register reg) {
    static const char* aarch64_reg_names[] = {
        "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
        "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
        "x24", "x25", "x26", "x27", "x28", "x29", "x30", "sp"
    };
    
    if (reg < sizeof(aarch64_reg_names) / sizeof(aarch64_reg_names[0])) {
        return aarch64_reg_names[reg];
    }
    return "?";
}