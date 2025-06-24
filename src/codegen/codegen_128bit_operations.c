/**
 * Asthra Programming Language Compiler
 * 128-bit Integer Code Generation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file implements code generation for 128-bit integer operations (i128/u128).
 * These operations require special handling as they exceed the native register size
 * on most platforms and need multi-instruction sequences.
 */

#include "codegen_128bit_operations.h"
#include "code_generator.h"
#include "code_generator_instructions.h"
#include "register_allocator.h"
#include <string.h>

// =============================================================================
// 128-BIT REGISTER ALLOCATION
// =============================================================================

/**
 * Represents a 128-bit value using two 64-bit registers
 */
typedef struct {
    Register low;   // Lower 64 bits
    Register high;  // Upper 64 bits
} Register128;

/**
 * Allocate a register pair for 128-bit operations
 */
static Register128 allocate_128bit_register(CodeGenerator *generator) {
    Register128 result;
    result.low = register_allocate(generator->register_allocator, true);
    result.high = register_allocate(generator->register_allocator, true);
    return result;
}

/**
 * Free a 128-bit register pair
 */
static void free_128bit_register(CodeGenerator *generator, Register128 reg) {
    register_free(generator->register_allocator, reg.low);
    register_free(generator->register_allocator, reg.high);
}

// =============================================================================
// 128-BIT LOAD/STORE OPERATIONS
// =============================================================================

bool generate_128bit_load(CodeGenerator *generator, const char *var_name, Register128 dest) {
    if (!generator || !var_name) return false;
    
    // Get variable location
    LocalVariable *var = find_local_variable(generator, var_name);
    if (!var) return false;
    
    int32_t stack_offset = var->offset;
    
    // Load low 64 bits
    AssemblyInstruction *load_low = create_instruction(INST_MOV, 2);
    if (!load_low) return false;
    load_low->operands[0] = create_register_operand(dest.low);
    load_low->operands[1] = create_memory_operand(ASTHRA_REG_RBP, ASTHRA_REG_NONE, 1, -stack_offset);
    
    if (!instruction_buffer_add(generator->instruction_buffer, load_low)) {
        return false;
    }
    
    // Load high 64 bits (8 bytes above low part)
    AssemblyInstruction *load_high = create_instruction(INST_MOV, 2);
    if (!load_high) return false;
    load_high->operands[0] = create_register_operand(dest.high);
    load_high->operands[1] = create_memory_operand(ASTHRA_REG_RBP, ASTHRA_REG_NONE, 1, -stack_offset + 8);
    
    if (!instruction_buffer_add(generator->instruction_buffer, load_high)) {
        return false;
    }
    
    return true;
}

bool generate_128bit_store(CodeGenerator *generator, Register128 src, const char *var_name) {
    if (!generator || !var_name) return false;
    
    // Get variable location
    LocalVariable *var = find_local_variable(generator, var_name);
    if (!var) return false;
    
    int32_t stack_offset = var->offset;
    
    // Store low 64 bits
    AssemblyInstruction *store_low = create_instruction(INST_MOVQ, 2);
    if (!store_low) return false;
    store_low->operands[0] = create_memory_operand(ASTHRA_REG_RBP, ASTHRA_REG_NONE, 1, -stack_offset);
    store_low->operands[1] = create_register_operand(src.low);
    
    if (!instruction_buffer_add(generator->instruction_buffer, store_low)) {
        return false;
    }
    
    // Store high 64 bits (8 bytes above low part)
    AssemblyInstruction *store_high = create_instruction(INST_MOVQ, 2);
    if (!store_high) return false;
    store_high->operands[0] = create_memory_operand(ASTHRA_REG_RBP, ASTHRA_REG_NONE, 1, -stack_offset + 8);
    store_high->operands[1] = create_register_operand(src.high);
    
    if (!instruction_buffer_add(generator->instruction_buffer, store_high)) {
        return false;
    }
    
    return true;
}

// =============================================================================
// 128-BIT ARITHMETIC OPERATIONS
// =============================================================================

