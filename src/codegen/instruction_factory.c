/**
 * Asthra Programming Language Compiler
 * Assembly Instruction Factory Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "code_generator.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// =============================================================================
// INSTRUCTION CREATION WITH C17 FEATURES
// =============================================================================

AssemblyInstruction *create_instruction(InstructionType type, size_t operand_count, ...) {
    // Allocate instruction with flexible array member
    size_t size = sizeof(AssemblyInstruction) + operand_count * sizeof(AssemblyOperand);
    AssemblyInstruction *inst = malloc(size);
    if (!inst) return NULL;
    
    inst->type = type;
    inst->operand_count = operand_count;
    inst->branch_hint = BRANCH_HINT_NONE;
    inst->comment = NULL;
    
    // Process variadic arguments for operands
    va_list args;
    va_start(args, operand_count);
    for (size_t i = 0; i < operand_count; i++) {
        inst->operands[i] = va_arg(args, AssemblyOperand);
    }
    va_end(args);
    
    return inst;
}

// Non-variadic version for helper functions that set operands manually
AssemblyInstruction *create_instruction_empty(InstructionType type, size_t operand_count) {
    // Allocate instruction with flexible array member
    size_t size = sizeof(AssemblyInstruction) + operand_count * sizeof(AssemblyOperand);
    AssemblyInstruction *inst = malloc(size);
    if (!inst) return NULL;
    
    inst->type = type;
    inst->operand_count = operand_count;
    inst->branch_hint = BRANCH_HINT_NONE;
    inst->comment = NULL;
    
    // Initialize operands to zero
    for (size_t i = 0; i < operand_count; i++) {
        memset(&inst->operands[i], 0, sizeof(AssemblyOperand));
    }
    
    return inst;
}

AssemblyInstruction *create_mov_instruction(Register dest, Register src) {
    // C17 compound literal for operands
    return create_instruction(INST_MOV, 2,
        (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = dest},
        (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = src}
    );
}

AssemblyInstruction *create_mov_immediate(Register dest, int64_t immediate) {
    return create_instruction(INST_MOV, 2,
        (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = dest},
        (AssemblyOperand){.type = OPERAND_IMMEDIATE, .data.immediate = immediate}
    );
}

AssemblyInstruction *create_call_instruction(const char *function_name) {
    char *name_copy = strdup(function_name);
    if (!name_copy) return NULL;
    
    return create_instruction(INST_CALL, 1,
        (AssemblyOperand){.type = OPERAND_LABEL, .data.label = name_copy}
    );
}

AssemblyInstruction *create_ret_instruction(void) {
    return create_instruction(INST_RET, 0);
}

AssemblyInstruction *create_jmp_instruction(const char *label) {
    char *label_copy = strdup(label);
    if (!label_copy) return NULL;
    
    return create_instruction(INST_JMP, 1,
        (AssemblyOperand){.type = OPERAND_LABEL, .data.label = label_copy}
    );
}

AssemblyInstruction *create_je_instruction(const char *label) {
    char *label_copy = strdup(label);
    if (!label_copy) return NULL;
    
    return create_instruction(INST_JE, 1,
        (AssemblyOperand){.type = OPERAND_LABEL, .data.label = label_copy}
    );
}

AssemblyInstruction *create_cmp_instruction(Register reg1, Register reg2) {
    return create_instruction(INST_CMP, 2,
        (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = reg1},
        (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = reg2}
    );
}

AssemblyInstruction *create_load_instruction(Register dest_reg, Register base_reg, int32_t offset) {
    return create_instruction(INST_MOV, 2,
        (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = dest_reg},
        (AssemblyOperand){.type = OPERAND_MEMORY, .data.memory = {
            .base = base_reg,
            .index = ASTHRA_REG_NONE,
            .scale = 1,
            .displacement = offset
        }}
    );
}

AssemblyInstruction *create_inc_instruction(Register reg) {
    return create_instruction(INST_INC, 1,
        (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = reg}
    );
}

// =============================================================================
// OPERAND CREATION HELPERS
// =============================================================================

AssemblyOperand create_register_operand(Register reg) {
    return (AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = reg};
}

AssemblyOperand create_immediate_operand(int64_t value) {
    return (AssemblyOperand){.type = OPERAND_IMMEDIATE, .data.immediate = value};
}

AssemblyOperand create_memory_operand(Register base, Register index, uint8_t scale, int32_t displacement) {
    return (AssemblyOperand){
        .type = OPERAND_MEMORY,
        .data.memory = {
            .base = base,
            .index = index,
            .scale = scale,
            .displacement = displacement
        }
    };
}

AssemblyOperand create_label_operand(const char *label) {
    char *label_copy = strdup(label);
    return (AssemblyOperand){.type = OPERAND_LABEL, .data.label = label_copy};
}

// =============================================================================
// ASSEMBLY INSTRUCTION CREATION HELPERS
// =============================================================================

AssemblyInstruction *create_store_local(Register src_reg, int32_t offset) {
    AssemblyInstruction *inst = create_instruction_empty(INST_MOV, 2);
    if (!inst) return NULL;
    
    inst->operands[0] = create_memory_operand(ASTHRA_REG_RBP, ASTHRA_REG_NONE, 1, offset);
    inst->operands[1] = create_register_operand(src_reg);
    
    return inst;
}

AssemblyInstruction *create_load_local(Register dest_reg, int32_t offset) {
    AssemblyInstruction *inst = create_instruction_empty(INST_MOV, 2);
    if (!inst) return NULL;
    
    inst->operands[0] = create_register_operand(dest_reg);
    inst->operands[1] = create_memory_operand(ASTHRA_REG_RBP, ASTHRA_REG_NONE, 1, offset);
    
    return inst;
}

AssemblyInstruction *create_lea_local(Register dest_reg, int32_t offset) {
    AssemblyInstruction *inst = create_instruction_empty(INST_LEA, 2);
    if (!inst) return NULL;
    
    inst->operands[0] = create_register_operand(dest_reg);
    inst->operands[1] = create_memory_operand(ASTHRA_REG_RBP, ASTHRA_REG_NONE, 1, offset);
    
    return inst;
}

AssemblyInstruction *create_store_indirect(Register src_reg, Register base_reg, int32_t offset) {
    AssemblyInstruction *inst = create_instruction_empty(INST_MOV, 2);
    if (!inst) return NULL;
    
    inst->operands[0] = create_memory_operand(base_reg, ASTHRA_REG_NONE, 1, offset);
    inst->operands[1] = create_register_operand(src_reg);
    
    return inst;
}

AssemblyInstruction *create_load_indirect(Register dest_reg, Register base_reg, int32_t offset) {
    AssemblyInstruction *inst = create_instruction_empty(INST_MOV, 2);
    if (!inst) return NULL;
    
    inst->operands[0] = create_register_operand(dest_reg);
    inst->operands[1] = create_memory_operand(base_reg, ASTHRA_REG_NONE, 1, offset);
    
    return inst;
}

AssemblyInstruction *create_binary_op(BinaryOperator op, Register left_reg, Register right_reg, int64_t immediate, Register dest_reg) {
    InstructionType inst_type;
    
    switch (op) {
        case BINOP_ADD: inst_type = INST_ADD; break;
        case BINOP_SUB: inst_type = INST_SUB; break;
        case BINOP_MUL: inst_type = INST_IMUL; break;
        default: return NULL;
    }
    
    AssemblyInstruction *inst = create_instruction_empty(inst_type, 2);
    if (!inst) return NULL;
    
    inst->operands[0] = create_register_operand(dest_reg);
    if (immediate != 0) {
        inst->operands[1] = create_immediate_operand(immediate);
    } else {
        inst->operands[1] = create_register_operand(right_reg);
    }
    
    return inst;
} 
