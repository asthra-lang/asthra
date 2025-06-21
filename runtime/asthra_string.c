/**
 * Asthra Programming Language String Operations v1.2
 * C17 Enhanced String Operations Engine
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * IMPLEMENTATION FEATURES:
 * - Deterministic string operations (concatenation + interpolation)
 * - UTF-8 support and character counting
 * - C17 restrict pointers for optimization
 * - Memory-safe string handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#include "asthra_runtime.h"

// =============================================================================
// C17 ENHANCED STRING OPERATIONS
// =============================================================================

AsthraString asthra_string_new(const char * restrict cstr) {
    if (!cstr) {
        // Return empty string using C17 compound literal
        return (AsthraString){
            .data = NULL,
            .len = 0,
            .cap = 0,
            .char_count = 0,
            .ownership = ASTHRA_OWNERSHIP_GC,
            .is_mutable = false
        };
    }

    size_t len = strlen(cstr);
    char * restrict data = asthra_alloc(len + 1, ASTHRA_ZONE_GC);
    if (!data) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_OUT_OF_MEMORY, "Failed to allocate string memory");
        return (AsthraString){0}; // C17 zero initialization
    }

    memcpy(data, cstr, len + 1);

    // Use C17 compound literal with designated initializers
    return (AsthraString){
        .data = data,
        .len = len,
        .cap = len + 1,
        .char_count = len, // Simplified: assume ASCII for now
        .ownership = ASTHRA_OWNERSHIP_GC,
        .is_mutable = true
    };
}

AsthraString asthra_string_with_capacity(size_t capacity) {
    if (capacity == 0) {
        return (AsthraString){0}; // C17 zero initialization
    }

    char * restrict data = asthra_alloc_zeroed(capacity, ASTHRA_ZONE_GC);
    if (!data) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_OUT_OF_MEMORY, "Failed to allocate string capacity");
        return (AsthraString){0};
    }

    // Use C17 compound literal
    return (AsthraString){
        .data = data,
        .len = 0,
        .cap = capacity,
        .char_count = 0,
        .ownership = ASTHRA_OWNERSHIP_GC,
        .is_mutable = true
    };
}

void asthra_string_free(AsthraString str) {
    if (str.data && str.ownership == ASTHRA_OWNERSHIP_GC) {
        asthra_free(str.data, ASTHRA_ZONE_GC);
    }
}

AsthraString asthra_string_concat(AsthraString a, AsthraString b) {
    size_t new_len = a.len + b.len;
    size_t new_cap = new_len + 1;
    
    char * restrict new_data = asthra_alloc(new_cap, ASTHRA_ZONE_GC);
    if (!new_data) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_OUT_OF_MEMORY, "Failed to allocate concatenated string");
        return (AsthraString){0};
    }

    // Use restrict pointers for optimization
    if (a.data) memcpy(new_data, a.data, a.len);
    if (b.data) memcpy(new_data + a.len, b.data, b.len);
    new_data[new_len] = '\0';

    // Use C17 compound literal
    return (AsthraString){
        .data = new_data,
        .len = new_len,
        .cap = new_cap,
        .char_count = a.char_count + b.char_count,
        .ownership = ASTHRA_OWNERSHIP_GC,
        .is_mutable = true
    };
}

AsthraString asthra_string_interpolate(const char * restrict template, 
                                     AsthraInterpolationArg * restrict args, size_t arg_count) {
    if (!template) {
        return (AsthraString){0}; // C17 zero initialization
    }

    // Estimate required capacity
    size_t estimated_len = strlen(template) + (arg_count * 32); // Conservative estimate
    char * restrict buffer = asthra_alloc(estimated_len, ASTHRA_ZONE_GC);
    if (!buffer) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_OUT_OF_MEMORY, "Failed to allocate interpolation buffer");
        return (AsthraString){0};
    }

    size_t buffer_pos = 0;
    size_t template_pos = 0;
    size_t arg_index = 0;
    
    while (template[template_pos] && buffer_pos < estimated_len - 1) {
        if (template[template_pos] == '{' && template[template_pos + 1] == '}' && arg_index < arg_count) {
            // Interpolate argument
            AsthraInterpolationArg *arg = &args[arg_index++];
            char temp_buf[64];
            const char * restrict arg_str = NULL;
            
            switch (arg->type) {
                case ASTHRA_INTERP_STRING:
                    arg_str = arg->value.str_val;
                    break;
                case ASTHRA_INTERP_INT:
                    snprintf(temp_buf, sizeof(temp_buf), "%" PRId64, arg->value.int_val);
                    arg_str = temp_buf;
                    break;
                case ASTHRA_INTERP_UINT:
                    snprintf(temp_buf, sizeof(temp_buf), "%" PRIu64, arg->value.uint_val);
                    arg_str = temp_buf;
                    break;
                case ASTHRA_INTERP_FLOAT:
                    snprintf(temp_buf, sizeof(temp_buf), "%g", arg->value.float_val);
                    arg_str = temp_buf;
                    break;
                case ASTHRA_INTERP_BOOL:
                    arg_str = arg->value.bool_val ? "true" : "false";
                    break;
                case ASTHRA_INTERP_CUSTOM:
                    if (arg->value.custom.formatter) {
                        arg_str = arg->value.custom.formatter(arg->value.custom.data);
                    }
                    break;
            }
            
            if (arg_str) {
                size_t arg_len = strlen(arg_str);
                if (buffer_pos + arg_len < estimated_len - 1) {
                    memcpy(buffer + buffer_pos, arg_str, arg_len);
                    buffer_pos += arg_len;
                }
            }
            
            template_pos += 2; // Skip "{}"
        } else {
            buffer[buffer_pos++] = template[template_pos++];
        }
    }
    
    buffer[buffer_pos] = '\0';

    // Use C17 compound literal
    return (AsthraString){
        .data = buffer,
        .len = buffer_pos,
        .cap = estimated_len,
        .char_count = buffer_pos, // Simplified: assume ASCII
        .ownership = ASTHRA_OWNERSHIP_GC,
        .is_mutable = true
    };
}

// String utility functions
size_t asthra_string_len(AsthraString str) { return str.len; }
size_t asthra_string_char_count(AsthraString str) { return str.char_count; }

bool asthra_string_equals(AsthraString a, AsthraString b) {
    if (a.len != b.len) return false;
    if (!a.data && !b.data) return true;
    if (!a.data || !b.data) return false;
    return memcmp(a.data, b.data, a.len) == 0;
}

AsthraString asthra_string_clone(AsthraString str) {
    return asthra_string_new(str.data);
}

char *asthra_string_to_cstr(AsthraString str, AsthraTransferType transfer) {
    if (!str.data) return NULL;
    
    if (transfer == ASTHRA_TRANSFER_FULL) {
        // Transfer ownership
        char *result = str.data;
        // Note: caller is responsible for freeing
        return result;
    } else {
        // Create copy
        char *copy = malloc(str.len + 1);
        if (copy) {
            memcpy(copy, str.data, str.len + 1);
        }
        return copy;
    }
}

AsthraSliceHeader asthra_string_to_slice(AsthraString str) {
    return asthra_slice_from_raw_parts(str.data, str.len, sizeof(char), false, str.ownership);
}

// String conversion utilities for I/O
AsthraString asthra_string_from_cstr(const char *cstr) {
    return asthra_string_new(cstr);
}

// =============================================================================
// FFI RUNTIME INTEGRATION FUNCTIONS
// =============================================================================

// Convert Asthra string to C string with transfer semantics
char *asthra_string_to_cstr_ffi(AsthraString str) {
    if (!str.data) return NULL;
    
    // Always create a copy for FFI safety
    char *copy = malloc(str.len + 1);
    if (copy) {
        memcpy(copy, str.data, str.len);
        copy[str.len] = '\0';
    }
    return copy;
}

// String utility functions for FFI
AsthraString asthra_string_substring(AsthraString str, size_t start, size_t end) {
    if (!str.data || start >= str.len || end > str.len || start >= end) {
        return (AsthraString){0};
    }
    
    size_t new_len = end - start;
    char *new_data = malloc(new_len + 1);
    if (!new_data) {
        return (AsthraString){0};
    }
    
    memcpy(new_data, str.data + start, new_len);
    new_data[new_len] = '\0';
    
    return (AsthraString){
        .data = new_data,
        .len = new_len,
        .cap = new_len + 1,
        .char_count = new_len, // Simplified
        .ownership = ASTHRA_OWNERSHIP_GC,
        .is_mutable = true
    };
}

int64_t asthra_string_find_char(AsthraString str, uint32_t ch) {
    if (!str.data) return -1;
    
    for (size_t i = 0; i < str.len; i++) {
        if ((uint32_t)str.data[i] == ch) {
            return (int64_t)i;
        }
    }
    return -1;
}

int asthra_string_starts_with_string(AsthraString str, AsthraString prefix) {
    if (!str.data || !prefix.data || prefix.len > str.len) {
        return 0;
    }
    
    return memcmp(str.data, prefix.data, prefix.len) == 0 ? 1 : 0;
}

AsthraString asthra_string_to_lowercase(AsthraString str) {
    if (!str.data) return (AsthraString){0};
    
    char *new_data = malloc(str.len + 1);
    if (!new_data) return (AsthraString){0};
    
    for (size_t i = 0; i < str.len; i++) {
        char c = str.data[i];
        if (c >= 'A' && c <= 'Z') {
            new_data[i] = c + 32;
        } else {
            new_data[i] = c;
        }
    }
    new_data[str.len] = '\0';
    
    return (AsthraString){
        .data = new_data,
        .len = str.len,
        .cap = str.len + 1,
        .char_count = str.len, // Simplified
        .ownership = ASTHRA_OWNERSHIP_GC,
        .is_mutable = true
    };
}

uint32_t asthra_string_char_at(AsthraString str, size_t index) {
    if (!str.data || index >= str.len) {
        return 0;
    }
    return (uint32_t)str.data[index];
}

int64_t asthra_string_find_last(AsthraString str, AsthraString needle) {
    if (!str.data || !needle.data || needle.len > str.len) {
        return -1;
    }
    
    for (int64_t i = (int64_t)(str.len - needle.len); i >= 0; i--) {
        if (memcmp(str.data + i, needle.data, needle.len) == 0) {
            return i;
        }
    }
    return -1;
}

// Convert bytes to string
AsthraString asthra_bytes_to_string(AsthraSliceHeader bytes) {
    if (!bytes.ptr || bytes.len == 0) {
        return (AsthraString){0};
    }
    
    char *new_data = malloc(bytes.len + 1);
    if (!new_data) return (AsthraString){0};
    
    memcpy(new_data, bytes.ptr, bytes.len);
    new_data[bytes.len] = '\0';
    
    return (AsthraString){
        .data = new_data,
        .len = bytes.len,
        .cap = bytes.len + 1,
        .char_count = bytes.len, // Simplified
        .ownership = ASTHRA_OWNERSHIP_GC,
        .is_mutable = true
    };
}

// Convert string to UTF-8 bytes
AsthraSliceHeader asthra_string_to_utf8(AsthraString str) {
    if (!str.data) {
        return (AsthraSliceHeader){0};
    }
    
    // For now, assume string is already UTF-8
    return asthra_slice_from_raw_parts(str.data, str.len, sizeof(char), false, str.ownership);
}

// Number to string conversions
AsthraString asthra_int_to_string(int64_t value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%" PRId64, value);
    return asthra_string_new(buffer);
}

AsthraString asthra_uint_to_string(uint64_t value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%" PRIu64, value);
    return asthra_string_new(buffer);
}

AsthraString asthra_float_to_string(double value) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%g", value);
    return asthra_string_new(buffer);
} 
