/**
 * Asthra Programming Language v1.2 Slice Header Structure Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for SliceHeader structure layout, initialization, and basic operations.
 */

#include "slice_test_common.h"

// =============================================================================
// SLICEHEADER STRUCTURE TESTS
// =============================================================================

AsthraTestResult test_slice_header_structure(AsthraV12TestContext *ctx) {
    // Test SliceHeader structure layout and initialization

    // Test creation with different element sizes
    size_t element_sizes[] = {1, 2, 4, 8, 16, 32, 64, 128};
    size_t size_count = sizeof(element_sizes) / sizeof(element_sizes[0]);

    for (size_t i = 0; i < size_count; i++) {
        size_t element_size = element_sizes[i];
        size_t capacity = 100;

        TestSliceHeader *slice = test_slice_create(element_size, capacity);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, slice != NULL,
                                "Failed to create slice with element size %zu", element_size)) {
            return ASTHRA_TEST_FAIL;
        }

        // Verify structure fields
        if (!ASTHRA_TEST_ASSERT(&ctx->base, slice->data != NULL, "Slice data should not be NULL")) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(&ctx->base, slice->len == 0,
                                "Initial slice length should be 0, got %zu", slice->len)) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(&ctx->base, slice->capacity == capacity,
                                "Slice capacity should be %zu, got %zu", capacity,
                                slice->capacity)) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(&ctx->base, slice->element_size == element_size,
                                "Element size should be %zu, got %zu", element_size,
                                slice->element_size)) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(&ctx->base, slice->bounds_check_magic == SLICE_BOUNDS_MAGIC,
                                "Bounds check magic should be valid")) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }

        test_slice_destroy(slice);
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_slice_header_operations(AsthraV12TestContext *ctx) {
    // Test basic slice operations

    TestSliceHeader *int_slice = test_slice_create(sizeof(int), 10);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, int_slice != NULL, "Failed to create integer slice")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test pushing elements
    for (int i = 0; i < 5; i++) {
        int value = i * 10;
        bool success = test_slice_push(int_slice, &value);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, success, "Failed to push element %d", i)) {
            test_slice_destroy(int_slice);
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(&ctx->base, int_slice->len == (size_t)(i + 1),
                                "Slice length should be %d, got %zu", i + 1, int_slice->len)) {
            test_slice_destroy(int_slice);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test getting elements
    for (int i = 0; i < 5; i++) {
        int *value_ptr = (int *)test_slice_get(int_slice, i);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, value_ptr != NULL, "Failed to get element at index %d",
                                i)) {
            test_slice_destroy(int_slice);
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(&ctx->base, *value_ptr == i * 10,
                                "Element value should be %d, got %d", i * 10, *value_ptr)) {
            test_slice_destroy(int_slice);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test setting elements
    for (int i = 0; i < 5; i++) {
        int new_value = i * 100;
        bool success = test_slice_set(int_slice, i, &new_value);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, success, "Failed to set element at index %d", i)) {
            test_slice_destroy(int_slice);
            return ASTHRA_TEST_FAIL;
        }

        int *value_ptr = (int *)test_slice_get(int_slice, i);
        if (!ASTHRA_TEST_ASSERT(&ctx->base, *value_ptr == new_value,
                                "Set element value should be %d, got %d", new_value, *value_ptr)) {
            test_slice_destroy(int_slice);
            return ASTHRA_TEST_FAIL;
        }
    }

    test_slice_destroy(int_slice);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=== Asthra Slice Header Structure Tests ===\n\n");

    AsthraV12TestContext ctx = {0};

    // Run tests
    AsthraTestResult results[] = {test_slice_header_structure(&ctx),
                                  test_slice_header_operations(&ctx)};

    // Print results
    const char *test_names[] = {"Header Structure", "Header Operations"};

    size_t test_count = sizeof(results) / sizeof(results[0]);
    size_t passed = 0;

    printf("\n=== Test Results ===\n");
    for (size_t i = 0; i < test_count; i++) {
        printf("[%s] %s\n", results[i] == ASTHRA_TEST_PASS ? "PASS" : "FAIL", test_names[i]);
        if (results[i] == ASTHRA_TEST_PASS) {
            passed++;
        }
    }

    printf("\n=== Summary ===\n");
    printf("Tests run: %zu\n", test_count);
    printf("Passed: %zu\n", passed);
    printf("Failed: %zu\n", test_count - passed);
    printf("Pass rate: %.1f%%\n", (double)passed / (double)test_count * 100.0);

    return (passed == test_count) ? 0 : 1;
}
