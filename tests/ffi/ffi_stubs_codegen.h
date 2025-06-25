/**
 * FFI Test Stubs - Code Generation
 *
 * Header file for enhanced codegen context functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_STUBS_CODEGEN_H
#define ASTHRA_FFI_STUBS_CODEGEN_H

#include "ffi_stubs_types.h"

// Forward declarations for functions needed by code generator
bool code_generator_emit_function_call(CodeGenerator *generator, const char *func_name,
                                       int arg_count, void *args);
bool code_generator_emit_return(CodeGenerator *generator, void *value);

// =============================================================================
// ENHANCED CODEGEN FUNCTIONS
// =============================================================================

// Creation and destruction
EnhancedCodegenContext *enhanced_codegen_create_context(void);
void enhanced_codegen_destroy_context(EnhancedCodegenContext *ctx);

// Code generation functions
bool enhanced_codegen_generate_function_call(EnhancedCodegenContext *ctx, const char *func_name);
bool enhanced_codegen_generate_return(EnhancedCodegenContext *ctx, void *value);
bool enhanced_codegen_generate_ffi_call(EnhancedCodegenContext *ctx, const char *func_name,
                                        void **params, size_t param_count);

// Assembly output and statistics
const char *enhanced_codegen_get_assembly_output(EnhancedCodegenContext *ctx);
CodegenStats enhanced_codegen_get_stats(EnhancedCodegenContext *ctx);

// =============================================================================
// LEGACY COMPATIBILITY FUNCTIONS
// =============================================================================

// Legacy compatibility wrappers
CodegenContext *codegen_create_context(void);
void codegen_destroy_context(CodegenContext *ctx);
bool codegen_generate_function_call(CodegenContext *ctx, const char *func_name);
bool codegen_generate_return(CodegenContext *ctx, void *value);
int codegen_get_instruction_count(CodegenContext *ctx);

#endif // ASTHRA_FFI_STUBS_CODEGEN_H