/**
 * Asthra Programming Language v1.2 String FFI Integration Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for string FFI integration, including C string conversion,
 * ownership transfer, and borrowed references.
 */

#include "test_string_helpers.h"

// =============================================================================
// MOCK FFI FUNCTIONS
// =============================================================================

// Mock FFI function that takes ownership of a string
static char *mock_ffi_take_ownership(TestString *str) {
    if (!str)
        return NULL;

    char *c_copy = malloc(str->length + 1);
    if (!c_copy)
        return NULL;

    strcpy(c_copy, str->data);

    // In real implementation, this would transfer ownership
    str->is_gc_managed = false; // Mark as no longer GC-managed

    return c_copy;
}

// Mock FFI function that returns a string with transferred ownership
static TestString *mock_ffi_return_ownership(const char *str) {
    TestString *result = test_string_create(str);
    if (result) {
        result->is_gc_managed = true; // Mark as GC-managed
    }
    return result;
}

// Mock FFI function that borrows a string (read-only access)
static size_t mock_ffi_get_length(const TestString *str) {
    return str ? str->length : 0;
}

// Mock FFI function that borrows string data
static const char *mock_ffi_get_data(const TestString *str) {
    return str ? str->data : NULL;
}

// =============================================================================
// FFI INTEGRATION TESTS
// =============================================================================

AsthraTestResult test_string_c_string_conversion(AsthraTestContext *context) {
    // Test conversion between Asthra strings and C strings

    const char *test_strings[] = {"Hello, World!",      "",
                                  "Unicode: 世界",      "Special chars: \n\t\r\"'\\",
                                  "Numbers: 123456789", NULL};

    for (int i = 0; test_strings[i] != NULL; i++) {
        // Convert C string to Asthra string
        TestString *asthra_string = test_string_create(test_strings[i]);

        if (!ASTHRA_TEST_ASSERT(context, asthra_string != NULL,
                                "Failed to convert C string to Asthra string: %s",
                                test_strings[i])) {
            return ASTHRA_TEST_FAIL;
        }

        // Convert back to C string
        const char *c_string = asthra_string->data;

        if (!ASTHRA_TEST_ASSERT(context, strcmp(c_string, test_strings[i]) == 0,
                                "C string conversion mismatch: expected '%s', got '%s'",
                                test_strings[i], c_string)) {
            test_string_destroy(asthra_string);
            return ASTHRA_TEST_FAIL;
        }

        // Test null termination
        if (!ASTHRA_TEST_ASSERT(context, c_string[asthra_string->length] == '\0',
                                "C string not properly null-terminated")) {
            test_string_destroy(asthra_string);
            return ASTHRA_TEST_FAIL;
        }

        test_string_destroy(asthra_string);
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_string_ffi_ownership_transfer(AsthraTestContext *context) {
    // Test ownership transfer in FFI scenarios

    // Test transferring ownership to C
    TestString *asthra_string = test_string_create("Transfer test");

    if (!ASTHRA_TEST_ASSERT(context, asthra_string != NULL, "Failed to create test string")) {
        return ASTHRA_TEST_FAIL;
    }

    char *c_string = mock_ffi_take_ownership(asthra_string);

    if (!ASTHRA_TEST_ASSERT(context, c_string != NULL, "FFI ownership transfer failed")) {
        test_string_destroy(asthra_string);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, strcmp(c_string, "Transfer test") == 0,
                            "FFI transferred string content incorrect")) {
        test_string_destroy(asthra_string);
        free(c_string);
        return ASTHRA_TEST_FAIL;
    }

    // Test receiving ownership from C
    TestString *returned_string = mock_ffi_return_ownership("Returned from C");

    if (!ASTHRA_TEST_ASSERT(context, returned_string != NULL, "FFI return ownership failed")) {
        test_string_destroy(asthra_string);
        free(c_string);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, returned_string->is_gc_managed,
                            "Returned string should be GC-managed")) {
        test_string_destroy(asthra_string);
        free(c_string);
        test_string_destroy(returned_string);
        return ASTHRA_TEST_FAIL;
    }

    test_string_destroy(asthra_string);
    free(c_string);
    test_string_destroy(returned_string);

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_string_ffi_borrowed_references(AsthraTestContext *context) {
    // Test borrowed references in FFI scenarios

    TestString *test_string = test_string_create("Borrowed reference test");

    if (!ASTHRA_TEST_ASSERT(context, test_string != NULL, "Failed to create test string")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test borrowing for length
    size_t borrowed_length = mock_ffi_get_length(test_string);

    if (!ASTHRA_TEST_ASSERT(context, borrowed_length == test_string->length,
                            "Borrowed length incorrect: expected %zu, got %zu", test_string->length,
                            borrowed_length)) {
        test_string_destroy(test_string);
        return ASTHRA_TEST_FAIL;
    }

    // Test borrowing for data access
    const char *borrowed_data = mock_ffi_get_data(test_string);

    if (!ASTHRA_TEST_ASSERT(context, borrowed_data == test_string->data,
                            "Borrowed data pointer incorrect")) {
        test_string_destroy(test_string);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, strcmp(borrowed_data, "Borrowed reference test") == 0,
                            "Borrowed data content incorrect")) {
        test_string_destroy(test_string);
        return ASTHRA_TEST_FAIL;
    }

    // Verify original string is unchanged after borrowing
    if (!ASTHRA_TEST_ASSERT(context, strcmp(test_string->data, "Borrowed reference test") == 0,
                            "Original string modified after borrowing")) {
        test_string_destroy(test_string);
        return ASTHRA_TEST_FAIL;
    }

    test_string_destroy(test_string);

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

#ifndef ASTHRA_TEST_LIBRARY_MODE
int main(void) {
    printf("=== Asthra String FFI Test Suite ===\n");

    // Create basic test context
    AsthraTestContext ctx = {.metadata = {.name = "String FFI Tests",
                                          .file = __FILE__,
                                          .line = __LINE__,
                                          .function = "main",
                                          .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
                                          .timeout_ns = 30000000000ULL,
                                          .skip = false,
                                          .skip_reason = NULL},
                             .result = ASTHRA_TEST_PASS,
                             .start_time_ns = 0,
                             .end_time_ns = 0,
                             .duration_ns = 0,
                             .error_message = NULL,
                             .error_message_allocated = false,
                             .assertions_in_test = 0,
                             .global_stats = NULL};

    int passed = 0;
    int total = 3;

    printf("\n[1/3] Running C String Conversion Test...\n");
    if (test_string_c_string_conversion(&ctx) == ASTHRA_TEST_PASS) {
        printf("  ✓ PASSED\n");
        passed++;
    } else {
        printf("  ✗ FAILED\n");
    }

    printf("\n[2/3] Running FFI Ownership Transfer Test...\n");
    if (test_string_ffi_ownership_transfer(&ctx) == ASTHRA_TEST_PASS) {
        printf("  ✓ PASSED\n");
        passed++;
    } else {
        printf("  ✗ FAILED\n");
    }

    printf("\n[3/3] Running FFI Borrowed References Test...\n");
    if (test_string_ffi_borrowed_references(&ctx) == ASTHRA_TEST_PASS) {
        printf("  ✓ PASSED\n");
        passed++;
    } else {
        printf("  ✗ FAILED\n");
    }

    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Success Rate: %.1f%%\n", (double)passed / total * 100.0);

    if (passed == total) {
        printf("🎉 All tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed.\n");
        return 1;
    }
}
#endif // ASTHRA_TEST_LIBRARY_MODE
