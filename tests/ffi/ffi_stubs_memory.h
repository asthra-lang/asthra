/**
 * FFI Test Stubs - Memory Management
 * 
 * Header file for FFI memory management functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_STUBS_MEMORY_H
#define ASTHRA_FFI_STUBS_MEMORY_H

#include "ffi_stubs_types.h"

// =============================================================================
// FFI MEMORY MANAGEMENT FUNCTIONS
// =============================================================================

// Initialize FFI memory manager
void ffi_memory_manager_init(void);

// Allocate and free memory via FFI (tracked)
void* ffi_allocate_memory(size_t size);
void ffi_free_memory(void* ptr, size_t size);

// Get FFI memory statistics
FFIMemoryStats ffi_get_memory_statistics(void);

// Cleanup FFI memory manager (called at shutdown)
void ffi_memory_manager_cleanup(void);

#endif // ASTHRA_FFI_STUBS_MEMORY_H