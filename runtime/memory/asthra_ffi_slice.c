/**
 * Asthra Safe C Memory Interface v1.0 - Slice Operations Implementation
 * Slice creation, management, bounds checking, and element access
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asthra_ffi_slice.h"
#include "asthra_ffi_memory_core.h"

// =============================================================================
// RESULT TYPE HELPER FUNCTIONS
// =============================================================================

// NOTE: Asthra_result_ok(), Asthra_result_err(), Asthra_result_is_ok(), and 
// Asthra_result_is_err() functions have been moved to asthra_ffi_memory_core.c 
// to eliminate code duplication. They are now shared across all FFI modules 
// that need result type functionality.

// =============================================================================
// SLICE CREATION AND MANAGEMENT
// =============================================================================

AsthraFFISliceHeader Asthra_slice_from_raw_parts(void* c_array_ptr, size_t len, 
                                              size_t element_size, bool is_mutable, 
                                              AsthraOwnershipTransfer ownership) {
    AsthraFFISliceHeader slice = {0};
    
    if (!c_array_ptr && len > 0) {
        return slice; // Invalid slice
    }
    
    slice.ptr = c_array_ptr;
    slice.len = len;
    slice.cap = len; // Assume capacity equals length for raw parts
    slice.element_size = element_size;
    slice.type_id = 0; // Unknown type
    slice.zone_hint = ASTHRA_ZONE_HINT_MANUAL; // Assume manual for raw parts
    slice.ownership = ownership;
    slice.is_mutable = is_mutable;
    slice.bounds_checking = true;
    slice.magic = ASTHRA_SLICE_MAGIC;
    
    extern AsthraFFIMemoryManager g_ffi_memory;
    pthread_mutex_lock(&g_ffi_memory.mutex);
    g_ffi_memory.stats.slice_count++;
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    
    return slice;
}

AsthraFFISliceHeader Asthra_slice_new(size_t element_size, size_t len, size_t cap, 
                                   AsthraMemoryZoneHint zone_hint) {
    AsthraFFISliceHeader slice = {0};
    
    if (element_size == 0 || cap < len) {
        return slice; // Invalid parameters
    }
    
    size_t total_size = element_size * cap;
    void *ptr = Asthra_ffi_alloc(total_size, zone_hint);
    if (!ptr) {
        return slice; // Allocation failed
    }
    
    slice.ptr = ptr;
    slice.len = len;
    slice.cap = cap;
    slice.element_size = element_size;
    slice.type_id = 0; // Unknown type
    slice.zone_hint = zone_hint;
    slice.ownership = ASTHRA_OWNERSHIP_TRANSFER_FULL;
    slice.is_mutable = true;
    slice.bounds_checking = true;
    slice.magic = ASTHRA_SLICE_MAGIC;
    
    extern AsthraFFIMemoryManager g_ffi_memory;
    pthread_mutex_lock(&g_ffi_memory.mutex);
    g_ffi_memory.stats.slice_count++;
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    
    return slice;
}

AsthraFFIResult Asthra_slice_subslice(AsthraFFISliceHeader slice, size_t start, size_t end) {
    if (slice.magic != ASTHRA_SLICE_MAGIC) {
        return Asthra_result_err(4, // ASTHRA_FFI_ERROR_INVALID_SLICE
                               "Invalid slice magic number", 
                               "Asthra_slice_subslice", NULL);
    }
    
    if (start > end || end > slice.len) {
        return Asthra_result_err(3, // ASTHRA_FFI_ERROR_BOUNDS_CHECK
                               "Invalid subslice range", 
                               "Asthra_slice_subslice", NULL);
    }
    
    AsthraFFISliceHeader *subslice = malloc(sizeof(AsthraFFISliceHeader));
    if (!subslice) {
        return Asthra_result_err(2, // ASTHRA_FFI_ERROR_OUT_OF_MEMORY
                               "Failed to allocate subslice", 
                               "Asthra_slice_subslice", NULL);
    }
    
    char *ptr = (char*)slice.ptr;
    subslice->ptr = ptr + (start * slice.element_size);
    subslice->len = end - start;
    subslice->cap = subslice->len;
    subslice->element_size = slice.element_size;
    subslice->type_id = slice.type_id;
    subslice->zone_hint = slice.zone_hint;
    subslice->ownership = ASTHRA_OWNERSHIP_TRANSFER_NONE; // Borrowed reference
    subslice->is_mutable = slice.is_mutable;
    subslice->bounds_checking = slice.bounds_checking;
    subslice->magic = ASTHRA_SLICE_MAGIC;
    
    return Asthra_result_ok(subslice, sizeof(AsthraFFISliceHeader), 0, 
                          ASTHRA_OWNERSHIP_TRANSFER_FULL);
}

void Asthra_slice_free(AsthraFFISliceHeader slice) {
    if (slice.magic != ASTHRA_SLICE_MAGIC) {
        return;
    }
    
    if (slice.ownership == ASTHRA_OWNERSHIP_TRANSFER_FULL && slice.ptr) {
        Asthra_ffi_free(slice.ptr, slice.zone_hint);
    }
    
    extern AsthraFFIMemoryManager g_ffi_memory;
    pthread_mutex_lock(&g_ffi_memory.mutex);
    if (g_ffi_memory.stats.slice_count > 0) {
        g_ffi_memory.stats.slice_count--;
    }
    pthread_mutex_unlock(&g_ffi_memory.mutex);
}

// =============================================================================
// SLICE PROPERTY ACCESS
// =============================================================================

void* Asthra_slice_get_ptr(AsthraFFISliceHeader slice) {
    if (slice.magic != ASTHRA_SLICE_MAGIC) {
        return NULL;
    }
    return slice.ptr;
}

size_t Asthra_slice_get_len(AsthraFFISliceHeader slice) {
    if (slice.magic != ASTHRA_SLICE_MAGIC) {
        return 0;
    }
    return slice.len;
}

size_t Asthra_slice_get_cap(AsthraFFISliceHeader slice) {
    if (slice.magic != ASTHRA_SLICE_MAGIC) {
        return 0;
    }
    return slice.cap;
}

size_t Asthra_slice_get_element_size(AsthraFFISliceHeader slice) {
    if (slice.magic != ASTHRA_SLICE_MAGIC) {
        return 0;
    }
    return slice.element_size;
}

// =============================================================================
// SLICE BOUNDS CHECKING AND VALIDATION
// =============================================================================

bool Asthra_slice_is_valid(AsthraFFISliceHeader slice) {
    return slice.magic == ASTHRA_SLICE_MAGIC && 
           slice.element_size > 0 && 
           slice.len <= slice.cap &&
           (slice.ptr != NULL || slice.len == 0);
}

AsthraFFIResult Asthra_slice_bounds_check(AsthraFFISliceHeader slice, size_t index) {
    if (slice.magic != ASTHRA_SLICE_MAGIC) {
        return Asthra_result_err(4, // ASTHRA_FFI_ERROR_INVALID_SLICE
                               "Invalid slice magic number", 
                               "Asthra_slice_bounds_check", NULL);
    }
    
    if (index >= slice.len) {
        return Asthra_result_err(3, // ASTHRA_FFI_ERROR_BOUNDS_CHECK
                               "Index out of bounds", 
                               "Asthra_slice_bounds_check", NULL);
    }
    
    return Asthra_result_ok(NULL, 0, 0, ASTHRA_OWNERSHIP_TRANSFER_NONE);
}

// =============================================================================
// SLICE ELEMENT ACCESS
// =============================================================================

AsthraFFIResult Asthra_slice_get_element(AsthraFFISliceHeader slice, size_t index, void* out_element) {
    if (!out_element) {
        return Asthra_result_err(1, // ASTHRA_FFI_ERROR_NULL_POINTER
                               "Output element pointer is NULL", 
                               "Asthra_slice_get_element", NULL);
    }
    
    AsthraFFIResult bounds_result = Asthra_slice_bounds_check(slice, index);
    if (Asthra_result_is_err(bounds_result)) {
        return bounds_result;
    }
    
    char *ptr = (char*)slice.ptr;
    memcpy(out_element, ptr + (index * slice.element_size), slice.element_size);
    
    return Asthra_result_ok(out_element, slice.element_size, slice.type_id, 
                          ASTHRA_OWNERSHIP_TRANSFER_NONE);
}

AsthraFFIResult Asthra_slice_set_element(AsthraFFISliceHeader slice, size_t index, const void* element) {
    if (!element) {
        return Asthra_result_err(1, // ASTHRA_FFI_ERROR_NULL_POINTER
                               "Element pointer is NULL", 
                               "Asthra_slice_set_element", NULL);
    }
    
    if (!slice.is_mutable) {
        return Asthra_result_err(5, // ASTHRA_FFI_ERROR_OWNERSHIP
                               "Slice is not mutable", 
                               "Asthra_slice_set_element", NULL);
    }
    
    AsthraFFIResult bounds_result = Asthra_slice_bounds_check(slice, index);
    if (Asthra_result_is_err(bounds_result)) {
        return bounds_result;
    }
    
    char *ptr = (char*)slice.ptr;
    memcpy(ptr + (index * slice.element_size), element, slice.element_size);
    
    return Asthra_result_ok(NULL, 0, 0, ASTHRA_OWNERSHIP_TRANSFER_NONE);
}

// =============================================================================
// SECURE SLICE OPERATIONS
// =============================================================================

void Asthra_secure_zero_slice(AsthraFFISliceHeader slice_ref) {
    if (slice_ref.magic != ASTHRA_SLICE_MAGIC || !slice_ref.ptr) {
        return;
    }
    
    Asthra_secure_zero(slice_ref.ptr, slice_ref.len * slice_ref.element_size);
} 
