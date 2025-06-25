/**
 * Asthra Programming Language v1.2 Enhanced FFI Tests
 * Memory Safety Testing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for memory safety features including bounds checking, leak prevention,
 * and double-free prevention.
 */

#include "../core/test_comprehensive.h"
#include "../runtime/ffi.h"
#include "../runtime/memory.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// HELPER STRUCTURES AND FUNCTIONS
// =============================================================================

typedef struct {
    bool memory_managed;
    size_t data_size;
    void *cleanup_func;
} FFIMemoryContext;

typedef void (*cleanup_c_func_t)(void *);

static void test_c_cleanup(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

// Track allocations for testing
static int allocation_count = 0;
static int deallocation_count = 0;

static void *tracked_malloc(size_t size) {
    allocation_count++;
    return malloc(size);
}

static void tracked_free(void *ptr) {
    if (ptr) {
        deallocation_count++;
        free(ptr);
    }
}

static bool safe_array_get(const int *arr, size_t arr_size, size_t index, int *out_value) {
    if (!arr || !out_value || index >= arr_size) {
        return false;
    }

    *out_value = arr[index];
    return true;
}

static void *auto_managed_alloc(size_t size, FFIMemoryContext *ctx) {
    void *ptr = tracked_malloc(size);
    if (ptr && ctx->memory_managed) {
        ctx->data_size = size;
    }
    return ptr;
}

// Safe pointer management for double-free prevention
typedef struct {
    void *ptr;
    bool is_freed;
} SafePointer;

static SafePointer safe_pointers[10];
static int safe_pointer_count = 0;

static SafePointer *safe_malloc(size_t size) {
    if (safe_pointer_count >= 10)
        return NULL;

    SafePointer *safe_ptr = &safe_pointers[safe_pointer_count++];
    safe_ptr->ptr = malloc(size);
    safe_ptr->is_freed = false;

    return safe_ptr;
}

static bool safe_free(SafePointer *safe_ptr) {
    if (!safe_ptr || safe_ptr->is_freed) {
        return false; // Already freed or invalid
    }

    free(safe_ptr->ptr);
    safe_ptr->ptr = NULL;
    safe_ptr->is_freed = true;
    return true;
}

// =============================================================================
// MEMORY SAFETY TESTS
// =============================================================================

AsthraTestResult test_ffi_memory_safety_bounds_checking(AsthraV12TestContext *ctx) {
    // Test memory safety with bounds checking

    // Test safe array access
    int safe_array[10];
    for (int i = 0; i < 10; i++) {
        safe_array[i] = i * i;
    }

    // Test valid access
    int value;
    bool success = safe_array_get(safe_array, 10, 5, &value);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, success,
                            "Safe array access should succeed for valid index")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, value == 25, "Safe array value should be 25, got %d",
                            value)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test invalid access
    success = safe_array_get(safe_array, 10, 15, &value);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, !success,
                            "Safe array access should fail for invalid index")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test NULL pointer safety
    success = safe_array_get(NULL, 10, 0, &value);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, !success, "Safe array access should fail for NULL array")) {
        return ASTHRA_TEST_FAIL;
    }

    success = safe_array_get(safe_array, 10, 0, NULL);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, !success,
                            "Safe array access should fail for NULL output")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ffi_memory_safety_leak_prevention(AsthraV12TestContext *ctx) {
    // Test memory leak prevention in FFI

    // Reset counters
    allocation_count = 0;
    deallocation_count = 0;

    // Test allocation/deallocation cycle
    const int num_allocations = 10;
    void *ptrs[num_allocations];

    // Allocate
    for (int i = 0; i < num_allocations; i++) {
        ptrs[i] = tracked_malloc(64);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, ptrs[i] != NULL, "Allocation %d should succeed", i)) {
            // Cleanup partial allocations
            for (int j = 0; j < i; j++) {
                tracked_free(ptrs[j]);
            }
            return ASTHRA_TEST_FAIL;
        }
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, allocation_count == num_allocations,
                            "Should have %d allocations, got %d", num_allocations,
                            allocation_count)) {
        // Cleanup
        for (int i = 0; i < num_allocations; i++) {
            tracked_free(ptrs[i]);
        }
        return ASTHRA_TEST_FAIL;
    }

    // Deallocate
    for (int i = 0; i < num_allocations; i++) {
        tracked_free(ptrs[i]);
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, deallocation_count == num_allocations,
                            "Should have %d deallocations, got %d", num_allocations,
                            deallocation_count)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test automatic cleanup with context
    FFIMemoryContext cleanup_ctx = {
        .memory_managed = true, .data_size = 0, .cleanup_func = tracked_free};

    int initial_alloc_count = allocation_count;
    void *managed_ptr = auto_managed_alloc(128, &cleanup_ctx);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, managed_ptr != NULL, "Managed allocation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, allocation_count == initial_alloc_count + 1,
                            "Allocation count should increase by 1")) {
        tracked_free(managed_ptr);
        return ASTHRA_TEST_FAIL;
    }

    // Cleanup using context
    int initial_dealloc_count = deallocation_count;
    if (cleanup_ctx.cleanup_func) {
        ((cleanup_c_func_t)cleanup_ctx.cleanup_func)(managed_ptr);
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, deallocation_count == initial_dealloc_count + 1,
                            "Deallocation count should increase by 1")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ffi_memory_safety_double_free_prevention(AsthraV12TestContext *ctx) {
    // Test double-free prevention

    // Reset
    safe_pointer_count = 0;

    // Test normal allocation and free
    SafePointer *ptr1 = safe_malloc(64);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, ptr1 != NULL, "Safe allocation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, ptr1->ptr != NULL,
                            "Safe pointer should have valid memory")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, !ptr1->is_freed,
                            "Safe pointer should not be marked as freed initially")) {
        return ASTHRA_TEST_FAIL;
    }

    // First free should succeed
    bool free_result = safe_free(ptr1);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, free_result, "First free should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, ptr1->is_freed, "Pointer should be marked as freed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Second free should fail (double-free prevention)
    free_result = safe_free(ptr1);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, !free_result,
                            "Second free should fail (double-free prevention)")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test freeing NULL-like pointer
    free_result = safe_free(NULL);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, !free_result, "Freeing NULL should fail safely")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}
