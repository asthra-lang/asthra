/**
 * Asthra Programming Language Compiler
 * FFI String Operation Generation Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_STRINGS_H
#define ASTHRA_FFI_STRINGS_H

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
// STRING OPERATION GENERATION
// =============================================================================

/**
 * Generate string concatenation (+)
 */
bool ffi_generate_string_concatenation(FFIAssemblyGenerator *generator,
                                     Register left_reg, Register right_reg,
                                     Register result_reg);

/**
 * Generate string interpolation ({})
 */
// String interpolation function removed - feature deprecated for AI generation efficiency

/**
 * Generate deterministic string operations
 */
bool ffi_generate_deterministic_string_op(FFIAssemblyGenerator *generator,
                                         StringOperationType op_type,
                                         Register *operand_regs, size_t operand_count,
                                         Register result_reg);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_STRINGS_H 
