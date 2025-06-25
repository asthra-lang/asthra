/**
 * Asthra Programming Language LLVM Type Conversion
 * Functions for converting Asthra types to LLVM types and debug types
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_TYPES_H
#define ASTHRA_LLVM_TYPES_H

#include "llvm_backend_internal.h"
#include "../analysis/type_info.h"

#ifdef __cplusplus
extern "C" {
#endif

// Convert Asthra type to LLVM type
LLVMTypeRef asthra_type_to_llvm(LLVMBackendData *data, const TypeInfo *type);

// Convert Asthra type to LLVM debug type
LLVMMetadataRef asthra_type_to_debug_type(LLVMBackendData *data, const TypeInfo *type);

// Initialize type cache in LLVMBackendData
void llvm_types_cache_init(LLVMBackendData *data);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_TYPES_H