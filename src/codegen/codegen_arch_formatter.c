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
static int format_x86_64_instruction(const CodeGenerator *generator, const AssemblyInstruction *inst, 
                                    char *buffer, size_t buffer_size);
static int format_aarch64_instruction(const AssemblyInstruction *inst, 
                                     char *buffer, size_t buffer_size);
static int format_wasm_instruction(const AssemblyInstruction *inst, 
                                  char *buffer, size_t buffer_size);
static const char* format_x86_64_operand(const CodeGenerator *generator, const AssemblyOperand *operand);
static const char* format_aarch64_operand(const AssemblyOperand *operand);
static const char* format_wasm_operand(const AssemblyOperand *operand);
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
            return format_x86_64_instruction(generator, inst, buffer, buffer_size);
        case TARGET_ARCH_WASM32:
            return format_wasm_instruction(inst, buffer, buffer_size);
        default:
            return snprintf(buffer, buffer_size, "    ; Unsupported architecture\n");
    }
}

/**
 * Format x86_64 assembly instruction with syntax awareness
 */
static int format_x86_64_instruction(const CodeGenerator *generator, const AssemblyInstruction *inst, 
                                    char *buffer, size_t buffer_size) {
    if (!generator || !inst || !buffer) return -1;
    
    bool is_intel = (generator->asm_syntax == ASTHRA_ASM_SYNTAX_INTEL);
    
    switch (inst->type) {
        case INST_MOV:
            if (inst->operand_count == 2) {
                if (is_intel) {
                    return snprintf(buffer, buffer_size, "    mov %s, %s\n",
                                   format_x86_64_operand(generator, &inst->operands[0]),
                                   format_x86_64_operand(generator, &inst->operands[1]));
                } else {
                    return snprintf(buffer, buffer_size, "    movq %s, %s\n",
                                   format_x86_64_operand(generator, &inst->operands[1]),
                                   format_x86_64_operand(generator, &inst->operands[0]));
                }
            }
            break;
            
        case INST_PUSH:
            if (inst->operand_count == 1) {
                if (is_intel) {
                    return snprintf(buffer, buffer_size, "    push %s\n",
                                   format_x86_64_operand(generator, &inst->operands[0]));
                } else {
                    return snprintf(buffer, buffer_size, "    pushq %s\n",
                                   format_x86_64_operand(generator, &inst->operands[0]));
                }
            }
            break;
            
        case INST_POP:
            if (inst->operand_count == 1) {
                if (is_intel) {
                    return snprintf(buffer, buffer_size, "    pop %s\n",
                                   format_x86_64_operand(generator, &inst->operands[0]));
                } else {
                    return snprintf(buffer, buffer_size, "    popq %s\n",
                                   format_x86_64_operand(generator, &inst->operands[0]));
                }
            }
            break;
            
        case INST_RET:
            return snprintf(buffer, buffer_size, "    ret\n");
            
        default:
            return snprintf(buffer, buffer_size, "    ; x86_64 instruction type %d\n", inst->type);
    }
    
    return snprintf(buffer, buffer_size, "    ; Invalid x86_64 instruction\n");
}

