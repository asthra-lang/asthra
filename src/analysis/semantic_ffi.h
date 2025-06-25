/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - FFI Module Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * FFI validation and extern function handling
 */

#ifndef ASTHRA_SEMANTIC_FFI_H
#define ASTHRA_SEMANTIC_FFI_H

#include "semantic_analyzer.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// FFI TYPE VALIDATION
// =============================================================================

/**
 * Validate that a type is FFI-compatible
 */
bool semantic_validate_ffi_type(SemanticAnalyzer *analyzer, TypeDescriptor *type);

/**
 * Check FFI annotations for validity
 */
bool semantic_check_ffi_annotations(SemanticAnalyzer *analyzer, ASTNodeList *annotations);

/**
 * Validate extern function declaration
 */
bool semantic_validate_extern_function(SemanticAnalyzer *analyzer, ASTNode *extern_decl);

// =============================================================================
// FFI CONSTANT-TIME SAFETY
// =============================================================================

/**
 * Validate FFI constant-time safety
 */
bool semantic_validate_ffi_constant_time_safety(SemanticAnalyzer *analyzer, ASTNode *extern_decl);

/**
 * Validate FFI parameter constant-time safety
 */
bool semantic_validate_ffi_parameter_constant_time_safety(SemanticAnalyzer *analyzer,
                                                          ASTNode *param);

// =============================================================================
// FFI VOLATILE MEMORY SAFETY
// =============================================================================

/**
 * Validate FFI volatile memory safety
 */
bool semantic_validate_ffi_volatile_memory_safety(SemanticAnalyzer *analyzer, ASTNode *call_expr);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_FFI_H
