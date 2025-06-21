/**
 * Asthra Safety System - Strings and Slices Safety Module
 * String operation validation and slice bounds checking
 */

#include "asthra_safety.h"
#include "asthra_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// STRING OPERATION VALIDATION IMPLEMENTATION
// =============================================================================

AsthraStringOperationValidation asthra_safety_validate_string_concatenation(AsthraString *strings, size_t count) {
    AsthraStringOperationValidation validation = {0};
    AsthraSafetyConfig config = asthra_safety_get_config();
    
    if (!config.enable_string_operation_validation) {
        validation.is_deterministic = true;
        validation.result_length = 0;
        return validation;
    }
    
    if (!strings || count == 0) {
        validation.has_overflow_risk = true;
        snprintf(validation.validation_message, sizeof(validation.validation_message),
                "Invalid string concatenation: null strings or zero count");
        return validation;
    }
    
    size_t total_length = 0;
    bool has_null_strings = false;
    bool has_encoding_issues = false;
    
    // Validate each string and calculate total length
    for (size_t i = 0; i < count; i++) {
        if (!strings[i].data) {
            has_null_strings = true;
            continue;
        }
        
        // Check for potential overflow
        if (total_length > SIZE_MAX - strings[i].len) {
            validation.has_overflow_risk = true;
            snprintf(validation.validation_message, sizeof(validation.validation_message),
                    "String concatenation would overflow at string %zu", i);
            return validation;
        }
        
        total_length += strings[i].len;
        
        // Basic UTF-8 validation (simplified)
        for (size_t j = 0; j < strings[i].len; j++) {
            uint8_t byte = (uint8_t)strings[i].data[j];
            
            // Check for invalid UTF-8 sequences (simplified check)
            if (byte >= 0x80) {
                // Multi-byte sequence - basic validation
                if (byte >= 0xF8) {
                    has_encoding_issues = true;
                    break;
                }
            }
        }
        
        if (has_encoding_issues) {
            break;
        }
    }
    
    // Set validation results
    validation.result_length = total_length;
    validation.max_safe_length = SIZE_MAX / 2; // Conservative safe limit
    validation.has_overflow_risk = (total_length > validation.max_safe_length);
    validation.has_encoding_issues = has_encoding_issues;
    validation.is_deterministic = !has_null_strings && !has_encoding_issues && !validation.has_overflow_risk;
    
    if (has_null_strings) {
        snprintf(validation.validation_message, sizeof(validation.validation_message),
                "String concatenation contains null strings");
    } else if (has_encoding_issues) {
        snprintf(validation.validation_message, sizeof(validation.validation_message),
                "String concatenation contains invalid UTF-8 sequences");
    } else if (validation.has_overflow_risk) {
        snprintf(validation.validation_message, sizeof(validation.validation_message),
                "String concatenation result too large: %zu bytes", total_length);
    } else {
        snprintf(validation.validation_message, sizeof(validation.validation_message),
                "String concatenation valid: %zu bytes", total_length);
    }
    
    // Report violations if found
    if (!validation.is_deterministic) {
        asthra_safety_report_violation(ASTHRA_VIOLATION_MEMORY_SAFETY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                     validation.validation_message, __FILE__, __LINE__, __func__,
                                     &validation, sizeof(validation));
    }
    
    return validation;
}

// =============================================================================
// SLICE BOUNDS CHECKING IMPLEMENTATION
// =============================================================================

