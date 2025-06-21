/**
 * Asthra Programming Language Runtime v1.2 - Enum Support Module
 * Enum Runtime Support and Tagged Union Operations Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_enum_support.h"
#include "core/asthra_runtime_core.h"
#include "memory/asthra_runtime_memory.h"
#include "errors/asthra_runtime_errors.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

// =============================================================================
// BASIC ENUM OPERATIONS
// =============================================================================

bool Asthra_enum_is_variant(void *enum_value, uint32_t expected_tag) {
    if (!enum_value) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_NULL_POINTER, "enum_value is NULL");
        return false;
    }
    
    AsthraEnumVariant *variant = (AsthraEnumVariant *)enum_value;
    return variant->tag == expected_tag;
}

void* Asthra_enum_get_data(void *enum_value) {
    if (!enum_value) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_NULL_POINTER, "enum_value is NULL");
        return NULL;
    }
    
    AsthraEnumVariant *variant = (AsthraEnumVariant *)enum_value;
    
    // For small values stored inline, return pointer to the inline storage
    if (variant->value_size <= sizeof(uint64_t)) {
        return &variant->data.value_inline;
    }
    
    // For larger values, return the pointer
    return variant->data.value_ptr;
}

uint32_t Asthra_enum_get_tag(void *enum_value) {
    if (!enum_value) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_NULL_POINTER, "enum_value is NULL");
        return UINT32_MAX; // Invalid tag
    }
    
    AsthraEnumVariant *variant = (AsthraEnumVariant *)enum_value;
    return variant->tag;
}

size_t Asthra_enum_get_data_size(void *enum_value) {
    if (!enum_value) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_NULL_POINTER, "enum_value is NULL");
        return 0;
    }
    
    AsthraEnumVariant *variant = (AsthraEnumVariant *)enum_value;
    return variant->value_size;
}

AsthraEnumVariant Asthra_enum_create_variant(uint32_t tag, void *data, size_t data_size, 
                                            uint32_t type_id, AsthraOwnershipHint ownership) {
    AsthraEnumVariant variant = {0};
    variant.tag = tag;
    variant.value_size = data_size;
    variant.value_type_id = type_id;
    variant.ownership = ownership;
    
    if (data && data_size > 0) {
        if (data_size <= sizeof(uint64_t)) {
            // Store small values inline
            memcpy(&variant.data.value_inline, data, data_size);
        } else {
            // Allocate memory for larger values
            variant.data.value_ptr = asthra_alloc(data_size, ASTHRA_ZONE_GC);
            if (variant.data.value_ptr) {
                memcpy(variant.data.value_ptr, data, data_size);
            } else {
                ASTHRA_SET_ERROR(ASTHRA_ERROR_OUT_OF_MEMORY, "Failed to allocate memory for enum variant data");
                variant.value_size = 0;
            }
        }
    }
    
    return variant;
}

void Asthra_enum_free_variant(AsthraEnumVariant *variant) {
    if (!variant) {
        return;
    }
    
    // Free allocated memory for larger values
    if (variant->value_size > sizeof(uint64_t) && variant->data.value_ptr) {
        asthra_free(variant->data.value_ptr, ASTHRA_ZONE_GC);
        variant->data.value_ptr = NULL;
    }
    
    // Clear the variant
    memset(variant, 0, sizeof(AsthraEnumVariant));
}

// =============================================================================
// RESULT<T,E> SPECIFIC FUNCTIONS
// =============================================================================

bool Asthra_result_is_ok(void *result) {
    return Asthra_enum_is_variant(result, ASTHRA_RESULT_TAG_OK);
}

bool Asthra_result_is_err(void *result) {
    return Asthra_enum_is_variant(result, ASTHRA_RESULT_TAG_ERR);
}

void* Asthra_result_unwrap_ok(void *result) {
    if (!Asthra_result_is_ok(result)) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_TYPE_MISMATCH, "Attempted to unwrap Ok from Err Result");
        return NULL;
    }
    
    return Asthra_enum_get_data(result);
}

void* Asthra_result_unwrap_err(void *result) {
    if (!Asthra_result_is_err(result)) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_TYPE_MISMATCH, "Attempted to unwrap Err from Ok Result");
        return NULL;
    }
    
    return Asthra_enum_get_data(result);
}

AsthraEnumVariant Asthra_result_create_ok(void *value, size_t value_size, 
                                         uint32_t value_type_id, AsthraOwnershipHint ownership) {
    return Asthra_enum_create_variant(ASTHRA_RESULT_TAG_OK, value, value_size, value_type_id, ownership);
}

AsthraEnumVariant Asthra_result_create_err(void *error, size_t error_size, 
                                          uint32_t error_type_id, AsthraOwnershipHint ownership) {
    return Asthra_enum_create_variant(ASTHRA_RESULT_TAG_ERR, error, error_size, error_type_id, ownership);
}

// =============================================================================
// OPTION<T> SPECIFIC FUNCTIONS
// =============================================================================

bool Asthra_option_is_some(void *option) {
    return Asthra_enum_is_variant(option, ASTHRA_OPTION_TAG_SOME);
}

bool Asthra_option_is_none(void *option) {
    return Asthra_enum_is_variant(option, ASTHRA_OPTION_TAG_NONE);
}

void* Asthra_option_unwrap(void *option) {
    if (!Asthra_option_is_some(option)) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_TYPE_MISMATCH, "Attempted to unwrap None Option");
        return NULL;
    }
    
    return Asthra_enum_get_data(option);
}

AsthraEnumVariant Asthra_option_create_some(void *value, size_t value_size, 
                                           uint32_t value_type_id, AsthraOwnershipHint ownership) {
    return Asthra_enum_create_variant(ASTHRA_OPTION_TAG_SOME, value, value_size, value_type_id, ownership);
}

AsthraEnumVariant Asthra_option_create_none(void) {
    return Asthra_enum_create_variant(ASTHRA_OPTION_TAG_NONE, NULL, 0, 0, ASTHRA_OWNERSHIP_GC);
}

// =============================================================================
// PATTERN MATCHING SUPPORT
// =============================================================================

int Asthra_enum_pattern_match(AsthraEnumVariant *variant, AsthraEnumMatchArm *arms, size_t arm_count) {
    if (!variant || !arms) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_NULL_POINTER, "variant or arms is NULL");
        return -1;
    }
    
    // Try to find exact tag match first
    for (size_t i = 0; i < arm_count; i++) {
        if (arms[i].tag == variant->tag) {
            if (arms[i].callback) {
                return arms[i].callback(variant, arms[i].user_data);
            }
            return 0; // Match found but no callback
        }
    }
    
    // Try wildcard match if no exact match found
    for (size_t i = 0; i < arm_count; i++) {
        if (arms[i].tag == ASTHRA_ENUM_TAG_WILDCARD) {
            if (arms[i].callback) {
                return arms[i].callback(variant, arms[i].user_data);
            }
            return 0; // Wildcard match found but no callback
        }
    }
    
    // No match found
    ASTHRA_SET_ERROR(ASTHRA_ERROR_TYPE_MISMATCH, "No pattern match found for enum variant");
    return -1;
}

// =============================================================================
// UTILITY FUNCTIONS FOR DEBUGGING AND VALIDATION
// =============================================================================

/**
 * Validate enum variant structure
 * @param variant Pointer to enum variant
 * @return true if valid, false otherwise
 */
