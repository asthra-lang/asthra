/**
 * Test Suite for Asthra Safe C Memory Interface - Integration Scenarios
 * Tests for real-world usage patterns and complex integration scenarios
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_ffi_memory_common.h"

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

void test_pqc_key_management_scenario(void) {
    TEST_SECTION("PQC Key Management Scenario");

    printf("Simulating Post-Quantum Cryptography key management...\n");

    // Simulate large cryptographic keys (variable size)
    size_t key_sizes[] = {1024, 2048, 4096, 8192}; // Different PQC key sizes
    AsthraFFISliceHeader key_slices[4];

    for (int i = 0; i < 4; i++) {
        key_slices[i] =
            Asthra_slice_new(sizeof(uint8_t), key_sizes[i], key_sizes[i], ASTHRA_ZONE_HINT_SECURE);
        TEST_ASSERT(Asthra_slice_is_valid(key_slices[i]), "PQC key slice creation succeeds");

        // Fill with simulated key data
        uint8_t *key_data = (uint8_t *)Asthra_slice_get_ptr(key_slices[i]);
        for (size_t j = 0; j < key_sizes[i]; j++) {
            key_data[j] = (uint8_t)((j * 31 + i * 17) % 256); // Pseudo-random pattern
        }

        printf("  Created %zu-byte key in secure zone\n", key_sizes[i]);
    }

    // Test key access patterns
    for (int i = 0; i < 4; i++) {
        // Verify key integrity
        uint8_t sample_byte;
        AsthraFFIResult access_result = Asthra_slice_get_element(key_slices[i], 100, &sample_byte);
        TEST_ASSERT(Asthra_result_is_ok(access_result), "Key integrity check succeeds");

        uint8_t expected = (uint8_t)((100 * 31 + i * 17) % 256);
        TEST_ASSERT(sample_byte == expected, "Key data integrity verified");
    }

    // Secure cleanup of all keys
    for (int i = 0; i < 4; i++) {
        Asthra_secure_zero_slice(key_slices[i]); // Secure cleanup
        Asthra_slice_free(key_slices[i]);
        printf("  Securely destroyed %zu-byte key\n", key_sizes[i]);
    }

    TEST_ASSERT(true, "PQC key management scenario completed successfully");
}

void test_ffi_string_processing_scenario(void) {
    TEST_SECTION("FFI String Processing Scenario");

    printf("Simulating FFI string processing workflow...\n");

    // Create base messages
    AsthraFFIString base_messages[3];
    base_messages[0] =
        Asthra_string_from_cstr("Processing operation: ", ASTHRA_OWNERSHIP_TRANSFER_FULL);
    base_messages[1] = Asthra_string_from_cstr("Status: ", ASTHRA_OWNERSHIP_TRANSFER_FULL);
    base_messages[2] = Asthra_string_from_cstr("Completed in ", ASTHRA_OWNERSHIP_TRANSFER_FULL);

    TEST_ASSERT(base_messages[0].data != NULL, "Base message 1 creation succeeds");
    TEST_ASSERT(base_messages[1].data != NULL, "Base message 2 creation succeeds");
    TEST_ASSERT(base_messages[2].data != NULL, "Base message 3 creation succeeds");

    // Simulate processing different operations
    const char *operations[] = {"Encryption", "Decryption", "Signing", "Verification"};
    const char *statuses[] = {"SUCCESS", "PENDING", "ERROR", "RETRY"};
    const double times[] = {1.23, 4.56, 0.89, 2.34};

    for (int i = 0; i < 4; i++) {
        // Create operation-specific messages using interpolation
        AsthraVariantArray args = Asthra_variant_array_new(4);

        AsthraFFIString op_str =
            Asthra_string_from_cstr(operations[i], ASTHRA_OWNERSHIP_TRANSFER_FULL);
        AsthraFFIString status_str =
            Asthra_string_from_cstr(statuses[i], ASTHRA_OWNERSHIP_TRANSFER_FULL);

        AsthraVariant op_var = {.type = ASTHRA_VARIANT_STRING, .value.string_val = op_str};
        AsthraVariant status_var = {.type = ASTHRA_VARIANT_STRING, .value.string_val = status_str};
        AsthraVariant time_var = {.type = ASTHRA_VARIANT_F64, .value.f64_val = times[i]};
        AsthraVariant id_var = {.type = ASTHRA_VARIANT_I32, .value.i32_val = i + 1};

        Asthra_variant_array_push(&args, op_var);
        Asthra_variant_array_push(&args, status_var);
        Asthra_variant_array_push(&args, time_var);
        Asthra_variant_array_push(&args, id_var);

        AsthraFFIResult interp_result =
            Asthra_string_interpolate("Operation[{}]: {} - Status: {} - Time: {:.2f}s", args);
        TEST_ASSERT(Asthra_result_is_ok(interp_result), "String interpolation succeeds");

        AsthraFFIString *final_msg = (AsthraFFIString *)Asthra_result_unwrap_ok(interp_result);
        printf("  %.*s\n", (int)final_msg->len, final_msg->data);

        // Convert to C string for external processing
        char *c_msg = Asthra_string_to_cstr(*final_msg, false);
        TEST_ASSERT(c_msg != NULL, "String to C string conversion succeeds");
        TEST_ASSERT(strlen(c_msg) == final_msg->len, "Converted string length matches");

        // Clean up
        free(c_msg);
        Asthra_string_free(*final_msg);
        free(final_msg);
        Asthra_variant_array_free(args); // This cleans up op_str and status_str
    }

    // Clean up base messages
    for (int i = 0; i < 3; i++) {
        Asthra_string_free(base_messages[i]);
    }

    TEST_ASSERT(true, "FFI string processing scenario completed successfully");
}

void test_error_handling_chain_scenario(void) {
    TEST_SECTION("Error Handling Chain Scenario");

    printf("Simulating complex error handling workflow...\n");

    // Create a slice for testing error propagation
    AsthraFFISliceHeader test_slice =
        Asthra_slice_new(sizeof(int), 10, 20, ASTHRA_ZONE_HINT_MANUAL);
    TEST_ASSERT(Asthra_slice_is_valid(test_slice), "Test slice creation succeeds");

    // Initialize with test data
    for (int i = 0; i < 10; i++) {
        int value = i * i; // Square values
        AsthraFFIResult set_result = Asthra_slice_set_element(test_slice, i, &value);
        TEST_ASSERT(Asthra_result_is_ok(set_result), "Slice initialization succeeds");
    }

    // Simulate a chain of operations with potential failures
    bool chain_success = true;
    char error_log[1000] = {0};

    // Operation 1: Valid access
    AsthraFFIResult op1_result = Asthra_slice_bounds_check(test_slice, 5);
    if (Asthra_result_is_ok(op1_result)) {
        int element;
        AsthraFFIResult get_result = Asthra_slice_get_element(test_slice, 5, &element);
        if (Asthra_result_is_ok(get_result)) {
            printf("  Op1 SUCCESS: Retrieved element[5] = %d\n", element);
            TEST_ASSERT(element == 25, "Retrieved value is correct");
        } else {
            snprintf(error_log + strlen(error_log), sizeof(error_log) - strlen(error_log),
                     "Op1 GET_FAIL: %s; ", Asthra_result_get_error_message(get_result));
            chain_success = false;
        }
    } else {
        snprintf(error_log + strlen(error_log), sizeof(error_log) - strlen(error_log),
                 "Op1 BOUNDS_FAIL: %s; ", Asthra_result_get_error_message(op1_result));
        chain_success = false;
    }

    // Operation 2: Invalid access (should fail)
    AsthraFFIResult op2_result = Asthra_slice_bounds_check(test_slice, 15);
    if (Asthra_result_is_ok(op2_result)) {
        // This shouldn't happen
        snprintf(error_log + strlen(error_log), sizeof(error_log) - strlen(error_log),
                 "Op2 UNEXPECTED_SUCCESS; ");
        TEST_ASSERT(false, "Invalid bounds check should have failed");
    } else {
        printf("  Op2 EXPECTED_FAIL: %s\n", Asthra_result_get_error_message(op2_result));
        // This is expected behavior
    }

    // Operation 3: Recovery operation
    AsthraFFIResult op3_result = Asthra_slice_bounds_check(test_slice, 9);
    if (Asthra_result_is_ok(op3_result)) {
        int element;
        AsthraFFIResult get_result = Asthra_slice_get_element(test_slice, 9, &element);
        if (Asthra_result_is_ok(get_result)) {
            printf("  Op3 RECOVERY_SUCCESS: Retrieved element[9] = %d\n", element);
            TEST_ASSERT(element == 81, "Recovery operation retrieved correct value");
        } else {
            snprintf(error_log + strlen(error_log), sizeof(error_log) - strlen(error_log),
                     "Op3 RECOVERY_FAIL: %s; ", Asthra_result_get_error_message(get_result));
            chain_success = false;
        }
    } else {
        snprintf(error_log + strlen(error_log), sizeof(error_log) - strlen(error_log),
                 "Op3 RECOVERY_BOUNDS_FAIL: %s; ", Asthra_result_get_error_message(op3_result));
        chain_success = false;
    }

    if (strlen(error_log) > 0) {
        printf("  Error log: %s\n", error_log);
    }

    // Subslice operation with error handling
    AsthraFFIResult subslice_result = Asthra_slice_subslice(test_slice, 2, 8);
    if (Asthra_result_is_ok(subslice_result)) {
        AsthraFFISliceHeader *subslice =
            (AsthraFFISliceHeader *)Asthra_result_unwrap_ok(subslice_result);
        printf("  Subslice creation SUCCESS: length = %zu\n", Asthra_slice_get_len(*subslice));

        // Test subslice access
        int sub_element;
        AsthraFFIResult sub_get = Asthra_slice_get_element(*subslice, 2, &sub_element);
        if (Asthra_result_is_ok(sub_get)) {
            printf("  Subslice access SUCCESS: element[2] = %d\n", sub_element);
            TEST_ASSERT(sub_element == 16, "Subslice element has correct value"); // (2+2)^2 = 16
        }

        free(subslice);
    } else {
        printf("  Subslice creation FAILED: %s\n",
               Asthra_result_get_error_message(subslice_result));
        chain_success = false;
    }

    TEST_ASSERT(chain_success, "Error handling chain completed with expected results");

    // Clean up
    Asthra_slice_free(test_slice);
}

void test_mixed_allocation_scenario(void) {
    TEST_SECTION("Mixed Allocation Scenario");

    printf("Simulating mixed allocation patterns across zones...\n");

    // Track allocations for cleanup
    void *manual_ptrs[3];
    void *gc_ptrs[2];
    void *secure_ptrs[2];
    AsthraFFISliceHeader slices[3];
    AsthraFFIString strings[2];

    // Phase 1: Manual zone allocations
    printf("  Phase 1: Manual zone allocations\n");
    for (int i = 0; i < 3; i++) {
        manual_ptrs[i] = Asthra_ffi_alloc(256 * (i + 1), ASTHRA_ZONE_HINT_MANUAL);
        TEST_ASSERT(manual_ptrs[i] != NULL, "Manual allocation succeeds");

        // Fill with test pattern
        memset(manual_ptrs[i], 0xAA + i, 256 * (i + 1));
        printf("    Allocated %d bytes in manual zone\n", 256 * (i + 1));
    }

    // Phase 2: GC zone allocations
    printf("  Phase 2: GC zone allocations\n");
    for (int i = 0; i < 2; i++) {
        gc_ptrs[i] = Asthra_ffi_alloc(512 * (i + 1), ASTHRA_ZONE_HINT_GC);
        TEST_ASSERT(gc_ptrs[i] != NULL, "GC allocation succeeds");
        printf("    Allocated %d bytes in GC zone\n", 512 * (i + 1));
    }

    // Phase 3: Secure zone allocations
    printf("  Phase 3: Secure zone allocations\n");
    for (int i = 0; i < 2; i++) {
        secure_ptrs[i] = Asthra_secure_alloc(128 * (i + 1));
        TEST_ASSERT(secure_ptrs[i] != NULL, "Secure allocation succeeds");
        printf("    Allocated %d bytes in secure zone\n", 128 * (i + 1));
    }

    // Phase 4: Slice allocations
    printf("  Phase 4: Slice allocations\n");
    slices[0] = Asthra_slice_new(sizeof(int), 50, 100, ASTHRA_ZONE_HINT_MANUAL);
    slices[1] = Asthra_slice_new(sizeof(double), 25, 50, ASTHRA_ZONE_HINT_GC);
    slices[2] = Asthra_slice_new(sizeof(uint8_t), 200, 400, ASTHRA_ZONE_HINT_SECURE);

    for (int i = 0; i < 3; i++) {
        TEST_ASSERT(Asthra_slice_is_valid(slices[i]), "Slice allocation succeeds");
        printf("    Created slice with %zu elements\n", Asthra_slice_get_len(slices[i]));
    }

    // Phase 5: String allocations
    printf("  Phase 5: String allocations\n");
    strings[0] =
        Asthra_string_from_cstr("Mixed allocation test string 1", ASTHRA_OWNERSHIP_TRANSFER_FULL);
    strings[1] =
        Asthra_string_from_cstr("Mixed allocation test string 2", ASTHRA_OWNERSHIP_TRANSFER_FULL);

    for (int i = 0; i < 2; i++) {
        TEST_ASSERT(strings[i].data != NULL, "String allocation succeeds");
        printf("    Created string with %zu characters\n", strings[i].len);
    }

    // Check memory statistics
    AsthraFFIMemoryStats stats = Asthra_ffi_get_memory_stats();
    printf("  Memory statistics after all allocations:\n");
    printf("    Current allocations: %zu\n", stats.current_allocations);
    printf("    Current bytes: %zu\n", stats.current_bytes);
    printf("    Slice count: %zu\n", stats.slice_count);

    TEST_ASSERT(stats.current_allocations >= 12, "All allocations tracked"); // 3+2+2+3+2

    // Phase 6: Interleaved operations
    printf("  Phase 6: Interleaved operations\n");

    // Test slice operations
    int test_value = 42;
    AsthraFFIResult slice_op = Asthra_slice_set_element(slices[0], 10, &test_value);
    TEST_ASSERT(Asthra_result_is_ok(slice_op), "Slice operation succeeds");

    // Test string concatenation
    AsthraFFIResult concat_result = Asthra_string_concat(strings[0], strings[1]);
    TEST_ASSERT(Asthra_result_is_ok(concat_result), "String concatenation succeeds");
    AsthraFFIString *concat_str = (AsthraFFIString *)Asthra_result_unwrap_ok(concat_result);
    printf("    Concatenated string length: %zu\n", concat_str->len);

    // Phase 7: Cleanup in reverse order
    printf("  Phase 7: Cleanup\n");

    Asthra_string_free(*concat_str);
    free(concat_str);

    for (int i = 0; i < 2; i++) {
        Asthra_string_free(strings[i]);
    }

    for (int i = 0; i < 3; i++) {
        Asthra_slice_free(slices[i]);
    }

    for (int i = 0; i < 2; i++) {
        Asthra_secure_zero(secure_ptrs[i], 128 * (i + 1));
        Asthra_secure_free(secure_ptrs[i], 128 * (i + 1));
    }

    for (int i = 0; i < 2; i++) {
        Asthra_ffi_free(gc_ptrs[i], ASTHRA_ZONE_HINT_GC);
    }

    for (int i = 0; i < 3; i++) {
        Asthra_ffi_free(manual_ptrs[i], ASTHRA_ZONE_HINT_MANUAL);
    }

    // Final statistics
    AsthraFFIMemoryStats final_stats = Asthra_ffi_get_memory_stats();
    printf("  Final statistics:\n");
    printf("    Current allocations: %zu\n", final_stats.current_allocations);
    printf("    Current bytes: %zu\n", final_stats.current_bytes);

    TEST_ASSERT(true, "Mixed allocation scenario completed successfully");
}

void test_stress_testing_scenario(void) {
    TEST_SECTION("Stress Testing Scenario");

    printf("Running stress tests with rapid allocation/deallocation...\n");

    const int stress_iterations = 500;
    const int batch_size = 10;

    AsthraFFIMemoryStats initial_stats = Asthra_ffi_get_memory_stats();

    for (int iteration = 0; iteration < stress_iterations; iteration++) {
        void *stress_ptrs[batch_size];

        // Rapid allocations
        for (int i = 0; i < batch_size; i++) {
            size_t size = 64 + (iteration + i) % 512; // Variable sizes
            stress_ptrs[i] = Asthra_ffi_alloc(size, ASTHRA_ZONE_HINT_MANUAL);
            TEST_ASSERT(stress_ptrs[i] != NULL, "Stress allocation succeeds");

            // Write pattern to ensure allocation is valid
            memset(stress_ptrs[i], (iteration + i) % 256, size);
        }

        // Verify some allocations
        if (iteration % 100 == 0) {
            for (int i = 0; i < batch_size; i++) {
                uint8_t *bytes = (uint8_t *)stress_ptrs[i];
                uint8_t expected = (iteration + i) % 256;
                TEST_ASSERT(bytes[0] == expected, "Stress allocation data integrity verified");
            }
            printf("  Iteration %d: Verified allocations\n", iteration);
        }

        // Rapid deallocations
        for (int i = 0; i < batch_size; i++) {
            Asthra_ffi_free(stress_ptrs[i], ASTHRA_ZONE_HINT_MANUAL);
        }
    }

    AsthraFFIMemoryStats final_stats = Asthra_ffi_get_memory_stats();

    printf("  Stress test completed:\n");
    printf("    Iterations: %d\n", stress_iterations);
    printf("    Total operations: %d\n", stress_iterations * batch_size * 2);
    printf("    Final allocations: %zu (should be close to initial: %zu)\n",
           final_stats.current_allocations, initial_stats.current_allocations);

    TEST_ASSERT(final_stats.current_allocations <= initial_stats.current_allocations + 5,
                "Stress test doesn't leak significant memory");

    TEST_ASSERT(true, "Stress testing scenario completed successfully");
}

// Test registration
static test_case_t integration_tests[] = {
    {"PQC Key Management", test_pqc_key_management_scenario},
    {"FFI String Processing", test_ffi_string_processing_scenario},
    {"Error Handling Chain", test_error_handling_chain_scenario},
    {"Mixed Allocation Patterns", test_mixed_allocation_scenario},
    {"Stress Testing", test_stress_testing_scenario}};

int main(void) {
    printf("Asthra Safe C Memory Interface - Integration Tests\n");
    printf("=================================================\n");

    test_runtime_init();

    int failed = run_test_suite("Integration Scenarios", integration_tests,
                                sizeof(integration_tests) / sizeof(integration_tests[0]));

    // Final comprehensive memory state
    printf("\nFinal comprehensive memory state:\n");
    Asthra_ffi_dump_memory_state(stdout);

    print_test_results();
    test_runtime_cleanup();

    return failed == 0 ? 0 : 1;
}
