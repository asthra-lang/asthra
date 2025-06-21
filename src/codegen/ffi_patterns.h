/**
 * Asthra Programming Language Compiler
 * FFI Pattern Matching Generation Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_PATTERNS_H
#define ASTHRA_FFI_PATTERNS_H

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
// PATTERN MATCHING GENERATION
// =============================================================================

/**
 * Generate pattern matching for match statements
 */
bool ffi_generate_match_statement(FFIAssemblyGenerator *generator, ASTNode *match_stmt);

/**
 * Generate if-let pattern matching
 */
bool ffi_generate_if_let_statement(FFIAssemblyGenerator *generator, ASTNode *if_let_stmt);

/**
 * Generate Result<T,E> pattern matching
 */
bool ffi_generate_result_pattern_match(FFIAssemblyGenerator *generator,
                                     ASTNode *pattern, Register result_reg);

/**
 * Generate jump table for efficient pattern matching
 */
bool ffi_generate_pattern_jump_table(FFIAssemblyGenerator *generator,
                                    PatternMatchContext *context);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_PATTERNS_H 
