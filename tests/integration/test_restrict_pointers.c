/**
 * Asthra Programming Language Compiler
 * Static Analysis Integration Test Suite - Restrict Pointer Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 5.2: Restrict Pointer Tests
 * - Test restrict pointer annotations
 * - Validate aligned allocation operations
 * - Test array allocation and reallocation
 */

#include "test_static_analysis_common.h"

// =============================================================================
// RESTRICT POINTER TESTS
// =============================================================================

AsthraTestResult test_restrict_pointer_operations(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Test aligned allocation
    void *ptr = asthra_alloc_aligned(1024, 32);

    if (!asthra_test_assert_pointer(context, ptr, "Aligned allocation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Check alignment
    uintptr_t addr = (uintptr_t)ptr;
    if (!asthra_test_assert_bool(context, (addr % 32) == 0, "Pointer should be 32-byte aligned")) {
        asthra_free_aligned(ptr);
        return ASTHRA_TEST_FAIL;
    }

    asthra_free_aligned(ptr);

    // Test array allocation
    size_t element_size = sizeof(int);
    size_t count = 100;
    int *array = (int *)asthra_alloc_array(element_size, count);

    if (!asthra_test_assert_pointer(context, array, "Array allocation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify zero initialization
    bool all_zero = true;
    for (size_t i = 0; i < count; i++) {
        if (array[i] != 0) {
            all_zero = false;
            break;
        }
    }

    if (!asthra_test_assert_bool(context, all_zero, "Array should be zero-initialized")) {
        free(array);
        return ASTHRA_TEST_FAIL;
    }

    // Test array reallocation
    size_t new_count = 200;
    array = (int *)asthra_realloc_array(array, count, new_count, element_size);

    if (!asthra_test_assert_pointer(context, array, "Array reallocation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify new elements are zero-initialized
    all_zero = true;
    for (size_t i = count; i < new_count; i++) {
        if (array[i] != 0) {
            all_zero = false;
            break;
        }
    }

    if (!asthra_test_assert_bool(context, all_zero,
                                 "New array elements should be zero-initialized")) {
        free(array);
        return ASTHRA_TEST_FAIL;
    }

    free(array);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

/**
 * Main test runner
 */
int main(void) {
    printf("=== Asthra Integration Tests - Restrict Pointers ===\n\n");

    // Create test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        printf("Failed to create test statistics\n");
        return 1;
    }

    // Create test metadata
    AsthraTestMetadata metadata = {.name = "restrict_pointer_operations",
                                   .file = __FILE__,
                                   .line = __LINE__,
                                   .function = "test_restrict_pointer_operations",
                                   .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                   .timeout_ns = 30000000000ULL,
                                   .skip = false,
                                   .skip_reason = NULL};

    AsthraTestContext *context = asthra_test_context_create(&metadata, stats);
    if (!context) {
        printf("Failed to create test context\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }

    // Run test
    AsthraTestResult result = test_restrict_pointer_operations(context);

    // Report results
    printf("Restrict pointer operations: %s\n", result == ASTHRA_TEST_PASS ? "PASS" : "FAIL");

    // Print statistics
    printf("\n=== Test Statistics ===\n");
    printf("Tests run:       1\n");
    printf("Tests passed:    %d\n", result == ASTHRA_TEST_PASS ? 1 : 0);
    printf("Tests failed:    %d\n", result == ASTHRA_TEST_FAIL ? 1 : 0);
    printf("Tests skipped:   0\n");
    printf("Tests error:     0\n");
    printf("Tests timeout:   0\n");
    printf("Total duration:  0.000 ms\n");
    printf("Max duration:    0.000 ms\n");
    printf("Min duration:    0.000 ms\n");
    printf("Assertions:      %llu checked, %llu failed\n",
           asthra_test_get_stat(&stats->assertions_checked),
           asthra_test_get_stat(&stats->assertions_failed));
    printf("========================\n");
    printf("Integration tests: %d/1 passed\n", result == ASTHRA_TEST_PASS ? 1 : 0);

    // Cleanup
    asthra_test_context_destroy(context);
    asthra_test_statistics_destroy(stats);

    return result == ASTHRA_TEST_PASS ? 0 : 1;
}
