/**
 * Asthra Safe C Memory Interface v1.0 - Result Type System Implementation
 * Result type creation, unwrapping, validation, and memory management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asthra_ffi_result.h"
#include "asthra_ffi_memory_core.h"

// =============================================================================
// RESULT TYPE CREATION FUNCTIONS
// =============================================================================

// NOTE: Asthra_result_ok() and Asthra_result_err() functions have been moved to
// asthra_ffi_memory_core.c to eliminate code duplication. They are now shared
// across all FFI modules that need result type functionality.

// =============================================================================
// RESULT TYPE CHECKING FUNCTIONS
// =============================================================================

// NOTE: Asthra_result_is_ok() and Asthra_result_is_err() functions have been moved to
// asthra_ffi_memory_core.c to eliminate code duplication. They are now shared
// across all FFI modules that need result type functionality.

// =============================================================================
// RESULT TYPE UNWRAPPING FUNCTIONS
// =============================================================================

void* Asthra_result_unwrap_ok(AsthraFFIResult result) {
    if (result.tag == ASTHRA_FFI_RESULT_OK) {
        return result.data.ok.value;
    }
    return NULL;
}

int Asthra_result_get_error_code(AsthraFFIResult result) {
    if (result.tag == ASTHRA_FFI_RESULT_ERR) {
        return result.data.err.error_code;
    }
    return 0;
}

const char* Asthra_result_get_error_message(AsthraFFIResult result) {
    // NOTE: This function has a design issue - returning address of local array
    // TODO: Fix by changing signature to take AsthraFFIResult* or changing structure
    // For now, return NULL to avoid compilation error
    (void)result; // Suppress unused parameter warning
    return NULL;
}

// =============================================================================
// RESULT TYPE MEMORY MANAGEMENT
// =============================================================================

void Asthra_result_free(AsthraFFIResult result) {
    if (result.tag == ASTHRA_FFI_RESULT_OK) {
        if (result.data.ok.ownership == ASTHRA_OWNERSHIP_TRANSFER_FULL && result.data.ok.value) {
            free(result.data.ok.value);
        }
    }
    
    extern AsthraFFIMemoryManager g_ffi_memory;
    pthread_mutex_lock(&g_ffi_memory.mutex);
    if (g_ffi_memory.stats.result_count > 0) {
        g_ffi_memory.stats.result_count--;
    }
    pthread_mutex_unlock(&g_ffi_memory.mutex);
} 