/**
 * Format ARM64/AArch64 assembly instruction with enhanced instruction set
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
            } else if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    add %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]),
                               format_aarch64_operand(&inst->operands[2]));
            }
            break;
            
        case INST_SUB:
            if (inst->operand_count == 2) {
                return snprintf(buffer, buffer_size, "    sub %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]));
            } else if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    sub %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]),
                               format_aarch64_operand(&inst->operands[2]));
            }
            break;
            
        case INST_MUL:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    mul %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]),
                               format_aarch64_operand(&inst->operands[2]));
            }
            break;
            
        case INST_DIV:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    udiv %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]),
                               format_aarch64_operand(&inst->operands[2]));
            }
            break;
            
        case INST_AND:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    and %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]),
                               format_aarch64_operand(&inst->operands[2]));
            }
            break;
            
        case INST_OR:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    orr %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]),
                               format_aarch64_operand(&inst->operands[2]));
            }
            break;
            
        case INST_XOR:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    eor %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]),
                               format_aarch64_operand(&inst->operands[2]));
            }
            break;
            
        case INST_SHL:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    lsl %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]),
                               format_aarch64_operand(&inst->operands[2]));
            }
            break;
            
        case INST_SHR:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    lsr %s, %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]),
                               format_aarch64_operand(&inst->operands[2]));
            }
            break;
            
        case INST_CMP:
            if (inst->operand_count == 2) {
                return snprintf(buffer, buffer_size, "    cmp %s, %s\n",
                               format_aarch64_operand(&inst->operands[0]),
                               format_aarch64_operand(&inst->operands[1]));
            }
            break;
            
        case INST_JMP:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    b %s\n",
                               format_aarch64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_JE:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    b.eq %s\n",
                               format_aarch64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_JNE:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    b.ne %s\n",
                               format_aarch64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_JL:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    b.lt %s\n",
                               format_aarch64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_JLE:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    b.le %s\n",
                               format_aarch64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_JG:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    b.gt %s\n",
                               format_aarch64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_JGE:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    b.ge %s\n",
                               format_aarch64_operand(&inst->operands[0]));
            }
            break;
            
        case INST_CALL:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    bl %s\n",
                               format_aarch64_operand(&inst->operands[0]));
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
            
        // Floating point instructions
        case INST_ADDSS:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    fadd s%d, s%d, s%d\n",
                               (int)inst->operands[0].data.reg - ASTHRA_REG_XMM0,
                               (int)inst->operands[1].data.reg - ASTHRA_REG_XMM0,
                               (int)inst->operands[2].data.reg - ASTHRA_REG_XMM0);
            }
            break;
            
        case INST_ADDSD:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    fadd d%d, d%d, d%d\n",
                               (int)inst->operands[0].data.reg - ASTHRA_REG_XMM0,
                               (int)inst->operands[1].data.reg - ASTHRA_REG_XMM0,
                               (int)inst->operands[2].data.reg - ASTHRA_REG_XMM0);
            }
            break;
            
        case INST_SUBSS:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    fsub s%d, s%d, s%d\n",
                               (int)inst->operands[0].data.reg - ASTHRA_REG_XMM0,
                               (int)inst->operands[1].data.reg - ASTHRA_REG_XMM0,
                               (int)inst->operands[2].data.reg - ASTHRA_REG_XMM0);
            }
            break;
            
        case INST_SUBSD:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    fsub d%d, d%d, d%d\n",
                               (int)inst->operands[0].data.reg - ASTHRA_REG_XMM0,
                               (int)inst->operands[1].data.reg - ASTHRA_REG_XMM0,
                               (int)inst->operands[2].data.reg - ASTHRA_REG_XMM0);
            }
            break;
            
        default:
            return snprintf(buffer, buffer_size, "    ; ARM64 instruction type %d\n", inst->type);
    }
    
    return snprintf(buffer, buffer_size, "    ; Invalid ARM64 instruction\n");
}

/**
 * Format x86_64 operand with syntax awareness
 */
