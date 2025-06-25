/**
 * Asthra Programming Language LLVM Runtime Interface
 * Runtime function declarations for LLVM backend
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_RUNTIME_H
#define ASTHRA_LLVM_RUNTIME_H

#include "llvm_backend_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declare all runtime functions in the LLVM module
void declare_runtime_functions(LLVMBackendData *data);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_RUNTIME_H