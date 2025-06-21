/**
 * Asthra Programming Language Compiler
 * Code Generation ABI-Specific Support
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CODE_GENERATOR_ABI_H
#define ASTHRA_CODE_GENERATOR_ABI_H

#include "code_generator_types.h"
#include "code_generator_registers.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SYSTEM V AMD64 ABI SPECIFIC FUNCTIONS
// =============================================================================

// Function prologue and epilogue generation
bool generate_function_prologue(CodeGenerator *generator, size_t stack_size, Register *saved_regs, size_t saved_count);
bool generate_function_epilogue(CodeGenerator *generator, size_t stack_size, Register *saved_regs, size_t saved_count);

// Parameter passing according to System V AMD64 ABI
bool generate_parameter_setup(CodeGenerator *generator, ASTNode *params);
bool generate_function_call(CodeGenerator *generator, const char *function_name, ASTNode *args);
bool generate_return_value(CodeGenerator *generator, ASTNode *return_expr, TypeDescriptor *return_type);

// =============================================================================
// TYPE-SPECIFIC CODE GENERATION
// =============================================================================

// Arithmetic operations
bool generate_binary_arithmetic(CodeGenerator *generator, BinaryOperator op, 
                               Register left_reg, Register right_reg, Register result_reg);
bool generate_unary_arithmetic(CodeGenerator *generator, UnaryOperator op, 
                              Register operand_reg, Register result_reg);

// Memory operations
bool generate_load(CodeGenerator *generator, Register dest_reg, Register base_reg, 
                  int32_t offset, size_t size);
bool generate_store(CodeGenerator *generator, Register src_reg, Register base_reg, 
                   int32_t offset, size_t size);

// Control flow
bool generate_conditional_jump(CodeGenerator *generator, BinaryOperator condition, 
                              Register left_reg, Register right_reg, const char *label);
bool generate_unconditional_jump(CodeGenerator *generator, const char *label);

// String operations (Asthra-specific)
bool generate_string_concatenation(CodeGenerator *generator, Register dest_reg, 
                                  Register left_reg, Register right_reg);
bool generate_string_length(CodeGenerator *generator, Register dest_reg, Register string_reg);
bool generate_string_slice(CodeGenerator *generator, Register dest_reg, Register string_reg, 
                          Register start_reg, Register end_reg);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CODE_GENERATOR_ABI_H 
