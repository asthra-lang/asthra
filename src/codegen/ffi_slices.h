/**
 * Asthra Programming Language Compiler
 * FFI Slice Operation Generation Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_SLICES_H
#define ASTHRA_FFI_SLICES_H

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
// SLICE OPERATION GENERATION
// =============================================================================

/**
 * Generate slice creation from array literal
 */
bool ffi_generate_slice_creation(FFIAssemblyGenerator *generator,
                                ASTNode *array_literal, Register result_reg);

/**
 * Generate slice length access (.len)
 */
bool ffi_generate_slice_length_access(FFIAssemblyGenerator *generator,
                                     Register slice_reg, Register result_reg);

/**
 * Generate slice bounds checking
 */
bool ffi_generate_slice_bounds_check(FFIAssemblyGenerator *generator,
                                    Register slice_reg, Register index_reg,
                                    char *bounds_error_label);

/**
 * Generate slice to FFI pointer conversion
 */
bool ffi_generate_slice_to_ffi(FFIAssemblyGenerator *generator,
                              Register slice_reg, Register ptr_reg, Register len_reg);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_SLICES_H 
