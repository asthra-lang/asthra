/**
 * Asthra Safe C Memory Interface v1.0 - Slice Operations
 * Slice creation, management, bounds checking, and element access
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_SLICE_H
#define ASTHRA_FFI_SLICE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "asthra_ffi_memory_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASTHRA_SLICE_MAGIC 0x534C4943 // "SLIC" in hex

// =============================================================================
// SLICE TYPES AND STRUCTURES
// =============================================================================

/**
 * Enhanced FFI slice header with comprehensive metadata
 */
typedef struct {
    void *ptr;                         // Pointer to data
    size_t len;                        // Number of elements
    size_t cap;                        // Capacity (for mutable slices)
    size_t element_size;               // Size of each element in bytes
    uint32_t type_id;                  // Runtime type identifier
    AsthraMemoryZoneHint zone_hint;    // Memory zone hint
    AsthraOwnershipTransfer ownership; // Ownership semantics
    bool is_mutable;                   // Mutability flag
    bool bounds_checking;              // Enable runtime bounds checking
    uint32_t magic;                    // Magic number for validation
} AsthraFFISliceHeader;

// =============================================================================
// SLICE CREATION AND MANAGEMENT
// =============================================================================

/**
 * Create slice from raw C array pointer and length
 * @param c_array_ptr Pointer to C array
 * @param len Number of elements
 * @param element_size Size of each element in bytes
 * @param is_mutable Whether slice is mutable
 * @param ownership Ownership transfer semantics
 * @return Initialized slice header
 */
AsthraFFISliceHeader Asthra_slice_from_raw_parts(void *c_array_ptr, size_t len, size_t element_size,
                                                 bool is_mutable,
                                                 AsthraOwnershipTransfer ownership);

/**
 * Create new slice with specified capacity
 * @param element_size Size of each element in bytes
 * @param len Initial length
 * @param cap Initial capacity
 * @param zone_hint Memory zone hint for allocation
 * @return New slice header
 */
AsthraFFISliceHeader Asthra_slice_new(size_t element_size, size_t len, size_t cap,
                                      AsthraMemoryZoneHint zone_hint);

/**
 * Create subslice from existing slice
 * @param slice Source slice
 * @param start Start index (inclusive)
 * @param end End index (exclusive)
 * @return Subslice result
 */
AsthraFFIResult Asthra_slice_subslice(AsthraFFISliceHeader slice, size_t start, size_t end);

/**
 * Free slice memory
 * @param slice Slice to free
 */
void Asthra_slice_free(AsthraFFISliceHeader slice);

// =============================================================================
// SLICE PROPERTY ACCESS
// =============================================================================

/**
 * Get slice data pointer
 * @param slice Slice to query
 * @return Data pointer
 */
void *Asthra_slice_get_ptr(AsthraFFISliceHeader slice);

/**
 * Get slice length
 * @param slice Slice to query
 * @return Number of elements
 */
size_t Asthra_slice_get_len(AsthraFFISliceHeader slice);

/**
 * Get slice capacity
 * @param slice Slice to query
 * @return Capacity in elements
 */
size_t Asthra_slice_get_cap(AsthraFFISliceHeader slice);

/**
 * Get element size
 * @param slice Slice to query
 * @return Size of each element in bytes
 */
size_t Asthra_slice_get_element_size(AsthraFFISliceHeader slice);

// =============================================================================
// SLICE BOUNDS CHECKING AND VALIDATION
// =============================================================================

/**
 * Check if slice is valid
 * @param slice Slice to validate
 * @return true if valid, false otherwise
 */
bool Asthra_slice_is_valid(AsthraFFISliceHeader slice);

/**
 * Perform bounds check on slice access
 * @param slice Slice to check
 * @param index Index to validate
 * @return Result indicating success or failure
 */
AsthraFFIResult Asthra_slice_bounds_check(AsthraFFISliceHeader slice, size_t index);

// =============================================================================
// SLICE ELEMENT ACCESS
// =============================================================================

/**
 * Get element from slice
 * @param slice Source slice
 * @param index Element index
 * @param out_element Output buffer for element
 * @return Result indicating success or failure
 */
AsthraFFIResult Asthra_slice_get_element(AsthraFFISliceHeader slice, size_t index,
                                         void *out_element);

/**
 * Set element in slice
 * @param slice Target slice
 * @param index Element index
 * @param element Element data to set
 * @return Result indicating success or failure
 */
AsthraFFIResult Asthra_slice_set_element(AsthraFFISliceHeader slice, size_t index,
                                         const void *element);

// =============================================================================
// SECURE SLICE OPERATIONS
// =============================================================================

/**
 * Securely zero slice contents
 * @param slice_ref Slice to zero
 */
void Asthra_secure_zero_slice(AsthraFFISliceHeader slice_ref);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_SLICE_H
