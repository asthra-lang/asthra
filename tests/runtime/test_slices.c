/**
 * Asthra Programming Language Runtime v1.2
 * Slice Management Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_common.h"

// =============================================================================
// SLICE MANAGEMENT TESTS
// =============================================================================

int test_slice_operations(void) {
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization failed");
    
    // Create a slice
    AsthraSliceHeader slice = asthra_slice_new(sizeof(int), 10, ASTHRA_OWNERSHIP_GC);
    TEST_ASSERT(slice.ptr != NULL, "Slice creation failed");
    TEST_ASSERT(asthra_slice_get_len(slice) == 10, "Slice length incorrect");
    TEST_ASSERT(asthra_slice_get_element_size(slice) == sizeof(int), "Element size incorrect");
    
    // Test bounds checking
    TEST_ASSERT(asthra_slice_bounds_check(slice, 5), "Valid index should pass bounds check");
    TEST_ASSERT(!asthra_slice_bounds_check(slice, 15), "Invalid index should fail bounds check");
    
    // Test element access and modification
    int value = 42;
    int set_result = asthra_slice_set_element(slice, 3, &value);
    TEST_ASSERT(set_result == 0, "Setting slice element failed");
    
    int *retrieved = (int *)asthra_slice_get_element(slice, 3);
    TEST_ASSERT(retrieved != NULL, "Getting slice element failed");
    TEST_ASSERT(*retrieved == 42, "Retrieved value incorrect");
    
    // Test subslice
    AsthraSliceHeader subslice = asthra_slice_subslice(slice, 2, 7);
    TEST_ASSERT(asthra_slice_get_len(subslice) == 5, "Subslice length incorrect");
    
    // Test slice from raw parts
    int raw_data[] = {1, 2, 3, 4, 5};
    AsthraSliceHeader raw_slice = asthra_slice_from_raw_parts(
        raw_data, 5, sizeof(int), false, ASTHRA_OWNERSHIP_C
    );
    TEST_ASSERT(asthra_slice_get_len(raw_slice) == 5, "Raw slice length incorrect");
    TEST_ASSERT(asthra_slice_get_ptr(raw_slice) == raw_data, "Raw slice pointer incorrect");
    
    asthra_slice_free(slice);
    asthra_runtime_cleanup();
    TEST_PASS("Slice operations");
}

// Test function declarations for external use
int test_slice_operations(void); 
