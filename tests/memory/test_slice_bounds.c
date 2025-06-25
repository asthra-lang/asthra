/**
 * Asthra Programming Language v1.2 Slice Bounds Checking Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for slice bounds checking including runtime checks, edge cases, and performance.
 */

#include "slice_test_common.h"

// =============================================================================
// BOUNDS CHECKING TESTS
// =============================================================================

AsthraTestResult test_slice_bounds_checking_runtime(AsthraV12TestContext *ctx) {
    // Test runtime bounds checking

    TestSliceHeader *slice = test_slice_create(sizeof(int), 5);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, slice != NULL, "Failed to create test slice")) {
        return ASTHRA_TEST_FAIL;
    }

    // Add some elements
    for (int i = 0; i < 3; i++) {
        int value = i;
        test_slice_push(slice, &value);
    }

    // Test valid access
    for (int i = 0; i < 3; i++) {
        bool valid = test_slice_bounds_check(slice, i);
        if (!ASTHRA_TEST_ASSERT(&ctx->base, valid, "Valid index %d should pass bounds check", i)) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }

        void *ptr = test_slice_get(slice, i);
        if (!ASTHRA_TEST_ASSERT(&ctx->base, ptr != NULL, "Valid access at index %d should succeed",
                                i)) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test invalid access (beyond length)
    for (int i = 3; i < 10; i++) {
        bool valid = test_slice_bounds_check(slice, i);
        if (!ASTHRA_TEST_ASSERT(&ctx->base, !valid, "Invalid index %d should fail bounds check",
                                i)) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }

        void *ptr = test_slice_get(slice, i);
        if (!ASTHRA_TEST_ASSERT(&ctx->base, ptr == NULL,
                                "Invalid access at index %d should return NULL", i)) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test corrupted slice (invalid magic)
    slice->bounds_check_magic = 0xBADC0DE;
    bool valid = test_slice_bounds_check(slice, 0);
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !valid, "Corrupted slice should fail bounds check")) {
        test_slice_destroy(slice);
        return ASTHRA_TEST_FAIL;
    }

    test_slice_destroy(slice);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_slice_bounds_checking_edge_cases(AsthraV12TestContext *ctx) {
    // Test edge cases in bounds checking

    // Test empty slice
    TestSliceHeader *empty_slice = test_slice_create(sizeof(int), 10);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, empty_slice != NULL, "Failed to create empty slice")) {
        return ASTHRA_TEST_FAIL;
    }

    // Access to empty slice should fail
    bool valid = test_slice_bounds_check(empty_slice, 0);
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !valid, "Access to empty slice should fail")) {
        test_slice_destroy(empty_slice);
        return ASTHRA_TEST_FAIL;
    }

    // Test maximum size_t index
    valid = test_slice_bounds_check(empty_slice, SIZE_MAX);
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !valid, "Maximum size_t index should fail bounds check")) {
        test_slice_destroy(empty_slice);
        return ASTHRA_TEST_FAIL;
    }

    test_slice_destroy(empty_slice);

    // Test slice at capacity
    TestSliceHeader *full_slice = test_slice_create(sizeof(char), 3);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, full_slice != NULL, "Failed to create full slice")) {
        return ASTHRA_TEST_FAIL;
    }

    // Fill to capacity
    for (int i = 0; i < 3; i++) {
        char value = 'A' + i;
        test_slice_push(full_slice, &value);
    }

    // Access at capacity boundary
    valid = test_slice_bounds_check(full_slice, 2); // Last valid index
    if (!ASTHRA_TEST_ASSERT(&ctx->base, valid, "Last valid index should pass bounds check")) {
        test_slice_destroy(full_slice);
        return ASTHRA_TEST_FAIL;
    }

    valid = test_slice_bounds_check(full_slice, 3); // First invalid index
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !valid, "First invalid index should fail bounds check")) {
        test_slice_destroy(full_slice);
        return ASTHRA_TEST_FAIL;
    }

    test_slice_destroy(full_slice);

    // Test NULL slice
    valid = test_slice_bounds_check(NULL, 0);
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !valid, "NULL slice should fail bounds check")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_slice_bounds_checking_performance(AsthraV12TestContext *ctx) {
    // Test bounds checking performance impact

    const int iterations = 100000;
    const size_t slice_size = 1000;

    TestSliceHeader *slice = test_slice_create(sizeof(int), slice_size);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, slice != NULL, "Failed to create performance test slice")) {
        return ASTHRA_TEST_ERROR;
    }

    // Fill slice
    for (size_t i = 0; i < slice_size; i++) {
        int value = (int)i;
        test_slice_push(slice, &value);
    }

    // Test bounds-checked access (simplified without timing)
    volatile int sum = 0;

    for (int iter = 0; iter < iterations; iter++) {
        size_t index = iter % slice_size;

        if (test_slice_bounds_check(slice, index)) {
            int *value_ptr = (int *)test_slice_get(slice, index);
            if (value_ptr) {
                sum += *value_ptr;
            }
        }
    }

    // Test unchecked access (for comparison)
    sum = 0;

    int *data = (int *)slice->data;
    for (int iter = 0; iter < iterations; iter++) {
        size_t index = iter % slice_size;
        sum += data[index]; // Direct access without bounds checking
    }

    // Basic validation that operations completed
    if (!ASTHRA_TEST_ASSERT(&ctx->base, slice->len == slice_size,
                            "Slice should be filled to capacity")) {
        test_slice_destroy(slice);
        return ASTHRA_TEST_FAIL;
    }

    test_slice_destroy(slice);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST EXECUTION
// =============================================================================

int main(void) {
    printf("=== Asthra Slice Bounds Checking Tests ===\n\n");

    // Create test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Create test metadata for context
    AsthraTestMetadata metadata = {.name = "slice_bounds_test",
                                   .file = __FILE__,
                                   .line = __LINE__,
                                   .function = "main",
                                   .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                   .timeout_ns = 60000000000ULL, // 60 seconds
                                   .skip = false,
                                   .skip_reason = NULL};

    // Create test context
    AsthraV12TestContext context = {0};

    // Run tests
    AsthraTestResult results[] = {test_slice_bounds_checking_runtime(&context),
                                  test_slice_bounds_checking_edge_cases(&context),
                                  test_slice_bounds_checking_performance(&context)};

    // Print results
    const char *test_names[] = {"Runtime Bounds Checking", "Edge Cases", "Performance Impact"};

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

    // Cleanup
    asthra_test_statistics_destroy(stats);

    return (passed == test_count) ? 0 : 1;
}