bool generate_128bit_add(CodeGenerator *generator, Register128 left, Register128 right, Register128 result) {
    if (!generator) return false;
    
    // Move left to result if different
    if (result.low != left.low) {
        AssemblyInstruction *mov_low = create_mov_instruction(result.low, left.low);
        if (!mov_low || !instruction_buffer_add(generator->instruction_buffer, mov_low)) {
            return false;
        }
    }
    if (result.high != left.high) {
        AssemblyInstruction *mov_high = create_mov_instruction(result.high, left.high);
        if (!mov_high || !instruction_buffer_add(generator->instruction_buffer, mov_high)) {
            return false;
        }
    }
    
    // Add low parts: result.low = left.low + right.low
    AssemblyInstruction *add_low = create_instruction(INST_ADD, 2);
    if (!add_low) return false;
    add_low->operands[0] = create_register_operand(result.low);
    add_low->operands[1] = create_register_operand(right.low);
    
    if (!instruction_buffer_add(generator->instruction_buffer, add_low)) {
        return false;
    }
    
    // Add high parts with carry: result.high = left.high + right.high + carry
    AssemblyInstruction *adc_high = create_instruction(INST_ADC, 2);
    if (!adc_high) return false;
    adc_high->operands[0] = create_register_operand(result.high);
    adc_high->operands[1] = create_register_operand(right.high);
    
    if (!instruction_buffer_add(generator->instruction_buffer, adc_high)) {
        return false;
    }
    
    return true;
}

bool generate_128bit_sub(CodeGenerator *generator, Register128 left, Register128 right, Register128 result) {
    if (!generator) return false;
    
    // Move left to result if different
    if (result.low != left.low) {
        AssemblyInstruction *mov_low = create_mov_instruction(result.low, left.low);
        if (!mov_low || !instruction_buffer_add(generator->instruction_buffer, mov_low)) {
            return false;
        }
    }
    if (result.high != left.high) {
        AssemblyInstruction *mov_high = create_mov_instruction(result.high, left.high);
        if (!mov_high || !instruction_buffer_add(generator->instruction_buffer, mov_high)) {
            return false;
        }
    }
    
    // Subtract low parts: result.low = left.low - right.low
    AssemblyInstruction *sub_low = create_instruction(INST_SUB, 2);
    if (!sub_low) return false;
    sub_low->operands[0] = create_register_operand(result.low);
    sub_low->operands[1] = create_register_operand(right.low);
    
    if (!instruction_buffer_add(generator->instruction_buffer, sub_low)) {
        return false;
    }
    
    // Subtract high parts with borrow: result.high = left.high - right.high - borrow
    AssemblyInstruction *sbb_high = create_instruction(INST_SBB, 2);
    if (!sbb_high) return false;
    sbb_high->operands[0] = create_register_operand(result.high);
    sbb_high->operands[1] = create_register_operand(right.high);
    
    if (!instruction_buffer_add(generator->instruction_buffer, sbb_high)) {
        return false;
    }
    
    return true;
}

bool generate_128bit_bitwise_and(CodeGenerator *generator, Register128 left, Register128 right, Register128 result) {
    if (!generator) return false;
    
    // Move left to result if different
    if (result.low != left.low) {
        AssemblyInstruction *mov_low = create_mov_instruction(result.low, left.low);
        if (!mov_low || !instruction_buffer_add(generator->instruction_buffer, mov_low)) {
            return false;
        }
    }
    if (result.high != left.high) {
        AssemblyInstruction *mov_high = create_mov_instruction(result.high, left.high);
        if (!mov_high || !instruction_buffer_add(generator->instruction_buffer, mov_high)) {
            return false;
        }
    }
    
    // AND low parts
    AssemblyInstruction *and_low = create_instruction(INST_AND, 2);
    if (!and_low) return false;
    and_low->operands[0] = create_register_operand(result.low);
    and_low->operands[1] = create_register_operand(right.low);
    
    if (!instruction_buffer_add(generator->instruction_buffer, and_low)) {
        return false;
    }
    
    // AND high parts
    AssemblyInstruction *and_high = create_instruction(INST_AND, 2);
    if (!and_high) return false;
    and_high->operands[0] = create_register_operand(result.high);
    and_high->operands[1] = create_register_operand(right.high);
    
    if (!instruction_buffer_add(generator->instruction_buffer, and_high)) {
        return false;
    }
    
    return true;
}

