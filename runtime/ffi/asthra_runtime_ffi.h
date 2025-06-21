/**
 * Asthra Programming Language Runtime v1.2 - FFI Module
 * Foreign Function Interface
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This module provides FFI functionality for interoperating
 * with C code and external libraries.
 */

#ifndef ASTHRA_RUNTIME_FFI_H
#define ASTHRA_RUNTIME_FFI_H

#include "../core/asthra_runtime_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// FFI INTERFACE WITH C17 ENHANCEMENTS
// =============================================================================

void *asthra_ffi_alloc(size_t size, AsthraTransferType transfer);
void asthra_ffi_free(void *ptr, AsthraTransferType transfer);
int asthra_ffi_register_external(void *ptr, size_t size, void (*cleanup)(void *));
void asthra_ffi_unregister_external(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_FFI_H 
