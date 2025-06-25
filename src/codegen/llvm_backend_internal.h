/**
 * Asthra Programming Language LLVM Backend Internal
 * Shared internal structures and declarations for LLVM backend modules
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_BACKEND_INTERNAL_H
#define ASTHRA_LLVM_BACKEND_INTERNAL_H

#include "backend_interface.h"
#include "../parser/ast_types.h"
#include "../analysis/semantic_symbols.h"
#include "../analysis/type_info.h"
#include "../compiler.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/DebugInfo.h>

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
} LLVMBackendData;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_BACKEND_INTERNAL_H