/**
 * Asthra Programming Language Slice Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Basic slice implementation with stubs.
 */

#include "slice.h"
#include <string.h>

#define SLICE_BOUNDS_MAGIC 0xDEADBEEF

SliceHeader* slice_create(size_t element_size, size_t capacity) {
    SliceHeader *slice = malloc(sizeof(SliceHeader));
    if (!slice) return NULL;
    
    slice->data = malloc(element_size * capacity);
    if (!slice->data) {
        free(slice);
        return NULL;
    }
    
    slice->len = 0;
    slice->capacity = capacity;
    slice->element_size = element_size;
    slice->is_gc_managed = false;
    slice->bounds_check_magic = SLICE_BOUNDS_MAGIC;
    
    return slice;
}

void slice_destroy(SliceHeader *slice) {
    if (!slice) return;
    
    if (slice->data) {
        free(slice->data);
    }
    free(slice);
}

bool slice_bounds_check(const SliceHeader *slice, size_t index) {
    if (!slice || slice->bounds_check_magic != SLICE_BOUNDS_MAGIC) {
        return false;
    }
    
    return index < slice->len;
}

void* slice_get(const SliceHeader *slice, size_t index) {
    if (!slice_bounds_check(slice, index)) {
        return NULL;
    }
    
    return (char*)slice->data + (index * slice->element_size);
}

bool slice_set(SliceHeader *slice, size_t index, const void *value) {
    if (!slice_bounds_check(slice, index) || !value) {
        return false;
    }
    
    void *dest = (char*)slice->data + (index * slice->element_size);
    memcpy(dest, value, slice->element_size);
    return true;
}

bool slice_push(SliceHeader *slice, const void *value) {
    if (!slice || !value || slice->len >= slice->capacity) {
        return false;
    }
    
    void *dest = (char*)slice->data + (slice->len * slice->element_size);
    memcpy(dest, value, slice->element_size);
    slice->len++;
    return true;
} 
