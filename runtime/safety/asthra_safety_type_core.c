/**
 * Asthra Programming Language Runtime Safety - Core Type Safety
 * Implementation of type validation and slice safety functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_safety_type_core.h"
#include "asthra_runtime.h"
#include <stdio.h>
#include <string.h>

// External references to global state from asthra_safety.c
extern AsthraSafetyConfig g_safety_config;

// =============================================================================
// ADVANCED TYPE SAFETY IMPLEMENTATION
// =============================================================================

AsthraTypeSafetyCheck asthra_safety_validate_slice_type_safety(AsthraSliceHeader slice, uint32_t expected_element_type_id) {
    AsthraTypeSafetyCheck check = {0};
    
    if (!g_safety_config.enable_type_safety_checks) {
        check.is_valid = true;
        return check;
    }
    
    check.expected_type_id = expected_element_type_id;
    check.actual_type_id = slice.type_id;
    check.context = "slice element type validation";
    
    if (expected_element_type_id == 0 || slice.type_id == expected_element_type_id) {
        check.is_valid = true;
    } else {
        check.is_valid = false;
        snprintf(check.type_error_message, sizeof(check.type_error_message),
                "Slice element type mismatch: expected type %u, got type %u", 
                expected_element_type_id, slice.type_id);
    }
    
    // Additional safety checks for slice structure
    if (!slice.ptr && slice.len > 0) {
        check.is_valid = false;
        snprintf(check.type_error_message, sizeof(check.type_error_message),
                "Slice has null pointer but non-zero length %zu", slice.len);
    }
    
    if (slice.element_size == 0) {
        check.is_valid = false;
        snprintf(check.type_error_message, sizeof(check.type_error_message),
                "Slice has zero element size");
    }
    
    return check;
} 
