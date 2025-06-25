/**
 * Asthra Programming Language v1.2 Slice Management Test Common Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of shared utilities for slice management tests.
 */

#include "slice_test_common.h"

// =============================================================================
// TEST HELPER IMPLEMENTATIONS
// =============================================================================

TestSliceHeader *test_slice_create(size_t element_size, size_t capacity) {
    TestSliceHeader *slice = malloc(sizeof(TestSliceHeader));
    if (!slice)
        return NULL;

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

void test_slice_destroy(TestSliceHeader *slice) {
    if (!slice)
        return;

    if (slice->data) {
        free(slice->data);
    }
    free(slice);
}

bool test_slice_bounds_check(const TestSliceHeader *slice, size_t index) {
    if (!slice || slice->bounds_check_magic != SLICE_BOUNDS_MAGIC) {
        return false;
    }

    return index < slice->len;
}

void *test_slice_get(const TestSliceHeader *slice, size_t index) {
    if (!test_slice_bounds_check(slice, index)) {
        return NULL;
    }

    return (char *)slice->data + (index * slice->element_size);
}

bool test_slice_set(TestSliceHeader *slice, size_t index, const void *value) {
    if (!test_slice_bounds_check(slice, index) || !value) {
        return false;
    }

    void *dest = (char *)slice->data + (index * slice->element_size);
    memcpy(dest, value, slice->element_size);
    return true;
}

bool test_slice_push(TestSliceHeader *slice, const void *value) {
    if (!slice || !value || slice->len >= slice->capacity) {
        return false;
    }

    void *dest = (char *)slice->data + (slice->len * slice->element_size);
    memcpy(dest, value, slice->element_size);
    slice->len++;
    return true;
}
