/**
 * Asthra Programming Language Slice Types
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Basic slice types and operations.
 */

#ifndef ASTHRA_SLICE_H
#define ASTHRA_SLICE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Basic slice header structure
typedef struct {
    void *data;
    size_t len;
    size_t capacity;
    size_t element_size;
    bool is_gc_managed;
    uint32_t bounds_check_magic;
} SliceHeader;

// Basic slice operations
SliceHeader *slice_create(size_t element_size, size_t capacity);
void slice_destroy(SliceHeader *slice);
bool slice_bounds_check(const SliceHeader *slice, size_t index);
void *slice_get(const SliceHeader *slice, size_t index);
bool slice_set(SliceHeader *slice, size_t index, const void *value);
bool slice_push(SliceHeader *slice, const void *value);

#endif // ASTHRA_SLICE_H
