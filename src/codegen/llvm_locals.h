/**
 * Asthra Programming Language LLVM Local Variables
 * Local variable tracking for LLVM backend
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_LOCALS_H
#define ASTHRA_LLVM_LOCALS_H

#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Register a local variable with its alloca instruction
void register_local_var(LLVMBackendData *data, const char *name, LLVMValueRef alloca, LLVMTypeRef type);

// Look up a local variable entry by name
LocalVar* lookup_local_var_entry(LLVMBackendData *data, const char *name);

// Look up a local variable's alloca by name
LLVMValueRef lookup_local_var(LLVMBackendData *data, const char *name);

// Clear all local variables (typically at end of function)
void clear_local_vars(LLVMBackendData *data);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_LOCALS_H