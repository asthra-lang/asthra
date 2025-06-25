/**
 * Asthra Programming Language Runtime v1.2
 * Memory Management Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_common.h"

// =============================================================================
// MEMORY MANAGEMENT TESTS
// =============================================================================

int test_memory_zones(void) {
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization failed");

    // Test GC zone allocation
    void *gc_ptr = asthra_alloc(1024, ASTHRA_ZONE_GC);
    TEST_ASSERT(gc_ptr != NULL, "GC allocation failed");

    // Test manual zone allocation
    void *manual_ptr = asthra_alloc(512, ASTHRA_ZONE_MANUAL);
    TEST_ASSERT(manual_ptr != NULL, "Manual allocation failed");

    // Test pinned zone allocation
    void *pinned_ptr = asthra_alloc(256, ASTHRA_ZONE_PINNED);
    TEST_ASSERT(pinned_ptr != NULL, "Pinned allocation failed");

    // Test zeroed allocation
    void *zeroed_ptr = asthra_alloc_zeroed(128, ASTHRA_ZONE_GC);
    TEST_ASSERT(zeroed_ptr != NULL, "Zeroed allocation failed");

    // Verify memory is zeroed
    uint8_t *bytes = (uint8_t *)zeroed_ptr;
    for (int i = 0; i < 128; i++) {
        TEST_ASSERT(bytes[i] == 0, "Memory should be zeroed");
    }

    // Test freeing
    asthra_free(manual_ptr, ASTHRA_ZONE_MANUAL);
    asthra_free(pinned_ptr, ASTHRA_ZONE_PINNED);

    asthra_runtime_cleanup();
    TEST_PASS("Memory zone management");
}

int test_gc_operations(void) {
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization failed");

    // Test GC collection
    asthra_gc_collect();

    // Test heap size queries
    size_t heap_size = asthra_gc_get_heap_size();
    size_t used_memory = asthra_gc_get_used_memory();

    printf("  Heap size: %zu bytes, Used: %zu bytes\n", heap_size, used_memory);

    asthra_runtime_cleanup();
    TEST_PASS("Garbage collection operations");
}

// Test function declarations for external use
int test_memory_zones(void);
int test_gc_operations(void);
