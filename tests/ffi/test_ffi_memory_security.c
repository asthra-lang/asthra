/**
 * Test Suite for Asthra Safe C Memory Interface - Security & Variants
 * Tests for secure memory operations and variant array handling
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_ffi_memory_common.h"

// =============================================================================
// SECURE MEMORY TESTS
// =============================================================================

void test_secure_allocation(void) {
    TEST_SECTION("Secure Memory Allocation");

    // Test secure allocation
    void *secure_ptr = Asthra_secure_alloc(1024);
    TEST_ASSERT(secure_ptr != NULL, "Secure allocation succeeds");

    // Fill with test data
    memset(secure_ptr, 0xAA, 1024);

    // Verify data was written
    uint8_t *bytes = (uint8_t *)secure_ptr;
    bool data_written = true;
    for (size_t i = 0; i < 1024; i++) {
        if (bytes[i] != 0xAA) {
            data_written = false;
            break;
        }
    }
    TEST_ASSERT(data_written, "Secure memory accepts data writes");

    // Test secure free
    Asthra_secure_free(secure_ptr, 1024);
    TEST_ASSERT(true, "Secure free completes without error");
}

void test_secure_zeroing(void) {
    TEST_SECTION("Secure Memory Zeroing");

    void *secure_ptr = Asthra_secure_alloc(512);
    TEST_ASSERT(secure_ptr != NULL, "Secure allocation for zeroing succeeds");

    // Fill with test pattern
    memset(secure_ptr, 0xBB, 512);

    // Test secure zeroing
    Asthra_secure_zero(secure_ptr, 512);

    // Verify zeroing
    uint8_t *bytes = (uint8_t *)secure_ptr;
    bool is_zeroed = true;
    for (size_t i = 0; i < 512; i++) {
        if (bytes[i] != 0) {
            is_zeroed = false;
            break;
        }
    }
    TEST_ASSERT(is_zeroed, "Secure zeroing works correctly");

    // Clean up
    Asthra_secure_free(secure_ptr, 512);
}

void test_secure_slice_operations(void) {
    TEST_SECTION("Secure Slice Operations");

    // Test slice-based secure zeroing
    uint8_t test_data[256];
    memset(test_data, 0xCC, sizeof(test_data));

    AsthraFFISliceHeader secure_slice = Asthra_slice_from_raw_parts(
        test_data, 256, sizeof(uint8_t), true, ASTHRA_OWNERSHIP_TRANSFER_NONE);

    TEST_ASSERT(Asthra_slice_is_valid(secure_slice), "Secure slice creation succeeds");

    // Verify initial data
    uint8_t initial_value;
    AsthraFFIResult get_result = Asthra_slice_get_element(secure_slice, 100, &initial_value);
    TEST_ASSERT(Asthra_result_is_ok(get_result), "Initial secure slice access succeeds");
    TEST_ASSERT(initial_value == 0xCC, "Initial secure slice data is correct");

    // Perform secure zeroing
    Asthra_secure_zero_slice(secure_slice);

    // Verify slice zeroing
    bool is_zeroed = true;
    for (size_t i = 0; i < 256; i++) {
        if (test_data[i] != 0) {
            is_zeroed = false;
            break;
        }
    }
    TEST_ASSERT(is_zeroed, "Slice-based secure zeroing works correctly");
}

void test_secure_key_management(void) {
    TEST_SECTION("Secure Key Management");

    // Simulate cryptographic key storage
    const size_t key_size = 32; // 256-bit key
    void *key_buffer = Asthra_secure_alloc(key_size);
    TEST_ASSERT(key_buffer != NULL, "Secure key allocation succeeds");

    // Fill with "key material"
    uint8_t *key_bytes = (uint8_t *)key_buffer;
    for (size_t i = 0; i < key_size; i++) {
        key_bytes[i] = (uint8_t)(i ^ 0x5A); // Simple pattern
    }

    // Create secure slice for key
    AsthraFFISliceHeader key_slice = Asthra_slice_from_raw_parts(
        key_buffer, key_size, sizeof(uint8_t), true, ASTHRA_OWNERSHIP_TRANSFER_NONE);

    TEST_ASSERT(Asthra_slice_is_valid(key_slice), "Key slice creation succeeds");

    // Test key access
    uint8_t key_byte;
    AsthraFFIResult access_result = Asthra_slice_get_element(key_slice, 10, &key_byte);
    TEST_ASSERT(Asthra_result_is_ok(access_result), "Key access succeeds");
    TEST_ASSERT(key_byte == (10 ^ 0x5A), "Key data is correct");

    // Secure cleanup
    Asthra_secure_zero_slice(key_slice);
    Asthra_secure_free(key_buffer, key_size);
    TEST_ASSERT(true, "Secure key cleanup completes");
}

// =============================================================================
// VARIANT ARRAY TESTS
// =============================================================================

void test_variant_array_creation(void) {
    TEST_SECTION("Variant Array Creation");

    // Create variant array
    AsthraVariantArray array = Asthra_variant_array_new(4);
    TEST_ASSERT(array.args != NULL, "Variant array creation succeeds");
    TEST_ASSERT(array.capacity == 4, "Variant array has correct initial capacity");
    TEST_ASSERT(array.count == 0, "Variant array starts empty");

    // Clean up
    Asthra_variant_array_free(array);
}

void test_variant_array_operations(void) {
    TEST_SECTION("Variant Array Operations");

    AsthraVariantArray array = Asthra_variant_array_new(3);

    // Add various types of variants
    AsthraVariant bool_var = {.type = ASTHRA_VARIANT_BOOL, .value.bool_val = true};
    AsthraVariant int_var = {.type = ASTHRA_VARIANT_I32, .value.i32_val = 123};
    AsthraVariant float_var = {.type = ASTHRA_VARIANT_F64, .value.f64_val = 2.718};

    AsthraFFIResult push_result = Asthra_variant_array_push(&array, bool_var);
    TEST_ASSERT(Asthra_result_is_ok(push_result), "Boolean variant push succeeds");

    push_result = Asthra_variant_array_push(&array, int_var);
    TEST_ASSERT(Asthra_result_is_ok(push_result), "Integer variant push succeeds");

    push_result = Asthra_variant_array_push(&array, float_var);
    TEST_ASSERT(Asthra_result_is_ok(push_result), "Float variant push succeeds");

    TEST_ASSERT(array.count == 3, "Variant array count is correct");

    // Test element retrieval
    AsthraFFIResult get_result = Asthra_variant_array_get(array, 1);
    TEST_ASSERT(Asthra_result_is_ok(get_result), "Variant retrieval succeeds");

    AsthraVariant *retrieved = (AsthraVariant *)Asthra_result_unwrap_ok(get_result);
    TEST_ASSERT(retrieved->type == ASTHRA_VARIANT_I32, "Retrieved variant has correct type");
    TEST_ASSERT(retrieved->value.i32_val == 123, "Retrieved variant has correct value");

    // Test bounds checking
    get_result = Asthra_variant_array_get(array, 100);
    TEST_ASSERT(Asthra_result_is_err(get_result), "Out-of-bounds access fails as expected");

    // Clean up
    free(retrieved);
    Asthra_variant_array_free(array);
}

void test_variant_array_growth(void) {
    TEST_SECTION("Variant Array Growth");

    AsthraVariantArray array = Asthra_variant_array_new(2);
    TEST_ASSERT(array.capacity == 2, "Initial capacity is correct");

    // Fill beyond initial capacity
    for (int i = 0; i < 5; i++) {
        AsthraVariant var = {.type = ASTHRA_VARIANT_I32, .value.i32_val = i * 10};
        AsthraFFIResult push_result = Asthra_variant_array_push(&array, var);
        TEST_ASSERT(Asthra_result_is_ok(push_result), "Array growth push succeeds");
    }

    TEST_ASSERT(array.count == 5, "Array count after growth is correct");
    TEST_ASSERT(array.capacity >= 5, "Array capacity grew as expected");

    // Verify all elements
    for (int i = 0; i < 5; i++) {
        AsthraFFIResult get_result = Asthra_variant_array_get(array, i);
        TEST_ASSERT(Asthra_result_is_ok(get_result), "Post-growth element access succeeds");

        AsthraVariant *var = (AsthraVariant *)Asthra_result_unwrap_ok(get_result);
        TEST_ASSERT(var->type == ASTHRA_VARIANT_I32, "Post-growth element has correct type");
        TEST_ASSERT(var->value.i32_val == i * 10, "Post-growth element has correct value");
        free(var);
    }

    Asthra_variant_array_free(array);
}

void test_variant_types(void) {
    TEST_SECTION("Variant Type Support");

    AsthraVariantArray array = Asthra_variant_array_new(10);

    // Test all supported variant types
    AsthraVariant variants[] = {
        {.type = ASTHRA_VARIANT_BOOL, .value.bool_val = false},
        {.type = ASTHRA_VARIANT_I8, .value.i8_val = -42},
        {.type = ASTHRA_VARIANT_U8, .value.u8_val = 200},
        {.type = ASTHRA_VARIANT_I16, .value.i16_val = -1000},
        {.type = ASTHRA_VARIANT_U16, .value.u16_val = 50000},
        {.type = ASTHRA_VARIANT_I32, .value.i32_val = -100000},
        {.type = ASTHRA_VARIANT_U32, .value.u32_val = 4000000000U},
        {.type = ASTHRA_VARIANT_I64, .value.i64_val = -9000000000000LL},
        {.type = ASTHRA_VARIANT_U64, .value.u64_val = 18000000000000000000ULL},
        {.type = ASTHRA_VARIANT_F64, .value.f64_val = 3.141592653589793}};

    // Add all variants
    for (size_t i = 0; i < sizeof(variants) / sizeof(variants[0]); i++) {
        AsthraFFIResult push_result = Asthra_variant_array_push(&array, variants[i]);
        TEST_ASSERT(Asthra_result_is_ok(push_result), "Variant type push succeeds");
    }

    // Verify all variants
    for (size_t i = 0; i < sizeof(variants) / sizeof(variants[0]); i++) {
        AsthraFFIResult get_result = Asthra_variant_array_get(array, i);
        TEST_ASSERT(Asthra_result_is_ok(get_result), "Variant type retrieval succeeds");

        AsthraVariant *retrieved = (AsthraVariant *)Asthra_result_unwrap_ok(get_result);
        TEST_ASSERT(retrieved->type == variants[i].type, "Variant type preserved");

        // Type-specific value checks
        switch (variants[i].type) {
        case ASTHRA_VARIANT_BOOL:
            TEST_ASSERT(retrieved->value.bool_val == variants[i].value.bool_val,
                        "Bool value preserved");
            break;
        case ASTHRA_VARIANT_I32:
            TEST_ASSERT(retrieved->value.i32_val == variants[i].value.i32_val,
                        "I32 value preserved");
            break;
        case ASTHRA_VARIANT_F64:
            TEST_ASSERT(retrieved->value.f64_val == variants[i].value.f64_val,
                        "F64 value preserved");
            break;
        // Add more cases as needed
        default:
            break;
        }

        free(retrieved);
    }

    Asthra_variant_array_free(array);
}

void test_variant_string_handling(void) {
    TEST_SECTION("Variant String Handling");

    AsthraVariantArray array = Asthra_variant_array_new(3);

    // Create string variants
    AsthraFFIString str1 = Asthra_string_from_cstr("Hello", ASTHRA_OWNERSHIP_TRANSFER_FULL);
    AsthraFFIString str2 = Asthra_string_from_cstr("World", ASTHRA_OWNERSHIP_TRANSFER_FULL);

    AsthraVariant str_var1 = {.type = ASTHRA_VARIANT_STRING, .value.string_val = str1};
    AsthraVariant str_var2 = {.type = ASTHRA_VARIANT_STRING, .value.string_val = str2};

    // Add to array
    AsthraFFIResult push1 = Asthra_variant_array_push(&array, str_var1);
    AsthraFFIResult push2 = Asthra_variant_array_push(&array, str_var2);

    TEST_ASSERT(Asthra_result_is_ok(push1), "String variant push 1 succeeds");
    TEST_ASSERT(Asthra_result_is_ok(push2), "String variant push 2 succeeds");

    // Retrieve and verify
    AsthraFFIResult get_result = Asthra_variant_array_get(array, 0);
    TEST_ASSERT(Asthra_result_is_ok(get_result), "String variant retrieval succeeds");

    AsthraVariant *retrieved = (AsthraVariant *)Asthra_result_unwrap_ok(get_result);
    TEST_ASSERT(retrieved->type == ASTHRA_VARIANT_STRING, "String variant type preserved");
    TEST_ASSERT(retrieved->value.string_val.len == 5, "String variant length preserved");
    TEST_ASSERT(strncmp(retrieved->value.string_val.data, "Hello", 5) == 0,
                "String variant content preserved");

    // Clean up
    free(retrieved);
    Asthra_variant_array_free(array); // This should clean up the strings too
}

// Test registration
static test_case_t security_tests[] = {{"Secure Allocation", test_secure_allocation},
                                       {"Secure Zeroing", test_secure_zeroing},
                                       {"Secure Slice Operations", test_secure_slice_operations},
                                       {"Secure Key Management", test_secure_key_management},
                                       {"Variant Array Creation", test_variant_array_creation},
                                       {"Variant Array Operations", test_variant_array_operations},
                                       {"Variant Array Growth", test_variant_array_growth},
                                       {"Variant Types", test_variant_types},
                                       {"Variant String Handling", test_variant_string_handling}};

int main(void) {
    printf("Asthra Safe C Memory Interface - Security & Variants Tests\n");
    printf("=========================================================\n");

    test_runtime_init();

    int failed = run_test_suite("Security & Variants", security_tests,
                                sizeof(security_tests) / sizeof(security_tests[0]));

    print_test_results();
    test_runtime_cleanup();

    return failed == 0 ? 0 : 1;
}
