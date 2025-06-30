/**
 * Asthra Programming Language LLVM Runtime Interface
 * Implementation of runtime function declarations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_runtime.h"

// Declare Asthra runtime functions in LLVM module
void declare_runtime_functions(LLVMBackendData *data) {
    // Memory allocation functions
    {
        // void* asthra_alloc(size_t size, int zone)
        LLVMTypeRef param_types[] = {data->i64_type, data->i32_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->ptr_type, param_types, 2, false);
        data->runtime_malloc_fn = LLVMAddFunction(data->module, "asthra_alloc", fn_type);
        LLVMSetLinkage(data->runtime_malloc_fn, LLVMExternalLinkage);
    }

    {
        // void asthra_free(void* ptr, int zone)
        LLVMTypeRef param_types[] = {data->ptr_type, data->i32_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 2, false);
        data->runtime_free_fn = LLVMAddFunction(data->module, "asthra_free", fn_type);
        LLVMSetLinkage(data->runtime_free_fn, LLVMExternalLinkage);
    }

    // GC functions
    {
        // void asthra_gc_register_root(void* ptr)
        LLVMTypeRef param_types[] = {data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
        LLVMValueRef fn = LLVMAddFunction(data->module, "asthra_gc_register_root", fn_type);
        LLVMSetLinkage(fn, LLVMExternalLinkage);
    }

    {
        // void asthra_gc_unregister_root(void* ptr)
        LLVMTypeRef param_types[] = {data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
        LLVMValueRef fn = LLVMAddFunction(data->module, "asthra_gc_unregister_root", fn_type);
        LLVMSetLinkage(fn, LLVMExternalLinkage);
    }

    // Error handling
    {
        // void asthra_panic(const char* message)
        LLVMTypeRef param_types[] = {data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
        data->runtime_panic_fn = LLVMAddFunction(data->module, "asthra_panic", fn_type);
        LLVMSetLinkage(data->runtime_panic_fn, LLVMExternalLinkage);
    }

    // Logging
    {
        // void asthra_log(int level, const char* format, ...)
        LLVMTypeRef param_types[] = {data->i32_type, data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 2, true); // variadic
        data->runtime_log_fn = LLVMAddFunction(data->module, "asthra_log", fn_type);
        LLVMSetLinkage(data->runtime_log_fn, LLVMExternalLinkage);
    }

    // String operations
    {
        // AsthraString* asthra_string_create(const char* data, size_t length)
        LLVMTypeRef param_types[] = {data->ptr_type, data->i64_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->ptr_type, param_types, 2, false);
        LLVMValueRef fn = LLVMAddFunction(data->module, "asthra_string_create", fn_type);
        LLVMSetLinkage(fn, LLVMExternalLinkage);
    }

    {
        // void asthra_string_free(AsthraString* str)
        LLVMTypeRef param_types[] = {data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
        LLVMValueRef fn = LLVMAddFunction(data->module, "asthra_string_free", fn_type);
        LLVMSetLinkage(fn, LLVMExternalLinkage);
    }

    {
        // char* asthra_string_concat_wrapper(const char* left, const char* right)
        LLVMTypeRef param_types[] = {data->ptr_type, data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->ptr_type, param_types, 2, false);
        data->runtime_string_concat_fn = LLVMAddFunction(data->module, "asthra_string_concat_wrapper", fn_type);
        LLVMSetLinkage(data->runtime_string_concat_fn, LLVMExternalLinkage);
    }

    // Print function for basic output (temporary until proper I/O)
    {
        // int printf(const char* format, ...)
        LLVMTypeRef param_types[] = {data->ptr_type};
        LLVMTypeRef fn_type = LLVMFunctionType(data->i32_type, param_types, 1, true); // variadic
        LLVMValueRef fn = LLVMAddFunction(data->module, "printf", fn_type);
        LLVMSetLinkage(fn, LLVMExternalLinkage);
    }
}