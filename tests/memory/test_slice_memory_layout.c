/**
 * Asthra Programming Language v1.2 Slice Memory Layout Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for slice memory layout, alignment requirements, and memory efficiency.
 */

#include "slice_test_common.h"

// =============================================================================
// MEMORY LAYOUT TESTS
// =============================================================================

AsthraTestResult test_slice_header_memory_layout(AsthraV12TestContext *ctx) {
    // Test predictable memory layout of SliceHeader
    
    TestSliceHeader *slice = test_slice_create(sizeof(double), 50);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, slice != NULL,
                           "Failed to create test slice")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test memory alignment
    uintptr_t data_addr = (uintptr_t)slice->data;
    
    // Data should be properly aligned for the element type
    if (!ASTHRA_TEST_ASSERT(&ctx->base, (data_addr % sizeof(double)) == 0,
                           "Slice data not properly aligned for double")) {
        test_slice_destroy(slice);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test that elements are contiguous
    for (int i = 0; i < 10; i++) {
        double value = i * 3.14159;
        test_slice_push(slice, &value);
    }
    
    // Verify contiguous layout
    double *first_element = (double*)test_slice_get(slice, 0);
    double *second_element = (double*)test_slice_get(slice, 1);
    
    ptrdiff_t element_distance = (uint8_t*)second_element - (uint8_t*)first_element;
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, element_distance == sizeof(double),
                           "Elements not contiguous: distance %td, expected %zu", 
                           element_distance, sizeof(double))) {
        test_slice_destroy(slice);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test that all elements are accessible via pointer arithmetic
    double *base_ptr = (double*)slice->data;
    for (size_t i = 0; i < slice->len; i++) {
        double expected = i * 3.14159;
        double actual = base_ptr[i];
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, actual == expected,
                               "Element %zu: expected %.5f, got %.5f", i, expected, actual)) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    test_slice_destroy(slice);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_slice_predictable_memory_layout(AsthraV12TestContext *ctx) {
    // Test that slice memory layout is predictable and consistent
    
    // Test with different data types
    struct TestStruct {
        int a;
        double b;
        char c;
    };
    
    TestSliceHeader *struct_slice = test_slice_create(sizeof(struct TestStruct), 10);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, struct_slice != NULL,
                           "Failed to create struct slice")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Add elements and verify layout
    for (int i = 0; i < 5; i++) {
        struct TestStruct value = {i, i * 2.5, 'A' + i};
        test_slice_push(struct_slice, &value);
    }
    
    // Verify elements are laid out contiguously
    struct TestStruct *base = (struct TestStruct*)struct_slice->data;
    
    for (int i = 0; i < 5; i++) {
        struct TestStruct *element = &base[i];
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, element->a == i,
                               "Element %d field 'a': expected %d, got %d", i, i, element->a)) {
            test_slice_destroy(struct_slice);
            return ASTHRA_TEST_FAIL;
        }
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, element->b == i * 2.5,
                               "Element %d field 'b': expected %.1f, got %.1f", i, i * 2.5, element->b)) {
            test_slice_destroy(struct_slice);
            return ASTHRA_TEST_FAIL;
        }
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, element->c == 'A' + i,
                               "Element %d field 'c': expected '%c', got '%c'", i, 'A' + i, element->c)) {
            test_slice_destroy(struct_slice);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Test memory addresses are sequential
    for (int i = 0; i < 4; i++) {
        uintptr_t addr1 = (uintptr_t)&base[i];
        uintptr_t addr2 = (uintptr_t)&base[i + 1];
        ptrdiff_t diff = addr2 - addr1;
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, diff == sizeof(struct TestStruct),
                               "Non-contiguous memory layout: difference %td, expected %zu", 
                               diff, sizeof(struct TestStruct))) {
            test_slice_destroy(struct_slice);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    test_slice_destroy(struct_slice);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_slice_alignment_requirements(AsthraV12TestContext *ctx) {
    // Test that slices respect alignment requirements
    
    // Test alignment for different types
    struct AlignmentTest {
        size_t element_size;
        size_t expected_alignment;
    } alignment_tests[] = {
        {sizeof(char), 1},
        {sizeof(short), sizeof(short)},
        {sizeof(int), sizeof(int)},
        {sizeof(long), sizeof(long)},
        {sizeof(double), sizeof(double)},
        {sizeof(void*), sizeof(void*)},
    };
    
    size_t test_count = sizeof(alignment_tests) / sizeof(alignment_tests[0]);
    
    for (size_t i = 0; i < test_count; i++) {
        TestSliceHeader *slice = test_slice_create(alignment_tests[i].element_size, 10);
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, slice != NULL,
                               "Failed to create slice for alignment test %zu", i)) {
            return ASTHRA_TEST_FAIL;
        }
        
        uintptr_t data_addr = (uintptr_t)slice->data;
        size_t alignment = alignment_tests[i].expected_alignment;
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, (data_addr % alignment) == 0,
                               "Data not aligned for element size %zu: address 0x%lx, alignment %zu", 
                               alignment_tests[i].element_size, data_addr, alignment)) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }
        
        test_slice_destroy(slice);
    }
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_slice_memory_efficiency(AsthraV12TestContext *ctx) {
    // Test memory efficiency of slice implementation
    
    const size_t capacity = 1000;
    TestSliceHeader *slice = test_slice_create(sizeof(int), capacity);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, slice != NULL,
                           "Failed to create efficiency test slice")) {
        return ASTHRA_TEST_ERROR;
    }
    
    // Calculate expected memory usage
    size_t header_size = sizeof(TestSliceHeader);
    size_t data_size = sizeof(int) * capacity;
    size_t expected_total = header_size + data_size;
    
    // In a real implementation, we would measure actual memory usage
    // For this test, we verify the slice uses expected amount
    
    // Verify no excessive padding in data
    if (!ASTHRA_TEST_ASSERT(&ctx->base, slice->capacity == capacity,
                           "Capacity should match requested: expected %zu, got %zu", 
                           capacity, slice->capacity)) {
        test_slice_destroy(slice);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test memory usage doesn't grow unexpectedly when adding elements
    size_t initial_capacity = slice->capacity;
    
    // Fill slice to capacity
    for (size_t i = 0; i < capacity; i++) {
        int value = (int)i;
        bool success = test_slice_push(slice, &value);
        
        if (!success) break; // Expected when reaching capacity
        
        // Capacity should remain constant
        if (!ASTHRA_TEST_ASSERT(&ctx->base, slice->capacity == initial_capacity,
                               "Capacity changed unexpectedly at element %zu", i)) {
            test_slice_destroy(slice);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Verify we used the full capacity
    if (!ASTHRA_TEST_ASSERT(&ctx->base, slice->len == capacity,
                           "Should have filled to capacity: expected %zu, got %zu", 
                           capacity, slice->len)) {
        test_slice_destroy(slice);
        return ASTHRA_TEST_FAIL;
    }
    
    test_slice_destroy(slice);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=== Asthra Slice Memory Layout Tests ===\n\n");
    
    AsthraV12TestContext ctx = {0};
    
    // Run tests
    AsthraTestResult results[] = {
        test_slice_header_memory_layout(&ctx),
        test_slice_predictable_memory_layout(&ctx),
        test_slice_alignment_requirements(&ctx),
        test_slice_memory_efficiency(&ctx)
    };
    
    // Print results
    const char* test_names[] = {
        "Header Memory Layout",
        "Predictable Memory Layout",
        "Alignment Requirements",
        "Memory Efficiency"
    };
    
    size_t test_count = sizeof(results) / sizeof(results[0]);
    size_t passed = 0;
    
    printf("\n=== Test Results ===\n");
    for (size_t i = 0; i < test_count; i++) {
        printf("[%s] %s\n", 
               results[i] == ASTHRA_TEST_PASS ? "PASS" : "FAIL",
               test_names[i]);
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
