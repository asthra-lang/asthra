/**
 * FFI Runtime Header
 * 
 * This header provides FFI runtime functions and definitions
 * for Asthra language FFI operations.
 */

#ifndef ASTHRA_FFI_H
#define ASTHRA_FFI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Include other runtime headers
#include "asthra_ffi_error.h"
#include "asthra_ffi_memory.h"
#include "asthra_safety.h"

// =============================================================================
// FFI TYPES AND STRUCTURES
// =============================================================================

typedef enum {
    FFI_TYPE_VOID,
    FFI_TYPE_INT8,
    FFI_TYPE_INT16,
    FFI_TYPE_INT32,
    FFI_TYPE_INT64,
    FFI_TYPE_UINT8,
    FFI_TYPE_UINT16,
    FFI_TYPE_UINT32,
    FFI_TYPE_UINT64,
    FFI_TYPE_FLOAT,
    FFI_TYPE_DOUBLE,
    FFI_TYPE_POINTER,
    FFI_TYPE_STRING,
    FFI_TYPE_SLICE
} AsthrFFIType;

typedef struct {
    void *data;
    size_t length;
} AsthrFFISlice;

typedef struct {
    const char *data;
    size_t length;
} AsthrFFIString;

// =============================================================================
// FFI RUNTIME FUNCTIONS
// =============================================================================

// String operations
AsthrFFIString asthra_ffi_string_create(const char *data, size_t length);
void asthra_ffi_string_destroy(AsthrFFIString *string);
AsthrFFIString asthra_ffi_string_concat(AsthrFFIString left, AsthrFFIString right);

// Slice operations
AsthrFFISlice asthra_ffi_slice_create(void *data, size_t length);
void asthra_ffi_slice_destroy(AsthrFFISlice *slice);
bool asthra_ffi_slice_bounds_check(AsthrFFISlice slice, size_t index);

// Error handling
void asthra_ffi_set_error(const char *message);
const char* asthra_ffi_get_error(void);
void asthra_ffi_clear_error(void);

#endif // ASTHRA_FFI_H 
