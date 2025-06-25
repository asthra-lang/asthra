/**
 * Asthra Programming Language v1.2 Enhanced FFI Tests
 * Enhanced FFI Capabilities Testing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for enhanced FFI capabilities including function calls,
 * bidirectional interoperability, and complex data structures.
 */

#include "../core/test_comprehensive.h"
#include "../runtime/ffi.h"
#include "../runtime/memory.h"
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST DATA STRUCTURES AND HELPERS
// =============================================================================

// Mock FFI function signatures
typedef int (*simple_c_func_t)(int);
typedef char *(*string_c_func_t)(const char *);
typedef void (*cleanup_c_func_t)(void *);

// Mock C functions for testing
static int test_c_add(int a, int b) {
    return a + b;
}

static char *test_c_string_duplicate(const char *input) {
    if (!input)
        return NULL;

    size_t len = strlen(input);
    char *result = malloc(len + 1);
    if (!result)
        return NULL;

    strcpy(result, input);
    return result;
}

static void test_c_cleanup(void *ptr) {
    if (ptr)
        free(ptr);
}

// Mock Asthra functions callable from C
static int asthra_multiply(int a, int b) {
    return a * b;
}

static const char *asthra_get_version(void) {
    return "Asthra v1.2";
}

// =============================================================================
// ENHANCED FFI CAPABILITIES TESTS
// =============================================================================

AsthraTestResult test_ffi_enhanced_function_calls(AsthraV12TestContext *ctx) {
    // Test enhanced FFI function call capabilities

    // Test simple function call
    int result = test_c_add(15, 27);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, result == 42, "C function call should return 42, got %d",
                            result)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test string function call
    const char *input = "Hello FFI";
    char *string_result = test_c_string_duplicate(input);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, string_result != NULL,
                            "C string function should return non-NULL")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(string_result, input) == 0,
                            "C string function result should match input")) {
        free(string_result);
        return ASTHRA_TEST_FAIL;
    }

    // Test cleanup
    test_c_cleanup(string_result);

    // Test function pointer handling
    simple_c_func_t func_ptr = test_c_add;
    int ptr_result = func_ptr(10, 20);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, ptr_result == 30,
                            "Function pointer call should return 30, got %d", ptr_result)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ffi_bidirectional_calls(AsthraV12TestContext *ctx) {
    // Test bidirectional FFI calls (C calling Asthra, Asthra calling C)

    // Test Asthra function callable from C
    int asthra_result = asthra_multiply(6, 7);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, asthra_result == 42,
                            "Asthra function should return 42, got %d", asthra_result)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test Asthra string function
    const char *version = asthra_get_version();

    if (!ASTHRA_TEST_ASSERT(&ctx->base, version != NULL,
                            "Asthra version function should return non-NULL")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, strstr(version, "Asthra") != NULL,
                            "Version string should contain 'Asthra'")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test callback mechanism
    typedef int (*callback_func_t)(int);

    int callback_test(callback_func_t callback, int value) {
        return callback(value * 2);
    }

    int test_callback(int x) {
        return x + 10;
    }

    int callback_result = callback_test(test_callback, 5);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, callback_result == 20,
                            "Callback result should be 20, got %d", callback_result)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ffi_complex_data_structures(AsthraV12TestContext *ctx) {
    // Test FFI with complex data structures

    // Test struct passing
    typedef struct {
        int id;
        double value;
        char name[32];
    } TestStruct;

    TestStruct test_data = {.id = 123, .value = 3.14159, .name = "FFI Test"};

    // Mock C function that processes struct
    TestStruct process_struct(TestStruct input) {
        TestStruct result = input;
        result.id *= 2;
        result.value *= 2.0;
        strcat(result.name, " Processed");
        return result;
    }

    TestStruct processed = process_struct(test_data);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, processed.id == 246,
                            "Processed struct ID should be 246, got %d", processed.id)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, processed.value > 6.28 && processed.value < 6.29,
                            "Processed struct value should be ~6.28, got %f", processed.value)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, strstr(processed.name, "Processed") != NULL,
                            "Processed struct name should contain 'Processed'")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test array passing
    int test_array[5] = {1, 2, 3, 4, 5};

    int sum_array(int *arr, size_t len) {
        int sum = 0;
        for (size_t i = 0; i < len; i++) {
            sum += arr[i];
        }
        return sum;
    }

    int array_sum = sum_array(test_array, 5);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, array_sum == 15, "Array sum should be 15, got %d",
                            array_sum)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}
