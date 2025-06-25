/**
 * Asthra Programming Language LLVM Backend Internal
 * Shared internal structures and declarations for LLVM backend modules
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_BACKEND_INTERNAL_H
#define ASTHRA_LLVM_BACKEND_INTERNAL_H

#include "../analysis/semantic_symbols.h"
#include "../analysis/type_info.h"
#include "../compiler.h"
#include "../parser/ast_types.h"
#include "backend_interface.h"

#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/DebugInfo.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

#ifdef __cplusplus
extern "C" {
#endif

// Simple local variable tracking
typedef struct LocalVar {
    const char *name;
    LLVMValueRef alloca;
    LLVMTypeRef type;
    struct LocalVar *next;
} LocalVar;

// Error reporting for LLVM backend
typedef struct LLVMBackendError {
    char *message;
    char *filename;
    size_t line;
    size_t column;
    const char *function_name;
    struct LLVMBackendError *next;
} LLVMBackendError;

// Loop context for break/continue statements
typedef struct LoopContext {
    LLVMBasicBlockRef continue_block; // Block to jump to for continue
    LLVMBasicBlockRef break_block;    // Block to jump to for break
    struct LoopContext *parent;       // Parent loop context (for nested loops)
} LoopContext;

// Private data for LLVM backend
typedef struct LLVMBackendData {
    LLVMContextRef context;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMValueRef current_function;
    AsthraCompilerContext *compiler_ctx;

    // Type cache for commonly used types
    LLVMTypeRef i32_type;
    LLVMTypeRef i64_type;
    LLVMTypeRef f32_type;
    LLVMTypeRef f64_type;
    LLVMTypeRef bool_type;
    LLVMTypeRef void_type;
    LLVMTypeRef unit_type;
    LLVMTypeRef ptr_type;

    // Runtime function declarations
    LLVMValueRef runtime_malloc_fn;
    LLVMValueRef runtime_free_fn;
    LLVMValueRef runtime_panic_fn;
    LLVMValueRef runtime_log_fn;

    // Debug info support
    LLVMDIBuilderRef di_builder;
    LLVMMetadataRef di_compile_unit;
    LLVMMetadataRef di_file;
    LLVMMetadataRef current_debug_scope;

    // Debug type cache
    LLVMMetadataRef di_i32_type;
    LLVMMetadataRef di_i64_type;
    LLVMMetadataRef di_f32_type;
    LLVMMetadataRef di_f64_type;
    LLVMMetadataRef di_bool_type;
    LLVMMetadataRef di_void_type;
    LLVMMetadataRef di_ptr_type;
    char *output_filename;

    // Local variable tracking
    LocalVar *local_vars;

    // Loop context tracking
    LoopContext *current_loop;

    // Error handling
    LLVMBackendError *error_list;
    bool has_errors;
} LLVMBackendData;

// Loop context management functions - implemented in llvm_backend.c
void llvm_backend_push_loop_context(LLVMBackendData *data, LLVMBasicBlockRef continue_block,
                                    LLVMBasicBlockRef break_block);
void llvm_backend_pop_loop_context(LLVMBackendData *data);
LoopContext *llvm_backend_get_current_loop(const LLVMBackendData *data);

// Error handling functions - implemented in llvm_backend.c
void llvm_backend_report_error(LLVMBackendData *data, const ASTNode *node, const char *message);
void llvm_backend_report_error_printf(LLVMBackendData *data, const ASTNode *node,
                                      const char *format, ...);
bool llvm_backend_has_errors(const LLVMBackendData *data);
void llvm_backend_clear_errors(LLVMBackendData *data);
void llvm_backend_print_errors(const LLVMBackendData *data);

// Convenience macros for common error scenarios
#define LLVM_REPORT_ERROR(data, node, msg)                                                         \
    do {                                                                                           \
        llvm_backend_report_error((data), (node), (msg));                                          \
        return NULL;                                                                               \
    } while (0)

#define LLVM_REPORT_ERROR_PRINTF(data, node, fmt, ...)                                             \
    do {                                                                                           \
        llvm_backend_report_error_printf((data), (node), (fmt), __VA_ARGS__);                      \
        return NULL;                                                                               \
    } while (0)

#define LLVM_CHECK_NULL_RETURN_ERROR(data, node, ptr, msg)                                         \
    do {                                                                                           \
        if (!(ptr)) {                                                                              \
            llvm_backend_report_error((data), (node), (msg));                                      \
            return NULL;                                                                           \
        }                                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_BACKEND_INTERNAL_H