/**
 * Asthra Programming Language Compiler
 * FFI Security Operation Generation Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_SECURITY_H
#define ASTHRA_FFI_SECURITY_H

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
// SECURITY OPERATION GENERATION
// =============================================================================

/**
 * Generate constant-time operations
 */
bool ffi_generate_constant_time_operation(FFIAssemblyGenerator *generator,
                                        ASTNode *operation);

/**
 * Generate volatile memory access
 */
bool ffi_generate_volatile_memory_access(FFIAssemblyGenerator *generator,
                                        Register memory_reg, size_t size,
                                        bool is_read);

/**
 * Generate secure memory zeroing
 */
bool ffi_generate_secure_zero(FFIAssemblyGenerator *generator,
                             Register memory_reg, Register size_reg);

/**
 * Generate FFI security boundary
 */
bool ffi_generate_ffi_security_boundary(FFIAssemblyGenerator *generator,
                                       ASTNode *extern_call, bool entering_ffi);

/**
 * Validate security annotations at boundary
 */
bool ffi_validate_security_annotations_at_boundary(FFIAssemblyGenerator *generator,
                                                  ASTNode *extern_decl);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_SECURITY_H 
