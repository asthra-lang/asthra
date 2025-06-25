/**
 * Asthra Programming Language Compiler
 * Static Analysis Integration Test Suite - Const Correctness Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 5.2: Const Correctness Tests
 * - Test const correctness improvements
 * - Validate string operations with proper const usage
 * - Test memory operations with const pointers
 */

#include "test_static_analysis_common.h"

// =============================================================================
// CONST CORRECTNESS TESTS
// =============================================================================

AsthraTestResult test_const_correct_string_operations(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Test string length
    const char *test_str = "Hello, Asthra!";
    size_t length = asthra_string_length(test_str);

    if (!asthra_test_assert_size_eq(context, length, 14, "String length should be 14")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test string comparison
    const char *str1 = "Hello";
    const char *str2 = "Hello";
    const char *str3 = "World";

    int cmp1 = asthra_string_compare(str1, str2);
    int cmp2 = asthra_string_compare(str1, str3);

    if (!asthra_test_assert_int_eq(context, cmp1, 0, "Identical strings should compare equal")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, cmp2 != 0,
                                 "Different strings should not compare equal")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test string copy
    char dest[32];
    char *result = asthra_string_copy(dest, test_str, sizeof(dest));

    if (!asthra_test_assert_pointer_eq(context, result, dest,
                                       "String copy should return dest pointer")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, strings_equal(dest, test_str),
                                 "Copied string should match original")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test string concatenation
    strcpy(dest, "Hello, ");
    result = asthra_string_concatenate(dest, "Asthra!", sizeof(dest), 7);

    if (!asthra_test_assert_bool(context, strings_equal(dest, "Hello, Asthra!"),
                                 "Concatenated string should be correct")) {
        return ASTHRA_TEST_FAIL;
    }

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_const_correct_memory_operations(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Test memory operations
    char src[] = "Test data for memory operations";
    char dest[64];

    // Test memory copy
    void *result = asthra_memory_copy(dest, src, strlen(src) + 1);

    if (!asthra_test_assert_pointer_eq(context, result, dest,
                                       "Memory copy should return dest pointer")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, strings_equal(dest, src),
                                 "Copied memory should match original")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test memory comparison
    int cmp = asthra_memory_compare(src, dest, strlen(src) + 1);

    if (!asthra_test_assert_int_eq(context, cmp, 0, "Identical memory should compare equal")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test memory set
    char buffer[32];
    result = asthra_memory_set(buffer, 0xAA, sizeof(buffer));

    if (!asthra_test_assert_pointer_eq(context, result, buffer,
                                       "Memory set should return buffer pointer")) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify all bytes are set correctly
    bool all_set = true;
    for (size_t i = 0; i < sizeof(buffer); i++) {
        if ((unsigned char)buffer[i] != 0xAA) {
            all_set = false;
            break;
        }
    }

    if (!asthra_test_assert_bool(context, all_set, "All bytes should be set to 0xAA")) {
        return ASTHRA_TEST_FAIL;
    }

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

/**
 * Main test runner
 */
int main(void) {
    printf("=== Asthra Integration Tests - Const Correctness ===\n\n");

    // Create test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        printf("Failed to create test statistics\n");
        return 1;
    }

    // Create test metadata
    AsthraTestMetadata metadata1 = {.name = "const_correct_string_operations",
                                    .file = __FILE__,
                                    .line = __LINE__,
                                    .function = "test_const_correct_string_operations",
                                    .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                    .timeout_ns = 30000000000ULL,
                                    .skip = false,
                                    .skip_reason = NULL};

    AsthraTestMetadata metadata2 = {.name = "const_correct_memory_operations",
                                    .file = __FILE__,
                                    .line = __LINE__,
                                    .function = "test_const_correct_memory_operations",
                                    .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                    .timeout_ns = 30000000000ULL,
                                    .skip = false,
                                    .skip_reason = NULL};

    AsthraTestContext *context1 = asthra_test_context_create(&metadata1, stats);
    AsthraTestContext *context2 = asthra_test_context_create(&metadata2, stats);

    if (!context1 || !context2) {
        printf("Failed to create test contexts\n");
        if (context1)
            asthra_test_context_destroy(context1);
        if (context2)
            asthra_test_context_destroy(context2);
        asthra_test_statistics_destroy(stats);
        return 1;
    }

    // Run tests
    AsthraTestResult result1 = test_const_correct_string_operations(context1);
    AsthraTestResult result2 = test_const_correct_memory_operations(context2);

    // Report results
    printf("Const correct string operations: %s\n", result1 == ASTHRA_TEST_PASS ? "PASS" : "FAIL");
    printf("Const correct memory operations: %s\n", result2 == ASTHRA_TEST_PASS ? "PASS" : "FAIL");

    // Print statistics
    printf("\n=== Test Statistics ===\n");
    printf("Tests run:       %d\n", 2);
    printf("Tests passed:    %d\n",
           (result1 == ASTHRA_TEST_PASS ? 1 : 0) + (result2 == ASTHRA_TEST_PASS ? 1 : 0));
    printf("Tests failed:    %d\n",
           (result1 == ASTHRA_TEST_FAIL ? 1 : 0) + (result2 == ASTHRA_TEST_FAIL ? 1 : 0));
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
    printf("Integration tests: %d/%d passed\n",
           (result1 == ASTHRA_TEST_PASS ? 1 : 0) + (result2 == ASTHRA_TEST_PASS ? 1 : 0), 2);

    // Cleanup
    asthra_test_context_destroy(context1);
    asthra_test_context_destroy(context2);
    asthra_test_statistics_destroy(stats);

    return (result1 == ASTHRA_TEST_PASS && result2 == ASTHRA_TEST_PASS) ? 0 : 1;
}
