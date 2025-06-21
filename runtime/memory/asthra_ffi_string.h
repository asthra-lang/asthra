/**
 * Asthra Safe C Memory Interface v1.0 - String Operations
 * String creation, concatenation, interpolation, and conversion
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_STRING_H
#define ASTHRA_FFI_STRING_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "asthra_ffi_memory_core.h"
#include "asthra_ffi_slice.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// STRING TYPES AND STRUCTURES
// =============================================================================

/**
 * String type for FFI operations
 */
typedef struct {
    char *data;                        // UTF-8 encoded string data
    size_t len;                        // Length in bytes
    size_t cap;                        // Capacity in bytes
    AsthraOwnershipTransfer ownership; // Ownership semantics
    bool is_mutable;                   // Mutability flag
} AsthraFFIString;

/**
 * Variant type for variadic function arguments
 */
typedef enum {
    ASTHRA_VARIANT_NULL,
    ASTHRA_VARIANT_BOOL,
    ASTHRA_VARIANT_I8, ASTHRA_VARIANT_U8,
    ASTHRA_VARIANT_I16, ASTHRA_VARIANT_U16,
    ASTHRA_VARIANT_I32, ASTHRA_VARIANT_U32,
    ASTHRA_VARIANT_I64, ASTHRA_VARIANT_U64,
    ASTHRA_VARIANT_F32, ASTHRA_VARIANT_F64,
    ASTHRA_VARIANT_PTR,
    ASTHRA_VARIANT_STRING,
    ASTHRA_VARIANT_SLICE
} AsthraVariantType;

typedef struct {
    AsthraVariantType type;
    union {
        bool bool_val;
        int8_t i8_val; uint8_t u8_val;
        int16_t i16_val; uint16_t u16_val;
        int32_t i32_val; uint32_t u32_val;
        int64_t i64_val; uint64_t u64_val;
        float f32_val; double f64_val;
        void *ptr_val;
        AsthraFFIString string_val;
        AsthraFFISliceHeader slice_val;
    } value;
} AsthraVariant;

typedef struct {
    AsthraVariant *args;
    size_t count;
    size_t capacity;
} AsthraVariantArray;

// =============================================================================
// STRING CREATION AND MANAGEMENT
// =============================================================================

/**
 * Create FFI string from C string
 * @param cstr C string (null-terminated)
 * @param ownership Ownership transfer semantics
 * @return FFI string structure
 */
AsthraFFIString Asthra_string_from_cstr(const char* cstr, AsthraOwnershipTransfer ownership);

/**
 * Free FFI string
 * @param s String to free
 */
void Asthra_string_free(AsthraFFIString s);

// =============================================================================
// STRING OPERATIONS
// =============================================================================

/**
 * Concatenate two FFI strings
 * @param a First string
 * @param b Second string
 * @return Result containing concatenated string or error
 */
AsthraFFIResult Asthra_string_concat(AsthraFFIString a, AsthraFFIString b);

/**
 * Interpolate string with variant arguments
 * @param template Template string with {} placeholders
 * @param args Array of variant arguments
 * @return Result containing interpolated string or error
 */
AsthraFFIResult Asthra_string_interpolate(const char* template, AsthraVariantArray args);

// =============================================================================
// STRING CONVERSION
// =============================================================================

/**
 * Convert FFI string to C string
 * @param s FFI string to convert
 * @param transfer_ownership Whether to transfer ownership of the data
 * @return C string pointer (caller responsible for freeing if ownership transferred)
 */
char* Asthra_string_to_cstr(AsthraFFIString s, bool transfer_ownership);

/**
 * Convert FFI string to slice
 * @param s FFI string to convert
 * @return Slice header representing the string data
 */
AsthraFFISliceHeader Asthra_string_to_slice(AsthraFFIString s);

// =============================================================================
// VARIANT ARRAY OPERATIONS
// =============================================================================

/**
 * Create new variant array
 * @param initial_capacity Initial capacity for the array
 * @return New variant array
 */
AsthraVariantArray Asthra_variant_array_new(size_t initial_capacity);

/**
 * Push variant to array
 * @param array Array to push to
 * @param variant Variant to add
 * @return Result indicating success or failure
 */
AsthraFFIResult Asthra_variant_array_push(AsthraVariantArray* array, AsthraVariant variant);

/**
 * Get variant from array
 * @param array Array to get from
 * @param index Index to retrieve
 * @return Result containing variant or error
 */
AsthraFFIResult Asthra_variant_array_get(AsthraVariantArray array, size_t index);

/**
 * Free variant array
 * @param array Array to free
 */
void Asthra_variant_array_free(AsthraVariantArray array);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_STRING_H 