bool generate_128bit_bitwise_or(CodeGenerator *generator, Register128 left, Register128 right, Register128 result) {
    if (!generator) return false;
    
    // Move left to result if different
    if (result.low != left.low) {
        AssemblyInstruction *mov_low = create_mov_instruction(result.low, left.low);
        if (!mov_low || !instruction_buffer_add(generator->instruction_buffer, mov_low)) {
            return false;
        }
    }
    if (result.high != left.high) {
        AssemblyInstruction *mov_high = create_mov_instruction(result.high, left.high);
        if (!mov_high || !instruction_buffer_add(generator->instruction_buffer, mov_high)) {
            return false;
        }
    }
    
    // OR low parts
    AssemblyInstruction *or_low = create_instruction(INST_OR, 2);
    if (!or_low) return false;
    or_low->operands[0] = create_register_operand(result.low);
    or_low->operands[1] = create_register_operand(right.low);
    
    if (!instruction_buffer_add(generator->instruction_buffer, or_low)) {
        return false;
    }
    
    // OR high parts
    AssemblyInstruction *or_high = create_instruction(INST_OR, 2);
    if (!or_high) return false;
    or_high->operands[0] = create_register_operand(result.high);
    or_high->operands[1] = create_register_operand(right.high);
    
    if (!instruction_buffer_add(generator->instruction_buffer, or_high)) {
        return false;
    }
    
    return true;
}

bool generate_128bit_bitwise_xor(CodeGenerator *generator, Register128 left, Register128 right, Register128 result) {
    if (!generator) return false;
    
    // Move left to result if different
    if (result.low != left.low) {
        AssemblyInstruction *mov_low = create_mov_instruction(result.low, left.low);
        if (!mov_low || !instruction_buffer_add(generator->instruction_buffer, mov_low)) {
            return false;
        }
    }
    if (result.high != left.high) {
        AssemblyInstruction *mov_high = create_mov_instruction(result.high, left.high);
        if (!mov_high || !instruction_buffer_add(generator->instruction_buffer, mov_high)) {
            return false;
        }
    }
    
    // XOR low parts
    AssemblyInstruction *xor_low = create_instruction(INST_XOR, 2);
    if (!xor_low) return false;
    xor_low->operands[0] = create_register_operand(result.low);
    xor_low->operands[1] = create_register_operand(right.low);
    
    if (!instruction_buffer_add(generator->instruction_buffer, xor_low)) {
        return false;
    }
    
    // XOR high parts
    AssemblyInstruction *xor_high = create_instruction(INST_XOR, 2);
    if (!xor_high) return false;
    xor_high->operands[0] = create_register_operand(result.high);
    xor_high->operands[1] = create_register_operand(right.high);
    
    if (!instruction_buffer_add(generator->instruction_buffer, xor_high)) {
        return false;
    }
    
    return true;
}

// =============================================================================
// 128-BIT COMPARISON OPERATIONS
// =============================================================================

