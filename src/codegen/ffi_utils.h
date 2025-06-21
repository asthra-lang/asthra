/**
 * Asthra Programming Language Compiler
 * FFI Utility Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_UTILS_H
#define ASTHRA_FFI_UTILS_H

#include "ffi_types.h"
#include "ffi_contexts.h"
#include "../parser/ast.h"
#include "code_generator.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct FFIAssemblyGenerator FFIAssemblyGenerator;

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Emit an instruction with variable number of operands
 */
bool emit_instruction(FFIAssemblyGenerator *generator, InstructionType type, size_t operand_count, ...);

/**
 * Emit a comment in the assembly
 */
void emit_comment(FFIAssemblyGenerator *generator, const char *comment);

/**
 * Emit a label in the assembly
 */
void emit_label(FFIAssemblyGenerator *generator, const char *label);

/**
 * Create register operand
 */
AssemblyOperand create_register_operand(Register reg);

/**
 * Create immediate operand
 */
AssemblyOperand create_immediate_operand(int64_t value);

/**
 * Create memory operand
 */
AssemblyOperand create_memory_operand(Register base, Register index, uint8_t scale, int32_t displacement);

/**
 * Create label operand
 */
AssemblyOperand create_label_operand(const char *label);

/**
 * Create instruction with variable number of operands
 */
AssemblyInstruction *create_instruction(InstructionType type, size_t operand_count, ...);

// =============================================================================
// VALIDATION AND UTILITY FUNCTIONS
// =============================================================================

/**
 * Validate generated assembly for correctness
 */
bool ffi_validate_generated_assembly(FFIAssemblyGenerator *generator);

/**
 * Get FFI assembly generation statistics
 */
void ffi_get_generation_statistics(FFIAssemblyGenerator *generator,
                                  size_t *ffi_calls, size_t *pattern_matches,
                                  size_t *string_ops, size_t *slice_ops,
                                  size_t *security_ops, size_t *spawn_stmts);

/**
 * Print FFI assembly with NASM syntax
 */
bool ffi_print_nasm_assembly(FFIAssemblyGenerator *generator,
                            char * restrict output_buffer, size_t buffer_size);

/**
 * Optimize zero-cost abstractions
 */
bool ffi_optimize_zero_cost_abstractions(FFIAssemblyGenerator *generator);

/**
 * Generate C-compatible struct layouts
 */
bool ffi_generate_c_struct_layout(FFIAssemblyGenerator *generator,
                                 ASTNode *struct_decl, bool packed);

/**
 * Generate ownership transfer code
 */
bool ffi_generate_ownership_transfer(FFIAssemblyGenerator *generator,
                                   Register source_reg, Register target_reg,
                                   FFIOwnershipTransferType transfer_type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_UTILS_H 
