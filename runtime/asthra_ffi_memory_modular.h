/**
 * Asthra Safe C Memory Interface v1.0 - Modular Umbrella Header
 * Provides backward compatibility by including all modularized memory components
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This header maintains backward compatibility with the original asthra_ffi_memory.h
 * while using the new modular structure internally.
 */

#ifndef ASTHRA_FFI_MEMORY_MODULAR_H
#define ASTHRA_FFI_MEMORY_MODULAR_H

// =============================================================================
// MODULAR FFI MEMORY SYSTEM INCLUDES
// =============================================================================

// Core memory management (allocation, deallocation, tracking)
#include "memory/asthra_ffi_memory_core.h"

// Slice operations (creation, bounds checking, element access)
#include "memory/asthra_ffi_slice.h"

// String operations (concatenation, interpolation, conversion)
#include "memory/asthra_ffi_string.h"

// Result type system (error handling, pattern matching)
#include "memory/asthra_ffi_result.h"

// Ownership tracking and transfer
#include "memory/asthra_ffi_ownership.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MODULARIZATION INFORMATION
// =============================================================================

#define ASTHRA_FFI_MEMORY_MODULAR_VERSION "1.0.0"
#define ASTHRA_FFI_MEMORY_MODULAR_COMPONENTS 5

/**
 * Information about the modular FFI memory system
 */
typedef struct {
    const char *version;
    int component_count;
    const char *components[ASTHRA_FFI_MEMORY_MODULAR_COMPONENTS];
} AsthraFFIMemoryModularInfo;

/**
 * Get information about the modular FFI memory system
 * @return Module information structure
 */
static inline AsthraFFIMemoryModularInfo asthra_ffi_memory_get_modular_info(void) {
    AsthraFFIMemoryModularInfo info = {.version = ASTHRA_FFI_MEMORY_MODULAR_VERSION,
                                       .component_count = ASTHRA_FFI_MEMORY_MODULAR_COMPONENTS,
                                       .components = {"asthra_ffi_memory_core", "asthra_ffi_slice",
                                                      "asthra_ffi_string", "asthra_ffi_result",
                                                      "asthra_ffi_ownership"}};
    return info;
}

// =============================================================================
// CONVENIENCE INITIALIZATION FUNCTION
// =============================================================================

/**
 * Initialize all FFI memory subsystems
 * @return 0 on success, -1 on failure
 */
static inline int asthra_ffi_memory_init_all(void) {
    return asthra_ffi_memory_init();
}

/**
 * Cleanup all FFI memory subsystems
 */
static inline void asthra_ffi_memory_cleanup_all(void) {
    asthra_ffi_memory_cleanup();
}

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_MEMORY_MODULAR_H
