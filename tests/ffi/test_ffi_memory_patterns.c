/**
 * Test Suite for Asthra Safe C Memory Interface - Pattern Matching & Error Handling
 * Tests for result types, error propagation, and pattern matching scenarios
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_ffi_memory_common.h"
#include <errno.h>

// Function prototypes
static void test_result_creation(void);
static void test_result_pattern_matching(void);
static void test_error_propagation(void);
static void test_error_contexts(void);
static void test_result_chaining(void);
static void test_error_recovery(void);

// =============================================================================
// PATTERN MATCHING ERROR HANDLING TESTS
// =============================================================================

static void test_result_creation(void) {
    TEST_SECTION("Result Creation");
    
    // Test successful result creation
    int success_value = 42;
    AsthraFFIResult ok_result = Asthra_result_ok(&success_value, sizeof(int), 0, 
                                                ASTHRA_OWNERSHIP_TRANSFER_NONE);
    TEST_ASSERT(Asthra_result_is_ok(ok_result), "OK result creation succeeds");
    TEST_ASSERT(!Asthra_result_is_err(ok_result), "OK result is not error");
    
    int *retrieved_value = (int*)Asthra_result_unwrap_ok(ok_result);
    TEST_ASSERT(*retrieved_value == 42, "OK result unwrapping returns correct value");
    
    // Test error result creation
    AsthraFFIResult err_result = Asthra_result_err(404, "Not found", "test_function", NULL);
    TEST_ASSERT(Asthra_result_is_err(err_result), "Error result creation succeeds");
    TEST_ASSERT(!Asthra_result_is_ok(err_result), "Error result is not OK");
    
    int error_code = Asthra_result_get_error_code(err_result);
    TEST_ASSERT(error_code == 404, "Error code retrieval is correct");
    
    const char *error_message = Asthra_result_get_error_message(err_result);
    TEST_ASSERT(strcmp(error_message, "Not found") == 0, "Error message retrieval is correct");
}

static void test_result_pattern_matching(void) {
    TEST_SECTION("Result Pattern Matching");
    
    int test_value = 123;
    AsthraFFIResult ok_result = Asthra_result_ok(&test_value, sizeof(int), 0, 
                                                ASTHRA_OWNERSHIP_TRANSFER_NONE);
    AsthraFFIResult err_result = Asthra_result_err(500, "Internal error", "test", NULL);
    
    // Test pattern matching for OK result
    bool handled_ok = false;
    bool handled_err = false;
    
    if (Asthra_result_is_ok(ok_result)) {
        int *value = (int*)Asthra_result_unwrap_ok(ok_result);
        TEST_ASSERT(*value == 123, "Pattern matched OK result has correct value");
        handled_ok = true;
    } else if (Asthra_result_is_err(ok_result)) {
        handled_err = true;
    }
    TEST_ASSERT(handled_ok && !handled_err, "OK result pattern matching works");
    
    // Reset flags
    handled_ok = false;
    handled_err = false;
    
    // Test pattern matching for error result
    if (Asthra_result_is_ok(err_result)) {
        handled_ok = true;
    } else if (Asthra_result_is_err(err_result)) {
        int code = Asthra_result_get_error_code(err_result);
        const char *msg = Asthra_result_get_error_message(err_result);
        TEST_ASSERT(code == 500, "Pattern matched error has correct code");
        TEST_ASSERT(strcmp(msg, "Internal error") == 0, "Pattern matched error has correct message");
        handled_err = true;
    }
    TEST_ASSERT(!handled_ok && handled_err, "Error result pattern matching works");
}

static void test_error_propagation(void) {
    TEST_SECTION("Error Propagation");
    
    // Simulate a chain of operations that might fail
    
    // Step 1: Create a slice that might fail bounds checking
    int test_array[] = {1, 2, 3, 4, 5};
    AsthraFFISliceHeader slice = Asthra_slice_from_raw_parts(
        test_array, 5, sizeof(int), false, ASTHRA_OWNERSHIP_TRANSFER_NONE);
    
    // Step 2: Try valid operation
    AsthraFFIResult bounds_result = Asthra_slice_bounds_check(slice, 2);
    if (Asthra_result_is_ok(bounds_result)) {
        int element;
        AsthraFFIResult get_result = Asthra_slice_get_element(slice, 2, &element);
        if (Asthra_result_is_ok(get_result)) {
            TEST_ASSERT(element == 3, "Successful operation chain returns correct value");
        } else {
            TEST_ASSERT(false, "Valid element access should not fail");
        }
    } else {
        TEST_ASSERT(false, "Valid bounds check should not fail");
    }
    
    // Step 3: Try invalid operation and verify error propagation
    bounds_result = Asthra_slice_bounds_check(slice, 10);
    if (Asthra_result_is_ok(bounds_result)) {
        TEST_ASSERT(false, "Invalid bounds check should fail");
    } else {
        // Error correctly propagated
        int error_code = Asthra_result_get_error_code(bounds_result);
        TEST_ASSERT(error_code != 0, "Error propagation includes error code");
        
        const char *error_msg = Asthra_result_get_error_message(bounds_result);
        TEST_ASSERT(error_msg != NULL, "Error propagation includes error message");
    }
}

static void test_error_contexts(void) {
    TEST_SECTION("Error Contexts");
    
    // Test error with different contexts
    AsthraFFIResult mem_error = Asthra_result_err(ENOMEM, "Out of memory", "allocation_function", NULL);
    AsthraFFIResult bounds_error = Asthra_result_err(ERANGE, "Index out of bounds", "slice_access", NULL);
    AsthraFFIResult type_error = Asthra_result_err(EINVAL, "Invalid type", "type_conversion", NULL);
    
    // Verify different error types
    TEST_ASSERT(Asthra_result_get_error_code(mem_error) == ENOMEM, "Memory error has correct code");
    TEST_ASSERT(Asthra_result_get_error_code(bounds_error) == ERANGE, "Bounds error has correct code");
    TEST_ASSERT(Asthra_result_get_error_code(type_error) == EINVAL, "Type error has correct code");
    
    // Test error message content
    TEST_ASSERT(strstr(Asthra_result_get_error_message(mem_error), "memory") != NULL,
                "Memory error message contains expected text");
    TEST_ASSERT(strstr(Asthra_result_get_error_message(bounds_error), "bounds") != NULL,
                "Bounds error message contains expected text");
    TEST_ASSERT(strstr(Asthra_result_get_error_message(type_error), "type") != NULL,
                "Type error message contains expected text");
}

static void test_result_chaining(void) {
    TEST_SECTION("Result Chaining");
    
    // Simulate a sequence of operations where each depends on the previous
    
    // Operation 1: Create a slice
    AsthraFFISliceHeader slice = Asthra_slice_new(sizeof(int), 5, 10, ASTHRA_ZONE_HINT_MANUAL);
    if (!Asthra_slice_is_valid(slice)) {
        TEST_ASSERT(false, "Slice creation should succeed");
        return;
    }
    
    // Operation 2: Set some elements
    int values[] = {10, 20, 30, 40, 50};
    bool all_sets_succeeded = true;
    
    for (int i = 0; i < 5; i++) {
        AsthraFFIResult set_result = Asthra_slice_set_element(slice, i, &values[i]);
        if (Asthra_result_is_err(set_result)) {
            all_sets_succeeded = false;
            printf("Set operation %d failed: %s\n", i, Asthra_result_get_error_message(set_result));
            break;
        }
    }
    TEST_ASSERT(all_sets_succeeded, "All set operations in chain succeed");
    
    // Operation 3: Verify all elements
    bool all_gets_succeeded = true;
    
    for (int i = 0; i < 5; i++) {
        int retrieved;
        AsthraFFIResult get_result = Asthra_slice_get_element(slice, i, &retrieved);
        if (Asthra_result_is_err(get_result)) {
            all_gets_succeeded = false;
            printf("Get operation %d failed: %s\n", i, Asthra_result_get_error_message(get_result));
            break;
        }
        if (retrieved != values[i]) {
            all_gets_succeeded = false;
            printf("Get operation %d returned wrong value: expected %d, got %d\n", 
                   i, values[i], retrieved);
            break;
        }
    }
    TEST_ASSERT(all_gets_succeeded, "All get operations in chain succeed");
    
    // Operation 4: Try an operation that should fail
    int dummy;
    AsthraFFIResult invalid_get = Asthra_slice_get_element(slice, 100, &dummy);
    TEST_ASSERT(Asthra_result_is_err(invalid_get), "Invalid operation in chain fails appropriately");
    
    // Clean up
    Asthra_slice_free(slice);
}

static void test_error_recovery(void) {
    TEST_SECTION("Error Recovery");
    
    // Test graceful handling of various error conditions
    
    // Recovery from allocation failure simulation
    void *large_ptr = Asthra_ffi_alloc(SIZE_MAX, ASTHRA_ZONE_HINT_MANUAL);
    if (large_ptr == NULL) {
        // Expected failure - try smaller allocation
        void *small_ptr = Asthra_ffi_alloc(1024, ASTHRA_ZONE_HINT_MANUAL);
        TEST_ASSERT(small_ptr != NULL, "Recovery allocation after failure succeeds");
        Asthra_ffi_free(small_ptr, ASTHRA_ZONE_HINT_MANUAL);
    } else {
        // Unexpectedly succeeded - clean up
        Asthra_ffi_free(large_ptr, ASTHRA_ZONE_HINT_MANUAL);
        TEST_ASSERT(true, "Large allocation unexpectedly succeeded");
    }
    
    // Recovery from slice operations
    int test_array[] = {1, 2, 3};
    AsthraFFISliceHeader slice = Asthra_slice_from_raw_parts(
        test_array, 3, sizeof(int), false, ASTHRA_OWNERSHIP_TRANSFER_NONE);
    
    // Try invalid access
    int element;
    AsthraFFIResult invalid_access = Asthra_slice_get_element(slice, 10, &element);
    if (Asthra_result_is_err(invalid_access)) {
        // Recover with valid access
        AsthraFFIResult valid_access = Asthra_slice_get_element(slice, 1, &element);
        TEST_ASSERT(Asthra_result_is_ok(valid_access), "Recovery access after error succeeds");
        TEST_ASSERT(element == 2, "Recovery access returns correct value");
    } else {
        TEST_ASSERT(false, "Invalid access should have failed");
    }
}

// Test registration
static test_case_t pattern_tests[] = {
    {"Result Creation", test_result_creation},
    {"Result Pattern Matching", test_result_pattern_matching},
    {"Error Propagation", test_error_propagation},
    {"Error Contexts", test_error_contexts},
    {"Result Chaining", test_result_chaining},
    {"Error Recovery", test_error_recovery}
};

int main(void) {
    printf("Asthra Safe C Memory Interface - Pattern Matching Tests\n");
    printf("======================================================\n");
    
    test_runtime_init();
    
    int failed = run_test_suite("Pattern Matching & Error Handling", pattern_tests, 
                               sizeof(pattern_tests) / sizeof(pattern_tests[0]));
    
    print_test_results();
    test_runtime_cleanup();
    
    return failed == 0 ? 0 : 1;
} 