bool generate_128bit_compare(CodeGenerator *generator, Register128 left, Register128 right, 
                            BinaryOperator op, Register result) {
    if (!generator) return false;
    
    // First compare high parts
    AssemblyInstruction *cmp_high = create_instruction(INST_CMP, 2);
    if (!cmp_high) return false;
    cmp_high->operands[0] = create_register_operand(left.high);
    cmp_high->operands[1] = create_register_operand(right.high);
    
    if (!instruction_buffer_add(generator->instruction_buffer, cmp_high)) {
        return false;
    }
    
    // Create labels for the comparison logic
    const char *check_low_label = label_manager_create_label(generator->label_manager, 
                                                           LABEL_BRANCH_TARGET, "cmp128_check_low");
    const char *done_label = label_manager_create_label(generator->label_manager,
                                                       LABEL_BRANCH_TARGET, "cmp128_done");
    
    // If high parts are not equal, we have our result
    AssemblyInstruction *jne_inst = create_jump_instruction(INST_JNE, check_low_label);
    if (!jne_inst || !instruction_buffer_add(generator->instruction_buffer, jne_inst)) {
        return false;
    }
    
    // High parts are equal, compare low parts
    AssemblyInstruction *cmp_low = create_instruction(INST_CMP, 2);
    if (!cmp_low) return false;
    cmp_low->operands[0] = create_register_operand(left.low);
    cmp_low->operands[1] = create_register_operand(right.low);
    
    if (!instruction_buffer_add(generator->instruction_buffer, cmp_low)) {
        return false;
    }
    
    // Set result based on the comparison
    ConditionCode cond;
    switch (op) {
        case BINOP_EQ:  cond = COND_E; break;
        case BINOP_NE:  cond = COND_NE; break;
        case BINOP_LT:  cond = COND_B; break;  // Use unsigned comparison for low parts
        case BINOP_LE:  cond = COND_BE; break;
        case BINOP_GT:  cond = COND_A; break;
        case BINOP_GE:  cond = COND_AE; break;
        default: return false;
    }
    
    AssemblyInstruction *setcc_inst = create_setcc_instruction(cond, result);
    if (!setcc_inst || !instruction_buffer_add(generator->instruction_buffer, setcc_inst)) {
        return false;
    }
    
    // Jump to done
    AssemblyInstruction *jmp_done = create_jump_instruction(INST_JMP, done_label);
    if (!jmp_done || !instruction_buffer_add(generator->instruction_buffer, jmp_done)) {
        return false;
    }
    
    // Label for checking low parts
    label_manager_define_label(generator->label_manager, check_low_label, 
                              generator->instruction_buffer->count);
    
    // Set result based on high parts comparison
    AssemblyInstruction *setcc_high = create_setcc_instruction(cond, result);
    if (!setcc_high || !instruction_buffer_add(generator->instruction_buffer, setcc_high)) {
        return false;
    }
    
    // Done label
    label_manager_define_label(generator->label_manager, done_label,
                              generator->instruction_buffer->count);
    
    return true;
}

// =============================================================================
// 128-BIT MULTIPLICATION (SIMPLIFIED)
// =============================================================================

bool generate_128bit_mul(CodeGenerator *generator, Register128 left, Register128 right, Register128 result) {
    if (!generator) return false;
    
    // 128-bit multiplication: (a*2^64 + b) * (c*2^64 + d) = 
    // a*c*2^128 + a*d*2^64 + b*c*2^64 + b*d
    // We ignore the overflow (a*c*2^128) for now
    
    // result.low = left.low * right.low (low 64 bits)
    AssemblyInstruction *mul_low = create_instruction(INST_MUL, 2);
    if (!mul_low) return false;
    mul_low->operands[0] = create_register_operand(result.low);
    mul_low->operands[1] = create_register_operand(right.low);
    
    if (!instruction_buffer_add(generator->instruction_buffer, mul_low)) {
        return false;
    }
    
    // Allocate temporary registers for intermediate results
    Register temp1 = register_allocate(generator->register_allocator, true);
    Register temp2 = register_allocate(generator->register_allocator, true);
    
    // temp1 = left.low * right.high
    AssemblyInstruction *mov_temp1 = create_mov_instruction(temp1, left.low);
    if (!mov_temp1 || !instruction_buffer_add(generator->instruction_buffer, mov_temp1)) {
        register_free(generator->register_allocator, temp1);
        register_free(generator->register_allocator, temp2);
        return false;
    }
    
    AssemblyInstruction *mul_temp1 = create_instruction(INST_IMUL, 2);
    if (!mul_temp1) {
        register_free(generator->register_allocator, temp1);
        register_free(generator->register_allocator, temp2);
        return false;
    }
    mul_temp1->operands[0] = create_register_operand(temp1);
    mul_temp1->operands[1] = create_register_operand(right.high);
    
    if (!instruction_buffer_add(generator->instruction_buffer, mul_temp1)) {
        register_free(generator->register_allocator, temp1);
        register_free(generator->register_allocator, temp2);
        return false;
    }
    
    // temp2 = left.high * right.low
    AssemblyInstruction *mov_temp2 = create_mov_instruction(temp2, left.high);
    if (!mov_temp2 || !instruction_buffer_add(generator->instruction_buffer, mov_temp2)) {
        register_free(generator->register_allocator, temp1);
        register_free(generator->register_allocator, temp2);
        return false;
    }
    
    AssemblyInstruction *mul_temp2 = create_instruction(INST_IMUL, 2);
    if (!mul_temp2) {
        register_free(generator->register_allocator, temp1);
        register_free(generator->register_allocator, temp2);
        return false;
    }
    mul_temp2->operands[0] = create_register_operand(temp2);
    mul_temp2->operands[1] = create_register_operand(right.low);
    
    if (!instruction_buffer_add(generator->instruction_buffer, mul_temp2)) {
        register_free(generator->register_allocator, temp1);
        register_free(generator->register_allocator, temp2);
        return false;
    }
    
    // result.high = temp1 + temp2 + (high part of low multiplication)
    AssemblyInstruction *add_temps = create_instruction(INST_ADD, 2);
    if (!add_temps) {
        register_free(generator->register_allocator, temp1);
        register_free(generator->register_allocator, temp2);
        return false;
    }
    add_temps->operands[0] = create_register_operand(temp1);
    add_temps->operands[1] = create_register_operand(temp2);
    
    if (!instruction_buffer_add(generator->instruction_buffer, add_temps)) {
        register_free(generator->register_allocator, temp1);
        register_free(generator->register_allocator, temp2);
        return false;
    }
    
    // Move result to high register
    AssemblyInstruction *mov_high = create_mov_instruction(result.high, temp1);
    if (!mov_high || !instruction_buffer_add(generator->instruction_buffer, mov_high)) {
        register_free(generator->register_allocator, temp1);
        register_free(generator->register_allocator, temp2);
        return false;
    }
    
    register_free(generator->register_allocator, temp1);
    register_free(generator->register_allocator, temp2);
    
    return true;
}

