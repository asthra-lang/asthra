/**
 * Asthra Programming Language v1.2 Slice Management Test Common Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Shared utilities and data structures for slice management tests.
 */

#ifndef SLICE_TEST_COMMON_H
#define SLICE_TEST_COMMON_H

#include "../../runtime/memory.h"
#include "../../runtime/slice.h"
#include "../core/test_comprehensive.h"
#include "../core/test_comprehensive_core.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Forward declarations for benchmark functions (implemented in concurrency_benchmark_stubs.c)
void asthra_benchmark_start(AsthraV12TestContext *ctx);
void asthra_benchmark_iteration(AsthraV12TestContext *ctx);
void asthra_benchmark_end(AsthraV12TestContext *ctx);
uint64_t asthra_test_get_time_ns(void);

// =============================================================================
// TEST DATA STRUCTURES AND HELPERS
// =============================================================================

// Mock SliceHeader structure for testing
typedef struct {
    void *data;
    size_t len;
    size_t capacity;
    size_t element_size;
    bool is_gc_managed;
    uint32_t bounds_check_magic;
} TestSliceHeader;

#define SLICE_BOUNDS_MAGIC 0xDEADBEEF

// Function declarations
TestSliceHeader *test_slice_create(size_t element_size, size_t capacity);
void test_slice_destroy(TestSliceHeader *slice);
bool test_slice_bounds_check(const TestSliceHeader *slice, size_t index);
void *test_slice_get(const TestSliceHeader *slice, size_t index);
bool test_slice_set(TestSliceHeader *slice, size_t index, const void *value);
bool test_slice_push(TestSliceHeader *slice, const void *value);

#endif // SLICE_TEST_COMMON_H
