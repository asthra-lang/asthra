/**
 * Test Suite for Asthra Safe C Memory Interface - Basic Allocation
 * Tests for allocation, reallocation, and zone management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_ffi_memory_common.h"

// =============================================================================
// BASIC MEMORY ALLOCATION TESTS
// =============================================================================

void test_basic_allocation(void) {
    TEST_SECTION("Basic Memory Allocation");

    // Test basic allocation
    void *ptr1 = Asthra_ffi_alloc(1024, ASTHRA_ZONE_HINT_MANUAL);
    TEST_ASSERT(ptr1 != NULL, "Basic allocation succeeds");

    // Test zeroed allocation
    void *ptr2 = Asthra_ffi_alloc_zeroed(512, ASTHRA_ZONE_HINT_MANUAL);
    TEST_ASSERT(ptr2 != NULL, "Zeroed allocation succeeds");

    // Verify zeroed memory
    uint8_t *bytes = (uint8_t *)ptr2;
    bool is_zeroed = true;
    for (size_t i = 0; i < 512; i++) {
        if (bytes[i] != 0) {
            is_zeroed = false;
            break;
        }
    }
    TEST_ASSERT(is_zeroed, "Zeroed allocation is actually zeroed");

    // Test reallocation
    void *ptr3 = Asthra_ffi_realloc(ptr1, 2048, ASTHRA_ZONE_HINT_MANUAL);
    TEST_ASSERT(ptr3 != NULL, "Reallocation succeeds");

    // Test zone migration
    void *ptr4 = Asthra_ffi_realloc(ptr3, 1024, ASTHRA_ZONE_HINT_GC);
    TEST_ASSERT(ptr4 != NULL, "Zone migration succeeds");

    // Clean up
    Asthra_ffi_free(ptr2, ASTHRA_ZONE_HINT_MANUAL);
    Asthra_ffi_free(ptr4, ASTHRA_ZONE_HINT_GC);

    // Test null pointer handling
    Asthra_ffi_free(NULL, ASTHRA_ZONE_HINT_MANUAL);
    TEST_ASSERT(true, "NULL pointer free handled gracefully");
}

void test_zone_allocation(void) {
    TEST_SECTION("Zone-Specific Allocation");

    // Test different zone hints
    void *manual_ptr = Asthra_ffi_alloc(1024, ASTHRA_ZONE_HINT_MANUAL);
    void *gc_ptr = Asthra_ffi_alloc(1024, ASTHRA_ZONE_HINT_GC);
    void *secure_ptr = Asthra_ffi_alloc(1024, ASTHRA_ZONE_HINT_SECURE);

    TEST_ASSERT(manual_ptr != NULL, "Manual zone allocation succeeds");
    TEST_ASSERT(gc_ptr != NULL, "GC zone allocation succeeds");
    TEST_ASSERT(secure_ptr != NULL, "Secure zone allocation succeeds");

    // Test large allocations
    void *large_ptr = Asthra_ffi_alloc(1024 * 1024, ASTHRA_ZONE_HINT_MANUAL);
    TEST_ASSERT(large_ptr != NULL, "Large allocation succeeds");

    // Clean up
    Asthra_ffi_free(manual_ptr, ASTHRA_ZONE_HINT_MANUAL);
    Asthra_ffi_free(gc_ptr, ASTHRA_ZONE_HINT_GC);
    Asthra_ffi_free(secure_ptr, ASTHRA_ZONE_HINT_SECURE);
    Asthra_ffi_free(large_ptr, ASTHRA_ZONE_HINT_MANUAL);
}

void test_allocation_edge_cases(void) {
    TEST_SECTION("Allocation Edge Cases");

    // Test zero-size allocation
    void *zero_ptr = Asthra_ffi_alloc(0, ASTHRA_ZONE_HINT_MANUAL);
    TEST_ASSERT(zero_ptr == NULL, "Zero-size allocation returns NULL (as per implementation)");
    // No need to free a NULL pointer

    // Test realloc with NULL pointer (should act like malloc)
    void *realloc_null = Asthra_ffi_realloc(NULL, 1024, ASTHRA_ZONE_HINT_MANUAL);
    TEST_ASSERT(realloc_null != NULL, "Realloc with NULL pointer succeeds");

    // Test realloc with zero size (should act like free)
    void *realloc_zero = Asthra_ffi_realloc(realloc_null, 0, ASTHRA_ZONE_HINT_MANUAL);
    TEST_ASSERT(realloc_zero == NULL, "Realloc with zero size returns NULL");
}

// Test registration
static test_case_t allocation_tests[] = {{"Basic Allocation", test_basic_allocation},
                                         {"Zone Allocation", test_zone_allocation},
                                         {"Allocation Edge Cases", test_allocation_edge_cases}};

int main(void) {
    printf("Asthra Safe C Memory Interface - Allocation Tests\n");
    printf("================================================\n");

    test_runtime_init();

    int failed = run_test_suite("Memory Allocation", allocation_tests,
                                sizeof(allocation_tests) / sizeof(allocation_tests[0]));

    print_test_results();
    test_runtime_cleanup();

    return failed == 0 ? 0 : 1;
}
