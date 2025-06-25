/**
 * Asthra Programming Language Runtime v1.2 - Strings Module
 * String Operations and UTF-8 Support
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides string management functionality including
 * creation, concatenation, interpolation, and UTF-8 support.
 */

#ifndef ASTHRA_RUNTIME_STRINGS_H
#define ASTHRA_RUNTIME_STRINGS_H

#include "../collections/asthra_runtime_slices.h"
#include "../core/asthra_runtime_core.h"
#include "../types/asthra_runtime_result.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// C17 ENHANCED STRING OPERATIONS ENGINE
// =============================================================================

// String type with UTF-8 support and C17 flexible array member
typedef struct {
    char *restrict data; // C17 restrict for UTF-8 encoded string data
    size_t len;          // Length in bytes
    size_t cap;          // Capacity in bytes
    size_t char_count;   // Number of UTF-8 characters
    AsthraOwnershipHint ownership;
    bool is_mutable;
} AsthraString;

// Compile-time validation of string structure assumptions
ASTHRA_STATIC_ASSERT(sizeof(char) == 1, "char must be 1 byte for UTF-8");
ASTHRA_STATIC_ASSERT(sizeof(AsthraString) <= 48, "String header should be compact");

// =============================================================================
// STRING CREATION AND MANAGEMENT
// =============================================================================

// String creation and management with C17 compound literals
AsthraString asthra_string_new(const char *restrict cstr);
AsthraString asthra_string_with_capacity(size_t capacity);
AsthraString asthra_string_from_utf8(const uint8_t *restrict bytes, size_t len);
void asthra_string_free(AsthraString str);

// =============================================================================
// STRING CONCATENATION OPERATIONS
// =============================================================================

// Deterministic string concatenation with restrict pointers
AsthraString asthra_string_concat(AsthraString a, AsthraString b);
AsthraString asthra_string_concat_cstr(AsthraString str, const char *restrict cstr);
AsthraString asthra_string_concat_multiple(AsthraString *restrict strings, size_t count);

// =============================================================================
// STRING INTERPOLATION ENGINE
// =============================================================================

// String interpolation engine
typedef enum {
    ASTHRA_INTERP_STRING,
    ASTHRA_INTERP_INT,
    ASTHRA_INTERP_UINT,
    ASTHRA_INTERP_FLOAT,
    ASTHRA_INTERP_BOOL,
    ASTHRA_INTERP_CUSTOM
} AsthraInterpolationType;

typedef struct {
    AsthraInterpolationType type;
    union {
        const char *restrict str_val;
        int64_t int_val;
        uint64_t uint_val;
        double float_val;
        bool bool_val;
        struct {
            void *data;
            char *(*formatter)(void *data);
        } custom;
    } value;
} AsthraInterpolationArg;

AsthraString asthra_string_interpolate(const char *restrict template,
                                       AsthraInterpolationArg *restrict args, size_t arg_count);

// =============================================================================
// STRING UTILITY OPERATIONS
// =============================================================================

// String operations with C17 optimizations
size_t asthra_string_len(AsthraString str);
size_t asthra_string_char_count(AsthraString str);
bool asthra_string_equals(AsthraString a, AsthraString b);
int asthra_string_compare(AsthraString a, AsthraString b);
AsthraString asthra_string_clone(AsthraString str);
char *asthra_string_to_cstr(AsthraString str, AsthraTransferType transfer);
AsthraSliceHeader asthra_string_to_slice(AsthraString str);

// =============================================================================
// STRING-RESULT INTEGRATION
// =============================================================================

// Result functions that use AsthraString
AsthraResult asthra_result_ok_string(AsthraString str);
AsthraResult asthra_result_err_string(AsthraString error_str);
AsthraString asthra_result_unwrap_string(AsthraResult result);
AsthraString asthra_result_unwrap_err_string(AsthraResult result);

// =============================================================================
// I/O SUPPORT FUNCTIONS
// =============================================================================

// String conversion utilities for I/O
AsthraString asthra_string_from_cstr(const char *cstr);
AsthraString asthra_float_to_string(double value);

// =============================================================================
// ADDITIONAL STRING FUNCTIONS
// =============================================================================

// Additional string functions (missing prototypes)
char *asthra_string_to_cstr_ffi(AsthraString str);
AsthraString asthra_string_substring(AsthraString str, size_t start, size_t end);
int64_t asthra_string_find_char(AsthraString str, uint32_t ch);
int asthra_string_starts_with_string(AsthraString str, AsthraString prefix);
AsthraString asthra_string_to_lowercase(AsthraString str);
uint32_t asthra_string_char_at(AsthraString str, size_t index);
int64_t asthra_string_find_last(AsthraString str, AsthraString needle);
AsthraString asthra_bytes_to_string(AsthraSliceHeader bytes);
AsthraSliceHeader asthra_string_to_utf8(AsthraString str);
AsthraString asthra_int_to_string(int64_t value);
AsthraString asthra_uint_to_string(uint64_t value);

#ifdef __cplusplus
}
#endif

// Missing function prototypes
AsthraString asthra_string_replace(AsthraString str, AsthraString old_str, AsthraString new_str);
bool asthra_string_ends_with(AsthraString str, AsthraString suffix);
AsthraString asthra_string_trim(AsthraString str);
AsthraString asthra_string_to_upper(AsthraString str);
AsthraSliceHeader asthra_string_split(AsthraString str, AsthraString delimiter);
AsthraString asthra_string_join(AsthraSliceHeader strings, AsthraString separator);

#endif // ASTHRA_RUNTIME_STRINGS_H
