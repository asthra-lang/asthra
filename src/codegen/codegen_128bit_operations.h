/**
 * Asthra Programming Language Compiler
 * 128-bit Integer Code Generation Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CODEGEN_128BIT_OPERATIONS_H
#define ASTHRA_CODEGEN_128BIT_OPERATIONS_H

#include "code_generator_types.h"
#include "../analysis/semantic_types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// 128-BIT TYPE CHECKING
// =============================================================================

/**
 * Check if a type is a 128-bit integer type (i128 or u128)
 */
bool is_128bit_type(TypeDescriptor *type);

// =============================================================================
// 128-BIT CODE GENERATION
// =============================================================================

/**
 * Generate code for 128-bit binary operations
 * 
 * @param generator The code generator context
 * @param op The binary operator
 * @param type The type of the operands (must be i128 or u128)
 * @param left Register containing left operand
 * @param right Register containing right operand
 * @param result Register for storing result
 * @return true if successful, false otherwise
 */
bool generate_128bit_binary_operation(CodeGenerator *generator, BinaryOperator op,
                                     TypeDescriptor *type, Register left, Register right, Register result);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CODEGEN_128BIT_OPERATIONS_H