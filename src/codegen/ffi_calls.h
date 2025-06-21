/**
 * Asthra Programming Language Compiler
 * FFI Call Generation Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_CALLS_H
#define ASTHRA_FFI_CALLS_H

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
// FFI CALL GENERATION
// =============================================================================

/**
 * Generate FFI function call with proper marshaling
 */
bool ffi_generate_extern_call(FFIAssemblyGenerator *generator, ASTNode *call_expr);



/**
 * Generate use declaration
 */


/**
 * Generate variadic function call
 */
bool ffi_generate_variadic_call(FFIAssemblyGenerator *generator, 
                               ASTNode *call_expr, size_t fixed_args);

/**
 * Generate parameter marshaling for FFI calls
 */
bool ffi_generate_parameter_marshaling(FFIAssemblyGenerator *generator,
                                     ASTNode *param, FFIMarshalingType marshal_type,
                                     FFIOwnershipTransferType transfer_type, Register target_reg);

/**
 * Generate return value unmarshaling for FFI calls
 */
bool ffi_generate_return_unmarshaling(FFIAssemblyGenerator *generator,
                                    FFIMarshalingType marshal_type,
                                    FFIOwnershipTransferType transfer_type,
                                    Register source_reg, Register target_reg);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_CALLS_H 
