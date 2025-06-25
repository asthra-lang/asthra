/**
 * Asthra Programming Language Runtime v1.2 - Collections Module
 * Slice Operations and Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides slice management functionality including
 * slice creation, operations, and bounds checking.
 */

#ifndef ASTHRA_RUNTIME_SLICES_H
#define ASTHRA_RUNTIME_SLICES_H

#include "../core/asthra_runtime_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// FORMALIZED SLICE MANAGEMENT WITH C17 ENHANCEMENTS
// =============================================================================

// SliceHeader<T> - Core slice representation with C17 flexible array member
typedef struct {
    void *restrict ptr;  // C17 restrict for non-aliasing optimization
    size_t len;          // Number of elements
    size_t cap;          // Capacity (for mutable slices)
    size_t element_size; // Size of each element in bytes
    AsthraOwnershipHint ownership;
    bool is_mutable;
    uint32_t type_id; // Runtime type identifier
} AsthraSliceHeader;

// Compile-time validation of slice header structure
ASTHRA_STATIC_ASSERT(sizeof(AsthraSliceHeader) <= 64, "SliceHeader should fit in cache line");
ASTHRA_STATIC_ASSERT(alignof(AsthraSliceHeader) >= alignof(void *), "SliceHeader alignment");

// =============================================================================
// SLICE CREATION AND MANAGEMENT
// =============================================================================

// Slice creation and management with C17 compound literals
AsthraSliceHeader asthra_slice_new(size_t element_size, size_t len, AsthraOwnershipHint ownership);
AsthraSliceHeader asthra_slice_from_raw_parts(void *restrict ptr, size_t len, size_t element_size,
                                              bool is_mutable, AsthraOwnershipHint ownership);
AsthraSliceHeader asthra_slice_subslice(AsthraSliceHeader slice, size_t start, size_t end);
void asthra_slice_free(AsthraSliceHeader slice);

// =============================================================================
// SLICE OPERATIONS
// =============================================================================

// Slice operations with restrict pointers for optimization
void *asthra_slice_get_ptr(AsthraSliceHeader slice);
size_t asthra_slice_get_len(AsthraSliceHeader slice);
size_t asthra_slice_get_cap(AsthraSliceHeader slice);
size_t asthra_slice_get_element_size(AsthraSliceHeader slice);
bool asthra_slice_bounds_check(AsthraSliceHeader slice, size_t index);
void *asthra_slice_get_element(AsthraSliceHeader slice, size_t index);
int asthra_slice_set_element(AsthraSliceHeader slice, size_t index, const void *restrict element);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_SLICES_H
