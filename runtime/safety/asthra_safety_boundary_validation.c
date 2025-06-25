/**
 * Asthra Programming Language Runtime Safety - Boundary & Memory Validation
 * Implementation of enhanced boundary checks and memory layout validation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_safety_boundary_validation.h"
#include "asthra_runtime.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// External references to global state from asthra_safety.c
extern AsthraSafetyConfig g_safety_config;

// =============================================================================
// ENHANCED BOUNDARY CHECKS IMPLEMENTATION
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
        snprintf(check.error_details, sizeof(check.error_details), "Slice has null pointer");
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
                 "Integer overflow in offset calculation: index=%zu, element_size=%zu", index,
                 slice.element_size);
        return check;
    }

    check.is_valid = true;
    return check;
}

// =============================================================================
// MEMORY LAYOUT VALIDATION IMPLEMENTATION
// =============================================================================

AsthraMemoryLayoutValidation asthra_safety_validate_slice_header(AsthraSliceHeader slice) {
    AsthraMemoryLayoutValidation validation = {0};

    if (!g_safety_config.enable_memory_layout_validation) {
        validation.is_valid = true;
        return validation;
    }

    // Check alignment
    uintptr_t ptr_addr = (uintptr_t)slice.ptr;
    if (slice.ptr && (ptr_addr % sizeof(void *)) != 0) {
        validation.has_correct_alignment = false;
        snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                 "Pointer not properly aligned: %p", slice.ptr);
    } else {
        validation.has_correct_alignment = true;
    }

    // Check pointer validity
    if (slice.len > 0 && !slice.ptr) {
        validation.has_valid_pointer = false;
        snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                 "Null pointer with non-zero length %zu", slice.len);
    } else {
        validation.has_valid_pointer = true;
    }

    // Check length consistency
    if (slice.len <= slice.cap) {
        validation.has_consistent_length = true;
    } else {
        validation.has_consistent_length = false;
        validation.detected_corruption_offset = offsetof(AsthraSliceHeader, len);
        snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                 "Length %zu exceeds capacity %zu", slice.len, slice.cap);
    }

    // Check capacity validity
    if (slice.cap < SIZE_MAX / 2) {
        validation.has_valid_capacity = true;
    } else {
        validation.has_valid_capacity = false;
        validation.detected_corruption_offset = offsetof(AsthraSliceHeader, cap);
        snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                 "Suspicious capacity value: %zu", slice.cap);
    }

    // Check element size
    if (slice.element_size > 0 && slice.element_size <= 1024 * 1024) {
        validation.has_correct_element_size = true;
    } else {
        validation.has_correct_element_size = false;
        validation.detected_corruption_offset = offsetof(AsthraSliceHeader, element_size);
        snprintf(validation.corruption_details, sizeof(validation.corruption_details),
                 "Invalid element size: %zu", slice.element_size);
    }

    validation.is_valid = validation.has_correct_alignment && validation.has_valid_pointer &&
                          validation.has_consistent_length && validation.has_valid_capacity &&
                          validation.has_correct_element_size;

    return validation;
}
