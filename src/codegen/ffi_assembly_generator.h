/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator with Enhanced Language Features
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Enhanced Code Generation for:
 * - FFI calls with annotations (#[transfer_...], #[borrowed])
 * - Unsafe blocks with GC interaction management
 * - Security operations (#[constant_time], #[volatile_memory])
 * - Spawn statements for concurrency
 * - Pattern matching for Result<T,E> types
 * - String operations (concatenation and interpolation)
 * - Slice operations with bounds checking
 * - Variadic function handling
 * - C17-compatible calling conventions
 */

#ifndef ASTHRA_FFI_ASSEMBLY_GENERATOR_H
#define ASTHRA_FFI_ASSEMBLY_GENERATOR_H

// Include all modular FFI header files
#include "ffi_types.h"
#include "ffi_contexts.h"
#include "ffi_generator_core.h"
#include "ffi_utils.h"
#include "ffi_calls.h"
#include "ffi_patterns.h"
#include "ffi_strings.h"
#include "ffi_slices.h"
#include "ffi_security.h"
#include "ffi_concurrency.h"

#ifdef __cplusplus
extern "C" {
#endif

// Re-export the main structure for backward compatibility
typedef struct FFIAssemblyGenerator FFIAssemblyGenerator;

/**
 * Extract FFI annotation from AST node
 * Phase 4: FFI Annotation Ambiguity Fix - Code Generation Updates
 * 
 * @param generator FFI assembly generator instance
 * @param node AST node that may have FFI annotations (function, extern, param)
 * @param annotation_type Output parameter for the extracted annotation type
 * @return true if FFI annotation found and extracted, false otherwise
 */
bool ffi_extract_annotation(FFIAssemblyGenerator *generator, ASTNode *node, FFIOwnershipTransferType *annotation_type);

/**
 * Generate ownership transfer code based on FFI annotation
 * Phase 4: FFI Annotation Ambiguity Fix - Code Generation Updates
 * 
 * @param generator FFI assembly generator instance
 * @param source_reg Register containing the source value
 * @param target_reg Register for the target value
 * @param transfer_type FFI ownership transfer type from annotation
 * @return true if code generation successful, false otherwise
 */
bool ffi_generate_annotation_based_transfer(FFIAssemblyGenerator *generator,
                                           Register source_reg, Register target_reg,
                                           FFIOwnershipTransferType transfer_type);

/**
 * Generate return value handling with FFI annotation
 * Phase 4: FFI Annotation Ambiguity Fix - Code Generation Updates
 * 
 * @param generator FFI assembly generator instance
 * @param return_expr The return expression AST node
 * @param result_reg Register for the return value
 * @return true if code generation successful, false otherwise
 */
bool ffi_generate_annotated_return(FFIAssemblyGenerator *generator,
                                  ASTNode *return_expr, Register result_reg);

/**
 * Generate parameter marshaling with FFI annotation
 * Phase 4: FFI Annotation Ambiguity Fix - Code Generation Updates
 * 
 * @param generator FFI assembly generator instance
 * @param param_node Parameter declaration with FFI annotation
 * @param value_reg Register containing parameter value
 * @param target_reg Target register for marshaled value
 * @return true if code generation successful, false otherwise
 */
bool ffi_generate_annotated_parameter(FFIAssemblyGenerator *generator,
                                     ASTNode *param_node, Register value_reg,
                                     Register target_reg);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_ASSEMBLY_GENERATOR_H 