AsthraBoundaryCheck asthra_safety_slice_bounds_check(AsthraSliceHeader slice, size_t index) {
    AsthraBoundaryCheck check = {0};
    AsthraSafetyConfig config = asthra_safety_get_config();
    
    if (!config.enable_slice_bounds_checking) {
        check.is_valid = true;
        return check;
    }
    
    check.attempted_index = index;
    check.slice_length = slice.len;
    check.element_size = slice.element_size;
    
    // Check for null pointer
    if (!slice.ptr) {
        check.is_null_pointer = true;
        snprintf(check.error_details, sizeof(check.error_details),
                "Slice has null pointer");
        
        asthra_safety_report_violation(ASTHRA_VIOLATION_MEMORY_SAFETY, ASTHRA_SAFETY_LEVEL_ENHANCED,
                                     check.error_details, __FILE__, __LINE__, __func__,
                                     &check, sizeof(check));
        return check;
    }
    
    // Check for corrupted header
    if (slice.element_size == 0) {
        check.is_corrupted_header = true;
        snprintf(check.error_details, sizeof(check.error_details),
                "Slice has zero element size");
        
        asthra_safety_report_violation(ASTHRA_VIOLATION_MEMORY_SAFETY, ASTHRA_SAFETY_LEVEL_ENHANCED,
                                     check.error_details, __FILE__, __LINE__, __func__,
                                     &check, sizeof(check));
        return check;
    }
    
    // Check bounds
    if (index >= slice.len) {
        check.is_out_of_bounds = true;
        snprintf(check.error_details, sizeof(check.error_details),
                "Index %zu out of bounds for slice of length %zu", index, slice.len);
        
        asthra_safety_report_violation(ASTHRA_VIOLATION_MEMORY_SAFETY, ASTHRA_SAFETY_LEVEL_ENHANCED,
                                     check.error_details, __FILE__, __LINE__, __func__,
                                     &check, sizeof(check));
        return check;
    }
    
    // Additional safety checks for large indices
    if (index > SIZE_MAX / slice.element_size) {
        check.is_out_of_bounds = true;
        snprintf(check.error_details, sizeof(check.error_details),
                "Index %zu would cause arithmetic overflow with element size %zu", 
                index, slice.element_size);
        
        asthra_safety_report_violation(ASTHRA_VIOLATION_MEMORY_SAFETY, ASTHRA_SAFETY_LEVEL_ENHANCED,
                                     check.error_details, __FILE__, __LINE__, __func__,
                                     &check, sizeof(check));
        return check;
    }
    
    check.is_valid = true;
    snprintf(check.error_details, sizeof(check.error_details),
            "Slice access valid: index %zu in slice of length %zu", index, slice.len);
    
    return check;
}

// =============================================================================
// ENHANCED SLICE VALIDATION
// =============================================================================

AsthraMemoryLayoutValidation asthra_safety_validate_slice_header(AsthraSliceHeader slice) {
    AsthraMemoryLayoutValidation validation = {0};
    AsthraSafetyConfig config = asthra_safety_get_config();
    
    if (!config.enable_memory_layout_validation) {
        validation.is_valid = true;
        return validation;
    }
    
    // Check pointer validity
    validation.has_valid_pointer = (slice.ptr != NULL);
    
    // Check length consistency
    validation.has_consistent_length = (slice.len < SIZE_MAX / 2);
    
    // Check element size validity
    validation.has_correct_element_size = (slice.element_size > 0 && slice.element_size <= 1024);
    
    // Check capacity if available (assuming capacity field exists)
    validation.has_valid_capacity = true; // Simplified - would check slice.cap >= slice.len
    
    // Check alignment (simplified - assumes natural alignment)
    uintptr_t ptr_value = (uintptr_t)slice.ptr;
    size_t alignment = slice.element_size <= 8 ? slice.element_size : 8;
    validation.has_correct_alignment = (ptr_value % alignment == 0);
    
    // Overall validation
    validation.is_valid = validation.has_valid_pointer && 
                         validation.has_consistent_length && 
                         validation.has_correct_element_size && 
                         validation.has_valid_capacity && 
                         validation.has_correct_alignment;
    
    if (!validation.is_valid) {
        snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                "Slice header validation failed: ptr=%p, len=%zu, elem_size=%zu, aligned=%s",
                slice.ptr, slice.len, slice.element_size,
                validation.has_correct_alignment ? "yes" : "no");
        
        asthra_safety_report_violation(ASTHRA_VIOLATION_MEMORY_SAFETY, ASTHRA_SAFETY_LEVEL_ENHANCED,
                                     validation.corruption_details, __FILE__, __LINE__, __func__,
                                     &validation, sizeof(validation));
    }
    
    return validation;
}
