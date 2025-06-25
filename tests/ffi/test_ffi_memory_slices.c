/**
 * Test Suite for Asthra Safe C Memory Interface - Slice Operations
 * Tests for slice creation, bounds checking, element access, and subslicing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_ffi_memory_common.h"

// =============================================================================
// SLICE MANAGEMENT TESTS
// =============================================================================

void test_slice_creation(void) {
    TEST_SECTION("Slice Creation");

    // Test slice creation from raw parts
    int test_array[] = {1, 2, 3, 4, 5};
    AsthraFFISliceHeader slice1 = Asthra_slice_from_raw_parts(test_array, 5, sizeof(int), false,
                                                              ASTHRA_OWNERSHIP_TRANSFER_NONE);

    TEST_ASSERT(Asthra_slice_is_valid(slice1), "Slice from raw parts is valid");
    TEST_ASSERT(Asthra_slice_get_len(slice1) == 5, "Slice length is correct");
    TEST_ASSERT(Asthra_slice_get_element_size(slice1) == sizeof(int), "Element size is correct");

    // Test new slice creation
    AsthraFFISliceHeader slice2 = Asthra_slice_new(sizeof(double), 10, 20, ASTHRA_ZONE_HINT_MANUAL);
    TEST_ASSERT(Asthra_slice_is_valid(slice2), "New slice is valid");
    TEST_ASSERT(Asthra_slice_get_len(slice2) == 10, "New slice length is correct");
    TEST_ASSERT(Asthra_slice_get_cap(slice2) == 20, "New slice capacity is correct");

    // Clean up
    Asthra_slice_free(slice2);
}

void test_slice_bounds_checking(void) {
    TEST_SECTION("Slice Bounds Checking");

    int test_array[] = {10, 20, 30, 40, 50};
    AsthraFFISliceHeader slice = Asthra_slice_from_raw_parts(test_array, 5, sizeof(int), false,
                                                             ASTHRA_OWNERSHIP_TRANSFER_NONE);

    // Test valid indices
    AsthraFFIResult bounds_result = Asthra_slice_bounds_check(slice, 0);
    TEST_ASSERT(Asthra_result_is_ok(bounds_result), "Index 0 passes bounds check");

    bounds_result = Asthra_slice_bounds_check(slice, 2);
    TEST_ASSERT(Asthra_result_is_ok(bounds_result), "Index 2 passes bounds check");

    bounds_result = Asthra_slice_bounds_check(slice, 4);
    TEST_ASSERT(Asthra_result_is_ok(bounds_result), "Index 4 passes bounds check");

    // Test invalid indices
    bounds_result = Asthra_slice_bounds_check(slice, 5);
    TEST_ASSERT(Asthra_result_is_err(bounds_result), "Index 5 fails bounds check");

    bounds_result = Asthra_slice_bounds_check(slice, 10);
    TEST_ASSERT(Asthra_result_is_err(bounds_result), "Index 10 fails bounds check");
}

void test_slice_element_access(void) {
    TEST_SECTION("Slice Element Access");

    int test_array[] = {100, 200, 300, 400, 500};
    AsthraFFISliceHeader slice = Asthra_slice_from_raw_parts(test_array, 5, sizeof(int), true,
                                                             ASTHRA_OWNERSHIP_TRANSFER_NONE);

    // Test element reading
    int element;
    AsthraFFIResult get_result = Asthra_slice_get_element(slice, 2, &element);
    TEST_ASSERT(Asthra_result_is_ok(get_result), "Element access succeeds");
    TEST_ASSERT(element == 300, "Retrieved element has correct value");

    // Test element writing
    int new_value = 999;
    AsthraFFIResult set_result = Asthra_slice_set_element(slice, 3, &new_value);
    TEST_ASSERT(Asthra_result_is_ok(set_result), "Setting element succeeds");

    // Verify the change
    get_result = Asthra_slice_get_element(slice, 3, &element);
    TEST_ASSERT(Asthra_result_is_ok(get_result), "Getting updated element succeeds");
    TEST_ASSERT(element == 999, "Updated element has correct value");

    // Test out-of-bounds access
    get_result = Asthra_slice_get_element(slice, 10, &element);
    TEST_ASSERT(Asthra_result_is_err(get_result), "Out-of-bounds access fails");
}

void test_slice_subslicing(void) {
    TEST_SECTION("Slice Subslicing");

    int test_array[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    AsthraFFISliceHeader parent_slice = Asthra_slice_from_raw_parts(
        test_array, 10, sizeof(int), false, ASTHRA_OWNERSHIP_TRANSFER_NONE);

    // Test valid subslice
    AsthraFFIResult subslice_result = Asthra_slice_subslice(parent_slice, 2, 7);
    TEST_ASSERT(Asthra_result_is_ok(subslice_result), "Valid subslice creation succeeds");

    AsthraFFISliceHeader *subslice =
        (AsthraFFISliceHeader *)Asthra_result_unwrap_ok(subslice_result);
    TEST_ASSERT(Asthra_slice_get_len(*subslice) == 5, "Subslice has correct length");

    // Test element access in subslice
    int element;
    AsthraFFIResult get_result = Asthra_slice_get_element(*subslice, 0, &element);
    TEST_ASSERT(Asthra_result_is_ok(get_result), "Subslice element access succeeds");
    TEST_ASSERT(element == 3, "Subslice element has correct value");

    // Test invalid subslice range
    AsthraFFIResult invalid_subslice = Asthra_slice_subslice(parent_slice, 5, 15);
    TEST_ASSERT(Asthra_result_is_err(invalid_subslice), "Invalid subslice range fails");

    // Clean up
    free(subslice);
}

void test_slice_type_safety(void) {
    TEST_SECTION("Slice Type Safety");

    // Test different element types
    double double_array[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    AsthraFFISliceHeader double_slice = Asthra_slice_from_raw_parts(
        double_array, 5, sizeof(double), true, ASTHRA_OWNERSHIP_TRANSFER_NONE);

    TEST_ASSERT(Asthra_slice_get_element_size(double_slice) == sizeof(double),
                "Double slice has correct element size");

    double value = 9.9;
    AsthraFFIResult set_result = Asthra_slice_set_element(double_slice, 2, &value);
    TEST_ASSERT(Asthra_result_is_ok(set_result), "Setting double element succeeds");

    double retrieved;
    AsthraFFIResult get_result = Asthra_slice_get_element(double_slice, 2, &retrieved);
    TEST_ASSERT(Asthra_result_is_ok(get_result), "Getting double element succeeds");
    TEST_ASSERT(retrieved == 9.9, "Retrieved double has correct value");

    // Test struct slices
    typedef struct {
        int id;
        float score;
    } TestStruct;

    TestStruct struct_array[] = {{1, 1.5f}, {2, 2.5f}, {3, 3.5f}};
    AsthraFFISliceHeader struct_slice = Asthra_slice_from_raw_parts(
        struct_array, 3, sizeof(TestStruct), true, ASTHRA_OWNERSHIP_TRANSFER_NONE);

    TEST_ASSERT(Asthra_slice_get_element_size(struct_slice) == sizeof(TestStruct),
                "Struct slice has correct element size");

    TestStruct new_struct = {99, 99.9f};
    set_result = Asthra_slice_set_element(struct_slice, 1, &new_struct);
    TEST_ASSERT(Asthra_result_is_ok(set_result), "Setting struct element succeeds");

    TestStruct retrieved_struct;
    get_result = Asthra_slice_get_element(struct_slice, 1, &retrieved_struct);
    TEST_ASSERT(Asthra_result_is_ok(get_result), "Getting struct element succeeds");
    TEST_ASSERT(retrieved_struct.id == 99 && retrieved_struct.score == 99.9f,
                "Retrieved struct has correct values");
}

// Test registration
static test_case_t slice_tests[] = {{"Slice Creation", test_slice_creation},
                                    {"Slice Bounds Checking", test_slice_bounds_checking},
                                    {"Slice Element Access", test_slice_element_access},
                                    {"Slice Subslicing", test_slice_subslicing},
                                    {"Slice Type Safety", test_slice_type_safety}};

int main(void) {
    printf("Asthra Safe C Memory Interface - Slice Tests\n");
    printf("===========================================\n");

    test_runtime_init();

    int failed = run_test_suite("Slice Operations", slice_tests,
                                sizeof(slice_tests) / sizeof(slice_tests[0]));

    print_test_results();
    test_runtime_cleanup();

    return failed == 0 ? 0 : 1;
}