bool Asthra_enum_validate_variant(AsthraEnumVariant *variant) {
    if (!variant) {
        return false;
    }
    
    // Check for reasonable tag value (not too large)
    if (variant->tag > 1000) {
        return false;
    }
    
    // Validate data consistency
    if (variant->value_size == 0) {
        // No data should mean no pointer
        return variant->data.value_ptr == NULL;
    } else if (variant->value_size <= sizeof(uint64_t)) {
        // Small data should not use pointer
        return true; // inline storage is always valid
    } else {
        // Large data should have valid pointer
        return variant->data.value_ptr != NULL;
    }
}

/**
 * Get debug string representation of enum variant
 * @param variant Pointer to enum variant
 * @param buffer Buffer to write debug string to
 * @param buffer_size Size of buffer
 * @return Number of characters written
 */
size_t Asthra_enum_debug_string(AsthraEnumVariant *variant, char *buffer, size_t buffer_size) {
    if (!variant || !buffer || buffer_size == 0) {
        return 0;
    }
    
    const char *storage_type = (variant->value_size <= sizeof(uint64_t)) ? "inline" : "ptr";
    
    int result = snprintf(buffer, buffer_size, 
                        "EnumVariant{tag=%u, size=%zu, type_id=%u, storage=%s}", 
                        variant->tag, variant->value_size, variant->value_type_id, storage_type);
    return result >= 0 ? (size_t)result : 0;
}
