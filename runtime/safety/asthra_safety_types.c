/**
 * Asthra Programming Language Runtime Safety System
 * Core Type Safety Module - Type validation and checking implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_safety_types.h"
#include "../asthra_safety_common.h"
#include <stdio.h>
#include <string.h>

// External references to safety configuration
// Use accessor function instead of direct global
#define g_safety_config (*asthra_safety_get_config_ptr())

// =============================================================================
// TYPE SAFETY IMPLEMENTATION
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

AsthraTypeSafetyCheck asthra_safety_validate_result_type_usage(AsthraResult result, uint32_t expected_type_id) {
    AsthraTypeSafetyCheck check = {0};
    
    if (!g_safety_config.enable_type_safety_checks) {
        check.is_valid = true;
        return check;
    }
    
    check.expected_type_id = expected_type_id;
    check.context = "result type validation";
    
    if (result.tag == ASTHRA_RESULT_OK) {
        check.actual_type_id = result.data.ok.value_type_id;
        if (expected_type_id == 0 || result.data.ok.value_type_id == expected_type_id) {
            check.is_valid = true;
        } else {
            check.is_valid = false;
            snprintf(check.type_error_message, sizeof(check.type_error_message),
                    "Result Ok type mismatch: expected type %u, got type %u",
                    expected_type_id, result.data.ok.value_type_id);
        }
    } else if (result.tag == ASTHRA_RESULT_ERR) {
        check.actual_type_id = result.data.err.error_type_id;
        // For error cases, we typically don't enforce strict type matching
        check.is_valid = true;
    } else {
        check.is_valid = false;
        snprintf(check.type_error_message, sizeof(check.type_error_message),
                "Invalid result tag: %d", result.tag);
    }
    
    return check;
}

AsthraPatternCompletenessCheck asthra_safety_check_pattern_completeness(AsthraMatchArm *arms, size_t arm_count, uint32_t result_type_id) {
    AsthraPatternCompletenessCheck check = {0};
    
    if (!g_safety_config.enable_pattern_matching_checks) {
        check.result = ASTHRA_PATTERN_COMPLETE;
        return check;
    }
    
    check.pattern_count = arm_count;
    check.covered_patterns = 0;
    
    bool has_ok_pattern = false;
    bool has_err_pattern = false;
    bool has_wildcard = false;
    
    // Analyze patterns
    for (size_t i = 0; i < arm_count; i++) {
        switch (arms[i].pattern) {
            case ASTHRA_MATCH_OK:
                has_ok_pattern = true;
                check.covered_patterns++;
                break;
            case ASTHRA_MATCH_ERR:
                has_err_pattern = true;
                check.covered_patterns++;
                break;
            case ASTHRA_MATCH_WILDCARD:
                has_wildcard = true;
                check.covered_patterns++;
                break;
        }
    }
    
    // Check completeness
    if (has_wildcard || (has_ok_pattern && has_err_pattern)) {
        check.result = ASTHRA_PATTERN_COMPLETE;
    } else {
        check.result = ASTHRA_PATTERN_INCOMPLETE;
        
        // Build missing patterns message
        char missing[1024] = {0};
        if (!has_ok_pattern) {
            strcat(missing, "Ok(_), ");
        }
        if (!has_err_pattern) {
            strcat(missing, "Err(_), ");
        }
        
        // Remove trailing comma and space
        size_t len = strlen(missing);
        if (len > 2) {
            missing[len - 2] = '\0';
        }
        
        snprintf(check.missing_patterns, sizeof(check.missing_patterns),
                "Missing patterns: %s", missing);
    }
    
    return check;
}

AsthraPatternCompletenessCheck asthra_safety_verify_match_exhaustiveness(AsthraMatchArm *arms, size_t arm_count, uint32_t result_type_id) {
    // This is the same as asthra_safety_check_pattern_completeness
    // but with additional verification for specific result types
    AsthraPatternCompletenessCheck check = asthra_safety_check_pattern_completeness(arms, arm_count, result_type_id);
    
    if (!g_safety_config.enable_pattern_matching_checks) {
        return check;
    }
    
    // Additional verification based on result type
    if (result_type_id != 0) {
        // Could add type-specific pattern completeness checks here
        // For now, we use the general completeness check
    }
    
    return check;
}

bool asthra_safety_validate_type_id(uint32_t type_id) {
    if (!g_safety_config.enable_type_safety_checks) {
        return true;
    }
    
    // Type ID 0 is reserved for "any type"
    if (type_id == 0) {
        return true;
    }
    
    // Check if type is registered in the runtime type system
    const char *type_name = asthra_get_type_name(type_id);
    return type_name != NULL;
}

const char *asthra_safety_get_type_name_safe(uint32_t type_id) {
    if (type_id == 0) {
        return "any";
    }
    
    const char *name = asthra_get_type_name(type_id);
    return name ? name : "unknown";
} 
