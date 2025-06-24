/**
 * Asthra Programming Language Compiler
 * Code Generation Instructions and Assembly Support
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CODE_GENERATOR_INSTRUCTIONS_H
#define ASTHRA_CODE_GENERATOR_INSTRUCTIONS_H

#include "code_generator_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// INSTRUCTION REPRESENTATION WITH C17 FEATURES
// =============================================================================

// Assembly instruction types
typedef enum {
    // Data movement
    INST_MOV, INST_MOVSX, INST_MOVZX, INST_LEA,
    
    // Arithmetic
    INST_ADD, INST_SUB, INST_MUL, INST_IMUL, INST_DIV, INST_IDIV,
    INST_INC, INST_DEC, INST_NEG,
    
    // Logical
    INST_AND, INST_OR, INST_XOR, INST_NOT, INST_SHL, INST_SHR, INST_SAR,
    
    // Comparison and control flow
    INST_CMP, INST_TEST, INST_JMP, INST_JE, INST_JNE, INST_JL, INST_JLE,
    INST_JG, INST_JGE, INST_JA, INST_JAE, INST_JB, INST_JBE,
    INST_JZ, INST_JNZ, INST_JS, INST_JNS, INST_JO, INST_JNO,
    INST_CALL, INST_RET,
    
    // Set condition codes
    INST_SETE, INST_SETNE, INST_SETL, INST_SETLE, INST_SETG, INST_SETGE,
    INST_SETA, INST_SETAE, INST_SETB, INST_SETBE, INST_SETZ, INST_SETNZ,
    
    // Stack operations
    INST_PUSH, INST_POP,
    
    // Floating point
    INST_MOVSS, INST_MOVSD, INST_ADDSS, INST_ADDSD, INST_SUBSS, INST_SUBSD,
    INST_MULSS, INST_MULSD, INST_DIVSS, INST_DIVSD,
    INST_COMISS, INST_COMISD, INST_CVTSI2SS, INST_CVTSI2SD,
    
    // Memory operations
    INST_MOVQ, INST_MOVL, INST_MOVW, INST_MOVB,
    
    // 128-bit operations
    INST_MOVDQA,  // Move aligned 128-bit
    INST_MOVDQU,  // Move unaligned 128-bit
    INST_ADC,     // Add with carry
    INST_SBB,     // Subtract with borrow
    INST_PXOR,    // Packed XOR (128-bit)
    INST_POR,     // Packed OR (128-bit)
    INST_PAND,    // Packed AND (128-bit)
    INST_PCMPEQQ, // Compare equal (128-bit)
    
    INST_COUNT
} InstructionType;

// Branch prediction hints for optimization
typedef enum {
    BRANCH_HINT_NONE = 0,
    BRANCH_HINT_LIKELY,
    BRANCH_HINT_UNLIKELY
} BranchHint;

// Condition codes for conditional instructions
typedef enum {
    COND_E,   // Equal
    COND_NE,  // Not equal
    COND_L,   // Less than (signed)
    COND_LE,  // Less than or equal (signed)
    COND_G,   // Greater than (signed)
    COND_GE,  // Greater than or equal (signed)
    COND_A,   // Above (unsigned)
    COND_AE,  // Above or equal (unsigned)
    COND_B,   // Below (unsigned)
    COND_BE,  // Below or equal (unsigned)
    COND_Z,   // Zero
    COND_NZ,  // Not zero
    COND_S,   // Sign (negative)
    COND_NS,  // Not sign (positive or zero)
    COND_O,   // Overflow
    COND_NO   // Not overflow
} ConditionCode;

// Operand types
typedef enum {
    OPERAND_REGISTER,
    OPERAND_IMMEDIATE,
    OPERAND_MEMORY,
    OPERAND_LABEL,
    OPERAND_COUNT
} OperandType;

// Memory operand with System V AMD64 addressing modes
typedef struct {
    Register base;          // Base register
    Register index;         // Index register
    uint8_t scale;          // Scale factor (1, 2, 4, 8)
    int32_t displacement;   // Displacement
} MemoryOperand;

// Assembly operand
typedef struct {
    OperandType type;
    union {
        Register reg;
        int64_t immediate;
        MemoryOperand memory;
        char *label;
    } data;
} AssemblyOperand;

// Assembly instruction with C17 flexible array member
struct AssemblyInstruction {
    InstructionType type;
    size_t operand_count;
    BranchHint branch_hint;  // Branch prediction hint for optimization
    char *comment;  // Optional comment for debugging
    AssemblyOperand operands[];  // C17 flexible array member
};

// C17 static assertion for instruction structure alignment (accounting for padding)
_Static_assert(offsetof(AssemblyInstruction, operands) >= sizeof(InstructionType) + sizeof(size_t) + sizeof(BranchHint),
               "AssemblyInstruction flexible array must be properly positioned");

// =============================================================================
// INSTRUCTION BUFFER WITH C17 RESTRICT POINTERS
// =============================================================================

// Instruction buffer for efficient code generation
struct InstructionBuffer {
    AssemblyInstruction **instructions;  // Array of instruction pointers
    size_t count;
    size_t capacity;
    
    // C17 atomic statistics for thread-safe code generation
    atomic_size_t total_instructions_generated;
    atomic_size_t total_bytes_estimated;
    
    // Thread safety for parallel compilation
    pthread_mutex_t mutex;
};

// =============================================================================
// INSTRUCTION BUFFER OPERATIONS WITH C17 FEATURES
// =============================================================================

InstructionBuffer *instruction_buffer_create(size_t initial_capacity);
void instruction_buffer_destroy(InstructionBuffer *buffer);

// Thread-safe instruction addition with restrict pointers
bool instruction_buffer_add(InstructionBuffer * restrict buffer, 
                           AssemblyInstruction * restrict instruction);
bool instruction_buffer_insert(InstructionBuffer * restrict buffer, 
                              size_t index, 
                              AssemblyInstruction * restrict instruction);
bool instruction_buffer_remove(InstructionBuffer * restrict buffer, size_t index);

// =============================================================================
// INSTRUCTION FACTORY FUNCTIONS
// =============================================================================

// Create assembly instruction with variadic operands
AssemblyInstruction *create_instruction(InstructionType type, size_t operand_count, ...);

// Create empty assembly instruction with uninitialized operands
AssemblyInstruction *create_instruction_empty(InstructionType type, size_t operand_count);

// Specialized instruction creators
AssemblyInstruction *create_mov_instruction(Register dest, Register src);
AssemblyInstruction *create_mov_immediate(Register dest, int64_t immediate);
AssemblyInstruction *create_call_instruction(const char *function_name);
AssemblyInstruction *create_ret_instruction(void);
AssemblyInstruction *create_jmp_instruction(const char *label);
AssemblyInstruction *create_je_instruction(const char *label);
AssemblyInstruction *create_jump_instruction(InstructionType jump_type, const char *label);
AssemblyInstruction *create_setcc_instruction(ConditionCode condition, Register dest);
AssemblyInstruction *create_cmp_instruction(Register reg1, Register reg2);
AssemblyInstruction *create_load_instruction(Register dest_reg, Register base_reg, int32_t offset);
AssemblyInstruction *create_inc_instruction(Register reg);

// =============================================================================
// OPERAND CREATION HELPERS
// =============================================================================

// Memory operand helpers
AssemblyOperand create_register_operand(Register reg);
AssemblyOperand create_immediate_operand(int64_t value);
AssemblyOperand create_memory_operand(Register base, Register index, uint8_t scale, int32_t displacement);
AssemblyOperand create_label_operand(const char *label);

// =============================================================================
// ASSEMBLY INSTRUCTION CREATION HELPERS
// =============================================================================

AssemblyInstruction *create_store_local(Register src_reg, int32_t offset);
AssemblyInstruction *create_load_local(Register dest_reg, int32_t offset);
AssemblyInstruction *create_lea_local(Register dest_reg, int32_t offset);
AssemblyInstruction *create_store_indirect(Register src_reg, Register base_reg, int32_t offset);
AssemblyInstruction *create_load_indirect(Register dest_reg, Register base_reg, int32_t offset);
AssemblyInstruction *create_binary_op(BinaryOperator op, Register left_reg, Register right_reg, int64_t immediate, Register dest_reg);

// =============================================================================
// C17 GENERIC MACROS FOR TYPE-SAFE OPERATIONS
// =============================================================================

// Type-safe instruction creation using C17 _Generic
#define CREATE_INSTRUCTION(type, ...) _Generic((type), \
    InstructionType: create_instruction, \
    default: create_instruction \
)(type, __VA_ARGS__)

// Type-safe operand creation
#define CREATE_OPERAND(value) _Generic((value), \
    Register: create_register_operand, \
    int: create_immediate_operand, \
    long: create_immediate_operand, \
    long long: create_immediate_operand, \
    char*: create_label_operand, \
    const char*: create_label_operand \
)(value)

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CODE_GENERATOR_INSTRUCTIONS_H 
