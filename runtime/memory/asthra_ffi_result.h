/**
 * Asthra Safe C Memory Interface v1.0 - Result Type System
 * Result type creation, unwrapping, validation, and memory management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_RESULT_H
#define ASTHRA_FFI_RESULT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "asthra_ffi_memory_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// RESULT TYPE DEFINITIONS
// =============================================================================

// NOTE: AsthraFFIResultTag and AsthraFFIResult types have been moved to
// asthra_ffi_memory_core.h to eliminate duplication and provide shared
// access across all FFI modules.

// =============================================================================
// RESULT TYPE CREATION FUNCTIONS
// =============================================================================

// NOTE: Asthra_result_ok() and Asthra_result_err() function declarations
// have been moved to asthra_ffi_memory_core.h and implementations to
// asthra_ffi_memory_core.c to eliminate code duplication.

// =============================================================================
// RESULT TYPE CHECKING FUNCTIONS
// =============================================================================

// NOTE: Asthra_result_is_ok() and Asthra_result_is_err() function declarations
// have been moved to asthra_ffi_memory_core.h and implementations to
// asthra_ffi_memory_core.c to eliminate code duplication.

// =============================================================================
// RESULT TYPE UNWRAPPING FUNCTIONS
// =============================================================================

/**
 * Unwrap the Ok value from a result
 * @param result Result to unwrap
 * @return Pointer to the value, or NULL if result is Err
 */
void *Asthra_result_unwrap_ok(AsthraFFIResult result);

/**
 * Get error code from a result
 * @param result Result to query
 * @return Error code, or 0 if result is Ok
 */
int Asthra_result_get_error_code(AsthraFFIResult result);

/**
 * Get error message from a result
 * @param result Result to query
 * @return Error message string, or NULL if result is Ok
 */
const char *Asthra_result_get_error_message(AsthraFFIResult result);

// =============================================================================
// RESULT TYPE MEMORY MANAGEMENT
// =============================================================================

/**
 * Free resources associated with a result
 * @param result Result to free
 */
void Asthra_result_free(AsthraFFIResult result);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_RESULT_H
