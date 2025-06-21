/**
 * Asthra Programming Language Runtime Safety System
 * Boundary and Memory Validation Module - Memory safety and bounds checking
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_SAFETY_BOUNDS_H
#define ASTHRA_SAFETY_BOUNDS_H

#include "asthra_safety_minimal_includes.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BOUNDARY CHECK STRUCTURES
// =============================================================================

// Enhanced boundary checks for slices
typedef struct {
    bool is_valid;
    size_t attempted_index;
    size_t slice_length;
    size_t element_size;
    bool is_out_of_bounds;
    bool is_null_pointer;
    bool is_corrupted_header;
    char error_details[256];
} AsthraBoundaryCheck;

// Memory layout validation for SliceHeader
typedef struct {
    bool is_valid;
    bool has_correct_alignment;
    bool has_valid_pointer;
    bool has_consistent_length;
    bool has_valid_capacity;
    bool has_correct_element_size;
    size_t detected_corruption_offset;
    char corruption_details[256];
} AsthraMemoryLayoutValidation;

// =============================================================================
// BOUNDARY AND MEMORY VALIDATION FUNCTIONS
// =============================================================================

/**
 * Enhanced boundary check for slice access
 */
AsthraBoundaryCheck asthra_safety_enhanced_boundary_check(AsthraSliceHeader slice, size_t index);

/**
 * Basic slice bounds checking
 */
AsthraBoundaryCheck asthra_safety_slice_bounds_check(AsthraSliceHeader slice, size_t index);

/**
 * Validate slice header memory layout
 */
AsthraMemoryLayoutValidation asthra_safety_validate_slice_header(AsthraSliceHeader slice);

/**
 * Check memory alignment for given pointer and alignment requirements
 */
bool asthra_safety_check_memory_alignment(void *ptr, size_t alignment);

/**
 * Validate memory region accessibility
 */
bool asthra_safety_validate_memory_region(void *ptr, size_t size);

/**
 * Check for memory corruption in slice data
 */
bool asthra_safety_check_slice_corruption(AsthraSliceHeader slice);

/**
 * Validate pointer for null and basic sanity checks
 */
bool asthra_safety_validate_pointer(void *ptr, size_t expected_min_size);

/**
 * Check for integer overflow in size calculations
 */
bool asthra_safety_check_size_overflow(size_t a, size_t b, size_t *result);

/**
 * Validate capacity and length consistency
 */
bool asthra_safety_validate_capacity_consistency(size_t length, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_BOUNDS_H 