// =============================================================================
// HIGH-LEVEL 128-BIT OPERATION INTERFACE
// =============================================================================

bool is_128bit_type(TypeDescriptor *type) {
    if (!type || type->category != TYPE_PRIMITIVE) {
        return false;
    }
    
    return type->data.primitive.primitive_kind == PRIMITIVE_I128 ||
           type->data.primitive.primitive_kind == PRIMITIVE_U128;
}

bool generate_128bit_binary_operation(CodeGenerator *generator, BinaryOperator op,
                                     TypeDescriptor *type, Register left, Register right, Register result) {
    if (!generator || !is_128bit_type(type)) {
        return false;
    }
    
    // For 128-bit operations, we need to work with register pairs
    // The left and right registers are actually pointers to 128-bit values
    // We need to load them into register pairs
    
    Register128 left_128 = allocate_128bit_register(generator);
    Register128 right_128 = allocate_128bit_register(generator);
    Register128 result_128 = allocate_128bit_register(generator);
    
    // For now, we'll implement basic operations
    bool success = false;
    
    switch (op) {
        case BINOP_ADD:
            success = generate_128bit_add(generator, left_128, right_128, result_128);
            break;
            
        case BINOP_SUB:
            success = generate_128bit_sub(generator, left_128, right_128, result_128);
            break;
            
        case BINOP_MUL:
            success = generate_128bit_mul(generator, left_128, right_128, result_128);
            break;
            
        case BINOP_BITWISE_AND:
            success = generate_128bit_bitwise_and(generator, left_128, right_128, result_128);
            break;
            
        case BINOP_BITWISE_OR:
            success = generate_128bit_bitwise_or(generator, left_128, right_128, result_128);
            break;
            
        case BINOP_BITWISE_XOR:
            success = generate_128bit_bitwise_xor(generator, left_128, right_128, result_128);
            break;
            
        case BINOP_EQ:
        case BINOP_NE:
        case BINOP_LT:
        case BINOP_LE:
        case BINOP_GT:
        case BINOP_GE:
            success = generate_128bit_compare(generator, left_128, right_128, op, result);
            break;
            
        default:
            // Division and modulo would require more complex implementations
            // For now, we'll leave them unimplemented
            success = false;
            break;
    }
    
    // Free the allocated register pairs
    free_128bit_register(generator, left_128);
    free_128bit_register(generator, right_128);
    free_128bit_register(generator, result_128);
    
    return success;
}