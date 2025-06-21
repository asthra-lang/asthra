/**
 * Asthra Programming Language Runtime Safety System
 * Boundary and Memory Validation Module - Memory safety and bounds checking implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_safety_bounds.h"
#include "../asthra_safety_common.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

// External references to safety configuration
// Use accessor function instead of direct global
#define g_safety_config (*asthra_safety_get_config_ptr())

// =============================================================================
// BOUNDARY AND MEMORY VALIDATION IMPLEMENTATION
// =============================================================================

AsthraBoundaryCheck asthra_safety_enhanced_boundary_check(AsthraSliceHeader slice, size_t index) {
    AsthraBoundaryCheck check = {0};
    
    if (!g_safety_config.enable_boundary_checks) {
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
        return check;
    }
    
    // Check for corrupted header
    if (slice.element_size == 0 || slice.element_size > 1024 * 1024) {
        check.is_corrupted_header = true;
        snprintf(check.error_details, sizeof(check.error_details),
                "Suspicious element size: %zu bytes", slice.element_size);
        return check;
    }
    
    // Check capacity consistency
    if (slice.cap < slice.len) {
        check.is_corrupted_header = true;
        snprintf(check.error_details, sizeof(check.error_details),
                "Capacity %zu is less than length %zu", slice.cap, slice.len);
        return check;
    }
    
    // Check bounds
    if (index >= slice.len) {
        check.is_out_of_bounds = true;
        snprintf(check.error_details, sizeof(check.error_details),
                "Index %zu out of bounds for slice of length %zu", index, slice.len);
        return check;
    }
    
    // Check for potential integer overflow in address calculation
    size_t offset = index * slice.element_size;
    if (offset / slice.element_size != index) {
        check.is_out_of_bounds = true;
        snprintf(check.error_details, sizeof(check.error_details),
                "Integer overflow in offset calculation: index=%zu, element_size=%zu", 
                index, slice.element_size);
        return check;
    }
    
    check.is_valid = true;
    return check;
}

AsthraBoundaryCheck asthra_safety_slice_bounds_check(AsthraSliceHeader slice, size_t index) {
    AsthraBoundaryCheck check = {0};
    
    if (!g_safety_config.enable_slice_bounds_checking) {
        check.is_valid = true;
        return check;
    }
    
    check.attempted_index = index;
    check.slice_length = slice.len;
    check.element_size = slice.element_size;
    
    if (!slice.ptr) {
        check.is_null_pointer = true;
        snprintf(check.error_details, sizeof(check.error_details),
                "Slice has null pointer");
        return check;
    }
    
    if (index >= slice.len) {
        check.is_out_of_bounds = true;
        snprintf(check.error_details, sizeof(check.error_details),
                "Index %zu out of bounds for slice of length %zu", index, slice.len);
        return check;
    }
    
    check.is_valid = true;
    return check;
}

AsthraMemoryLayoutValidation asthra_safety_validate_slice_header(AsthraSliceHeader slice) {
    AsthraMemoryLayoutValidation validation = {0};
    
    if (!g_safety_config.enable_memory_layout_validation) {
        validation.is_valid = true;
        return validation;
    }
    
    validation.is_valid = true;
    
    // Check pointer alignment
    if (slice.ptr && slice.element_size > 0) {
        uintptr_t ptr_val = (uintptr_t)slice.ptr;
        size_t alignment = slice.element_size >= 8 ? 8 : slice.element_size;
        validation.has_correct_alignment = (ptr_val % alignment == 0);
        
        if (!validation.has_correct_alignment) {
            validation.is_valid = false;
            snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                    "Pointer %p not aligned to %zu bytes", slice.ptr, alignment);
        }
    } else {
        validation.has_correct_alignment = (slice.ptr == NULL);
    }
    
    // Check pointer validity
    validation.has_valid_pointer = (slice.ptr != NULL || slice.len == 0);
    if (!validation.has_valid_pointer) {
        validation.is_valid = false;
        snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                "Null pointer with non-zero length %zu", slice.len);
    }
    
    // Check length consistency
    validation.has_consistent_length = (slice.len <= slice.cap);
    if (!validation.has_consistent_length) {
        validation.is_valid = false;
        validation.detected_corruption_offset = offsetof(AsthraSliceHeader, len);
        snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                "Length %zu exceeds capacity %zu", slice.len, slice.cap);
    }
    
    // Check capacity validity
    validation.has_valid_capacity = (slice.cap < SIZE_MAX / slice.element_size);
    if (!validation.has_valid_capacity) {
        validation.is_valid = false;
        validation.detected_corruption_offset = offsetof(AsthraSliceHeader, cap);
        snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                "Capacity %zu too large for element size %zu", slice.cap, slice.element_size);
    }
    
    // Check element size validity
    validation.has_correct_element_size = (slice.element_size > 0 && slice.element_size <= 1024 * 1024);
    if (!validation.has_correct_element_size) {
        validation.is_valid = false;
        validation.detected_corruption_offset = offsetof(AsthraSliceHeader, element_size);
        snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                "Invalid element size: %zu bytes", slice.element_size);
    }
    
    return validation;
}

bool asthra_safety_check_memory_alignment(void *ptr, size_t alignment) {
    if (!g_safety_config.enable_memory_layout_validation || !ptr) {
        return ptr == NULL; // null pointer is considered "aligned"
    }
    
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return false; // alignment must be a power of 2
    }
    
    uintptr_t ptr_val = (uintptr_t)ptr;
    return (ptr_val % alignment == 0);
}

bool asthra_safety_validate_memory_region(void *ptr, size_t size) {
    if (!g_safety_config.enable_memory_layout_validation) {
        return true;
    }
    
    if (!ptr) {
        return size == 0;
    }
    
    if (size == 0) {
        return true;
    }
    
    // Check for address space wrap-around
    uintptr_t start = (uintptr_t)ptr;
    uintptr_t end = start + size;
    if (end < start) {
        return false; // overflow
    }
    
    // Basic sanity check: ensure we're not dealing with obviously invalid addresses
    // This is a heuristic and may need platform-specific adjustments
    if (start < 4096) {
        return false; // likely null pointer dereference area
    }
    
    return true;
}

bool asthra_safety_check_slice_corruption(AsthraSliceHeader slice) {
    if (!g_safety_config.enable_memory_layout_validation) {
        return false; // assume no corruption if checks disabled
    }
    
    AsthraMemoryLayoutValidation validation = asthra_safety_validate_slice_header(slice);
    return !validation.is_valid;
}

bool asthra_safety_validate_pointer(void *ptr, size_t expected_min_size) {
    if (!g_safety_config.enable_boundary_checks) {
        return true;
    }
    
    if (!ptr) {
        return expected_min_size == 0;
    }
    
    // Basic pointer sanity checks
    uintptr_t ptr_val = (uintptr_t)ptr;
    
    // Check for obviously invalid addresses
    if (ptr_val < 4096) {
        return false; // likely null pointer dereference area
    }
    
    // Check for address space wrap-around with expected size
    if (expected_min_size > 0) {
        uintptr_t end = ptr_val + expected_min_size;
        if (end < ptr_val) {
            return false; // overflow
        }
    }
    
    return true;
}

bool asthra_safety_check_size_overflow(size_t a, size_t b, size_t *result) {
    if (!g_safety_config.enable_boundary_checks) {
        if (result) *result = a + b;
        return false; // assume no overflow if checks disabled
    }
    
    // Check for addition overflow
    if (a > SIZE_MAX - b) {
        return true; // overflow detected
    }
    
    if (result) {
        *result = a + b;
    }
    
    return false; // no overflow
}

bool asthra_safety_validate_capacity_consistency(size_t length, size_t capacity) {
    if (!g_safety_config.enable_memory_layout_validation) {
        return true;
    }
    
    return length <= capacity;
} 