static const char* format_x86_64_operand(const CodeGenerator *generator, const AssemblyOperand *operand) {
    static char operand_buffer[64];
    
    if (!generator || !operand) return "<?>";
    
    bool is_intel = (generator->asm_syntax == ASTHRA_ASM_SYNTAX_INTEL);
    
    switch (operand->type) {
        case OPERAND_REGISTER:
            if (is_intel) {
                // Intel syntax: no % prefix for registers
                return get_x86_64_register_name(operand->data.reg) + 1; // Skip '%' prefix
            } else {
                // AT&T syntax: registers prefixed with %
                return get_x86_64_register_name(operand->data.reg);
            }
        case OPERAND_IMMEDIATE:
            if (is_intel) {
                // Intel syntax: no $ prefix for immediates
                snprintf(operand_buffer, sizeof(operand_buffer), "%lld", 
                        (long long)operand->data.immediate);
            } else {
                // AT&T syntax: immediates prefixed with $
                snprintf(operand_buffer, sizeof(operand_buffer), "$%lld", 
                        (long long)operand->data.immediate);
            }
            return operand_buffer;
        case OPERAND_MEMORY:
            if (is_intel) {
                // Intel syntax: [base + displacement]
                snprintf(operand_buffer, sizeof(operand_buffer), "[%s + %d]", 
                        get_x86_64_register_name(operand->data.memory.base) + 1,
                        operand->data.memory.displacement);
            } else {
                // AT&T syntax: displacement(%base)
                snprintf(operand_buffer, sizeof(operand_buffer), "%d(%s)", 
                        operand->data.memory.displacement,
                        get_x86_64_register_name(operand->data.memory.base));
            }
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
 * Get ARM64 register name with proper mapping from x86_64 register enum
 */
static const char* get_aarch64_register_name(Register reg) {
    // Map x86_64 register enum to ARM64 equivalents
    switch (reg) {
        // Integer registers - use x86_64 to ARM64 mapping
        case ASTHRA_REG_RAX: return "x0";   // Return value/first argument
        case ASTHRA_REG_RCX: return "x1";   // Second argument
        case ASTHRA_REG_RDX: return "x2";   // Third argument
        case ASTHRA_REG_RBX: return "x3";   // Fourth argument
        case ASTHRA_REG_RSP: return "sp";   // Stack pointer
        case ASTHRA_REG_RBP: return "x29";  // Frame pointer
        case ASTHRA_REG_RSI: return "x4";   // Fifth argument
        case ASTHRA_REG_RDI: return "x5";   // Sixth argument
        case ASTHRA_REG_R8:  return "x6";   // Seventh argument
        case ASTHRA_REG_R9:  return "x7";   // Eighth argument
        case ASTHRA_REG_R10: return "x8";   // Caller-saved
        case ASTHRA_REG_R11: return "x9";   // Caller-saved
        case ASTHRA_REG_R12: return "x10";  // Caller-saved
        case ASTHRA_REG_R13: return "x11";  // Caller-saved
        case ASTHRA_REG_R14: return "x12";  // Caller-saved
        case ASTHRA_REG_R15: return "x13";  // Caller-saved
        
        // SIMD/FP registers - use d prefix for double precision by default
        case ASTHRA_REG_XMM0:  return "d0";
        case ASTHRA_REG_XMM1:  return "d1";
        case ASTHRA_REG_XMM2:  return "d2";
        case ASTHRA_REG_XMM3:  return "d3";
        case ASTHRA_REG_XMM4:  return "d4";
        case ASTHRA_REG_XMM5:  return "d5";
        case ASTHRA_REG_XMM6:  return "d6";
        case ASTHRA_REG_XMM7:  return "d7";
        case ASTHRA_REG_XMM8:  return "d8";
        case ASTHRA_REG_XMM9:  return "d9";
        case ASTHRA_REG_XMM10: return "d10";
        case ASTHRA_REG_XMM11: return "d11";
        case ASTHRA_REG_XMM12: return "d12";
        case ASTHRA_REG_XMM13: return "d13";
        case ASTHRA_REG_XMM14: return "d14";
        case ASTHRA_REG_XMM15: return "d15";
        
        default: return "x0"; // Default fallback
    }
}

/**
 * Format WebAssembly text format (WAT) instruction
 */
static int format_wasm_instruction(const AssemblyInstruction *inst, 
                                  char *buffer, size_t buffer_size) {
    if (!inst || !buffer) return -1;
    
    switch (inst->type) {
        case INST_MOV:
            // In WASM, use local.set for register assignments
            if (inst->operand_count == 2) {
                return snprintf(buffer, buffer_size, "    %s\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_ADD:
            if (inst->operand_count == 2) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    %s\n    i32.add\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[0]),
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[0]));
            } else if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    i32.add\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_SUB:
            if (inst->operand_count == 2) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    %s\n    i32.sub\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[0]),
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[0]));
            } else if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    i32.sub\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_MUL:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    i32.mul\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_DIV:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    i32.div_u\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_AND:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    i32.and\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_OR:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    i32.or\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_XOR:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    i32.xor\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_SHL:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    i32.shl\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_SHR:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    i32.shr_u\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_CMP:
            if (inst->operand_count == 2) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    i32.eq\n",
                               format_wasm_operand(&inst->operands[0]),
                               format_wasm_operand(&inst->operands[1]));
            }
            break;
            
        case INST_JMP:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    br %s\n",
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_JE:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    br_if %s\n",
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_CALL:
            if (inst->operand_count == 1) {
                return snprintf(buffer, buffer_size, "    call %s\n",
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_RET:
            return snprintf(buffer, buffer_size, "    return\n");
            
        // Floating point instructions
        case INST_ADDSS:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    f32.add\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_ADDSD:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    f64.add\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_SUBSS:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    f32.sub\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        case INST_SUBSD:
            if (inst->operand_count == 3) {
                return snprintf(buffer, buffer_size, "    local.get %s\n    local.get %s\n    f64.sub\n    local.set %s\n",
                               format_wasm_operand(&inst->operands[1]),
                               format_wasm_operand(&inst->operands[2]),
                               format_wasm_operand(&inst->operands[0]));
            }
            break;
            
        default:
            return snprintf(buffer, buffer_size, "    ;; WASM instruction type %d\n", inst->type);
    }
    
    return snprintf(buffer, buffer_size, "    ;; Invalid WASM instruction\n");
}

/**
 * Format WebAssembly operand
 */
static const char* format_wasm_operand(const AssemblyOperand *operand) {
    static char operand_buffer[64];
    
    if (!operand) return "$0";
    
    switch (operand->type) {
        case OPERAND_REGISTER:
            // Map registers to local variable indices
            snprintf(operand_buffer, sizeof(operand_buffer), "$local%d", (int)operand->data.reg);
            return operand_buffer;
        case OPERAND_IMMEDIATE:
            snprintf(operand_buffer, sizeof(operand_buffer), "i32.const %lld", 
                    (long long)operand->data.immediate);
            return operand_buffer;
        case OPERAND_MEMORY:
            // WebAssembly uses linear memory with offsets
            snprintf(operand_buffer, sizeof(operand_buffer), "i32.load offset=%d", 
                    operand->data.memory.displacement);
            return operand_buffer;
        case OPERAND_LABEL:
            return operand->data.label ? operand->data.label : "$label0";
        default:
            return "$0";
    }
}