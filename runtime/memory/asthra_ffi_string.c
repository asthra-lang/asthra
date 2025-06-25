/**
 * Asthra Safe C Memory Interface v1.0 - String Operations Implementation
 * String creation, concatenation, interpolation, and conversion
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asthra_ffi_memory_core.h"
#include "asthra_ffi_string.h"

// =============================================================================
// STRING CREATION AND MANAGEMENT
// =============================================================================

AsthraFFIString Asthra_string_from_cstr(const char *cstr, AsthraOwnershipTransfer ownership) {
    AsthraFFIString ffi_string = {0};

    if (!cstr) {
        return ffi_string;
    }

    size_t len = strlen(cstr);

    if (ownership == ASTHRA_OWNERSHIP_TRANSFER_FULL) {
        ffi_string.data = malloc(len + 1);
        if (ffi_string.data) {
            memcpy(ffi_string.data, cstr, len + 1);
            ffi_string.cap = len + 1;
        }
    } else {
        // Borrowed reference - cast away const for interface compatibility
        ffi_string.data = (char *)cstr;
        ffi_string.cap = len;
    }

    ffi_string.len = len;
    ffi_string.ownership = ownership;
    ffi_string.is_mutable = (ownership == ASTHRA_OWNERSHIP_TRANSFER_FULL);

    extern AsthraFFIMemoryManager g_ffi_memory;
    pthread_mutex_lock(&g_ffi_memory.mutex);
    g_ffi_memory.stats.string_count++;
    pthread_mutex_unlock(&g_ffi_memory.mutex);

    return ffi_string;
}

void Asthra_string_free(AsthraFFIString s) {
    if (s.ownership == ASTHRA_OWNERSHIP_TRANSFER_FULL && s.data) {
        free(s.data);
    }

    extern AsthraFFIMemoryManager g_ffi_memory;
    pthread_mutex_lock(&g_ffi_memory.mutex);
    if (g_ffi_memory.stats.string_count > 0) {
        g_ffi_memory.stats.string_count--;
    }
    pthread_mutex_unlock(&g_ffi_memory.mutex);
}

// =============================================================================
// STRING OPERATIONS
// =============================================================================

AsthraFFIResult Asthra_string_concat(AsthraFFIString a, AsthraFFIString b) {
    if (!a.data || !b.data) {
        return Asthra_result_err(1, // ASTHRA_FFI_ERROR_NULL_POINTER
                                 "One or both strings are NULL", "Asthra_string_concat", NULL);
    }

    size_t new_len = a.len + b.len;
    char *new_data = malloc(new_len + 1);
    if (!new_data) {
        return Asthra_result_err(2, // ASTHRA_FFI_ERROR_OUT_OF_MEMORY
                                 "Failed to allocate concatenated string", "Asthra_string_concat",
                                 NULL);
    }

    memcpy(new_data, a.data, a.len);
    memcpy(new_data + a.len, b.data, b.len);
    new_data[new_len] = '\0';

    AsthraFFIString *result_string = malloc(sizeof(AsthraFFIString));
    if (!result_string) {
        free(new_data);
        return Asthra_result_err(2, // ASTHRA_FFI_ERROR_OUT_OF_MEMORY
                                 "Failed to allocate result string structure",
                                 "Asthra_string_concat", NULL);
    }

    result_string->data = new_data;
    result_string->len = new_len;
    result_string->cap = new_len + 1;
    result_string->ownership = ASTHRA_OWNERSHIP_TRANSFER_FULL;
    result_string->is_mutable = true;

    return Asthra_result_ok(result_string, sizeof(AsthraFFIString), 0,
                            ASTHRA_OWNERSHIP_TRANSFER_FULL);
}

AsthraFFIResult Asthra_string_interpolate(const char *template, AsthraVariantArray args) {
    if (!template) {
        return Asthra_result_err(1, // ASTHRA_FFI_ERROR_NULL_POINTER
                                 "Template string is NULL", "Asthra_string_interpolate", NULL);
    }

    // Estimate result size (conservative)
    size_t estimated_size = strlen(template) + (args.count * 64);
    char *result = malloc(estimated_size);
    if (!result) {
        return Asthra_result_err(2, // ASTHRA_FFI_ERROR_OUT_OF_MEMORY
                                 "Failed to allocate interpolation buffer",
                                 "Asthra_string_interpolate", NULL);
    }

    size_t result_pos = 0;
    size_t template_pos = 0;
    size_t arg_index = 0;
    size_t template_len = strlen(template);

    while (template_pos < template_len && result_pos < estimated_size - 1) {
        if (template[template_pos] == '{' && template_pos + 1 < template_len &&
            template[template_pos + 1] == '}') {
            // Found placeholder, insert argument
            if (arg_index < args.count) {
                AsthraVariant arg = args.args[arg_index];
                char temp_buffer[64];
                const char *insert_str = NULL;

                switch (arg.type) {
                case ASTHRA_VARIANT_BOOL:
                    insert_str = arg.value.bool_val ? "true" : "false";
                    break;
                case ASTHRA_VARIANT_I32:
                    snprintf(temp_buffer, sizeof(temp_buffer), "%d", arg.value.i32_val);
                    insert_str = temp_buffer;
                    break;
                case ASTHRA_VARIANT_U32:
                    snprintf(temp_buffer, sizeof(temp_buffer), "%u", arg.value.u32_val);
                    insert_str = temp_buffer;
                    break;
                case ASTHRA_VARIANT_I64:
                    snprintf(temp_buffer, sizeof(temp_buffer), "%lld",
                             (long long)arg.value.i64_val);
                    insert_str = temp_buffer;
                    break;
                case ASTHRA_VARIANT_U64:
                    snprintf(temp_buffer, sizeof(temp_buffer), "%llu",
                             (unsigned long long)arg.value.u64_val);
                    insert_str = temp_buffer;
                    break;
                case ASTHRA_VARIANT_F32:
                    snprintf(temp_buffer, sizeof(temp_buffer), "%.6f", arg.value.f32_val);
                    insert_str = temp_buffer;
                    break;
                case ASTHRA_VARIANT_F64:
                    snprintf(temp_buffer, sizeof(temp_buffer), "%.6f", arg.value.f64_val);
                    insert_str = temp_buffer;
                    break;
                case ASTHRA_VARIANT_STRING:
                    insert_str = arg.value.string_val.data;
                    break;
                case ASTHRA_VARIANT_PTR:
                    snprintf(temp_buffer, sizeof(temp_buffer), "%p", arg.value.ptr_val);
                    insert_str = temp_buffer;
                    break;
                default:
                    insert_str = "<unknown>";
                    break;
                }

                if (insert_str) {
                    size_t insert_len = strlen(insert_str);
                    if (result_pos + insert_len < estimated_size - 1) {
                        memcpy(result + result_pos, insert_str, insert_len);
                        result_pos += insert_len;
                    }
                }

                arg_index++;
            }

            template_pos += 2; // Skip "{}"
        } else {
            result[result_pos++] = template[template_pos++];
        }
    }

    result[result_pos] = '\0';

    // Resize to actual size
    char *final_result = realloc(result, result_pos + 1);
    if (final_result) {
        result = final_result;
    }

    AsthraFFIString *result_string = malloc(sizeof(AsthraFFIString));
    if (!result_string) {
        free(result);
        return Asthra_result_err(2, // ASTHRA_FFI_ERROR_OUT_OF_MEMORY
                                 "Failed to allocate result string structure",
                                 "Asthra_string_interpolate", NULL);
    }

    result_string->data = result;
    result_string->len = result_pos;
    result_string->cap = result_pos + 1;
    result_string->ownership = ASTHRA_OWNERSHIP_TRANSFER_FULL;
    result_string->is_mutable = true;

    return Asthra_result_ok(result_string, sizeof(AsthraFFIString), 0,
                            ASTHRA_OWNERSHIP_TRANSFER_FULL);
}

// =============================================================================
// STRING CONVERSION
// =============================================================================

char *Asthra_string_to_cstr(AsthraFFIString s, bool transfer_ownership) {
    if (!s.data) {
        return NULL;
    }

    if (transfer_ownership) {
        if (s.ownership == ASTHRA_OWNERSHIP_TRANSFER_FULL) {
            // Transfer ownership of existing data
            char *result = s.data;
            // Note: caller is responsible for freeing
            return result;
        } else {
            // Create a copy
            char *result = malloc(s.len + 1);
            if (result) {
                memcpy(result, s.data, s.len);
                result[s.len] = '\0';
            }
            return result;
        }
    } else {
        // Return borrowed reference
        return s.data;
    }
}

AsthraFFISliceHeader Asthra_string_to_slice(AsthraFFIString s) {
    return Asthra_slice_from_raw_parts(s.data, s.len, sizeof(char), s.is_mutable, s.ownership);
}

// =============================================================================
// VARIANT ARRAY OPERATIONS
// =============================================================================

AsthraVariantArray Asthra_variant_array_new(size_t initial_capacity) {
    AsthraVariantArray array = {0};

    if (initial_capacity > 0) {
        array.args = malloc(initial_capacity * sizeof(AsthraVariant));
        if (array.args) {
            array.capacity = initial_capacity;
        }
    }

    return array;
}

AsthraFFIResult Asthra_variant_array_push(AsthraVariantArray *array, AsthraVariant variant) {
    if (!array) {
        return Asthra_result_err(1, // ASTHRA_FFI_ERROR_NULL_POINTER
                                 "Array pointer is NULL", "Asthra_variant_array_push", NULL);
    }

    if (array->count >= array->capacity) {
        // Grow array
        size_t new_capacity = array->capacity == 0 ? 4 : array->capacity * 2;
        AsthraVariant *new_args = realloc(array->args, new_capacity * sizeof(AsthraVariant));
        if (!new_args) {
            return Asthra_result_err(2, // ASTHRA_FFI_ERROR_OUT_OF_MEMORY
                                     "Failed to grow variant array", "Asthra_variant_array_push",
                                     NULL);
        }
        array->args = new_args;
        array->capacity = new_capacity;
    }

    array->args[array->count] = variant;
    array->count++;

    return Asthra_result_ok(NULL, 0, 0, ASTHRA_OWNERSHIP_TRANSFER_NONE);
}

AsthraFFIResult Asthra_variant_array_get(AsthraVariantArray array, size_t index) {
    if (index >= array.count) {
        return Asthra_result_err(3, // ASTHRA_FFI_ERROR_BOUNDS_CHECK
                                 "Index out of bounds", "Asthra_variant_array_get", NULL);
    }

    AsthraVariant *variant_copy = malloc(sizeof(AsthraVariant));
    if (!variant_copy) {
        return Asthra_result_err(2, // ASTHRA_FFI_ERROR_OUT_OF_MEMORY
                                 "Failed to allocate variant copy", "Asthra_variant_array_get",
                                 NULL);
    }

    *variant_copy = array.args[index];

    return Asthra_result_ok(variant_copy, sizeof(AsthraVariant), 0, ASTHRA_OWNERSHIP_TRANSFER_FULL);
}

void Asthra_variant_array_free(AsthraVariantArray array) {
    if (array.args) {
        // Free any owned string data in variants
        for (size_t i = 0; i < array.count; i++) {
            if (array.args[i].type == ASTHRA_VARIANT_STRING) {
                Asthra_string_free(array.args[i].value.string_val);
            }
        }
        free(array.args);
    }
}
