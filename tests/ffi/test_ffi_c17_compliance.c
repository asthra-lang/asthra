/**
 * Asthra Programming Language v1.2 Enhanced FFI Tests
 * C17 Compliance Testing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for C17 standard compliance including features, types, and functions.
 */

#include "../core/test_comprehensive.h"
#include "../runtime/ffi.h"
#include "../runtime/memory.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// C17 COMPLIANCE TESTS
// =============================================================================

// Test _Thread_local (C11/C17 feature) - must be at file scope
_Thread_local int thread_local_var = 100;

// Helper functions for compliance tests
static inline int inline_add(int a, int b) {
    return a + b;
}

void copy_array(int *restrict dest, const int *restrict src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

AsthraTestResult test_ffi_c17_compliance_features(AsthraV12TestContext *ctx) {
    // Test C17 standard compliance features

    // Test _Static_assert (C11/C17 feature)
    _Static_assert(sizeof(int) >= 4, "int must be at least 4 bytes");
    _Static_assert(sizeof(void *) >= sizeof(int), "pointer must be at least as large as int");

    // Test _Alignas and _Alignof (C11/C17 features)
    _Alignas(16) char aligned_buffer[64];

    if (!ASTHRA_TEST_ASSERT(&ctx->base, _Alignof(char[64]) >= 16,
                            "Buffer type should be aligned to at least 16 bytes")) {
        return ASTHRA_TEST_FAIL;
    }

// Test _Generic (C11/C17 feature)
#define TYPE_NAME(x)                                                                               \
    _Generic((x),                                                                                  \
        int: "int",                                                                                \
        float: "float",                                                                            \
        double: "double",                                                                          \
        char *: "string",                                                                          \
        default: "unknown")

    int test_int = 42;
    float test_float = 3.14f;
    double test_double = 2.718;
    char *test_string = "hello";

    const char *int_type = TYPE_NAME(test_int);
    const char *float_type = TYPE_NAME(test_float);
    const char *double_type = TYPE_NAME(test_double);
    const char *string_type = TYPE_NAME(test_string);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(int_type, "int") == 0,
                            "Generic should identify int type")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(float_type, "float") == 0,
                            "Generic should identify float type")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(double_type, "double") == 0,
                            "Generic should identify double type")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(string_type, "string") == 0,
                            "Generic should identify string type")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test _Thread_local (C11/C17 feature) - now using global variable

    if (!ASTHRA_TEST_ASSERT(&ctx->base, thread_local_var == 100,
                            "Thread local variable should be accessible")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ffi_c17_compliance_types(AsthraV12TestContext *ctx) {
    // Test C17 standard type compliance

    // Test exact-width integer types (stdint.h)
    int8_t i8 = -128;
    uint8_t u8 = 255;
    int16_t i16 = -32768;
    uint16_t u16 = 65535;
    int32_t i32 = -2147483648;
    uint32_t u32 = 4294967295U;
    int64_t i64 = -9223372036854775807LL - 1;
    uint64_t u64 = 18446744073709551615ULL;

    if (!ASTHRA_TEST_ASSERT(&ctx->base, sizeof(i8) == 1, "int8_t should be 1 byte")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, sizeof(u8) == 1, "uint8_t should be 1 byte")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, sizeof(i16) == 2, "int16_t should be 2 bytes")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, sizeof(u16) == 2, "uint16_t should be 2 bytes")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, sizeof(i32) == 4, "int32_t should be 4 bytes")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, sizeof(u32) == 4, "uint32_t should be 4 bytes")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, sizeof(i64) == 8, "int64_t should be 8 bytes")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, sizeof(u64) == 8, "uint64_t should be 8 bytes")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test value ranges
    if (!ASTHRA_TEST_ASSERT(&ctx->base, i8 == -128, "int8_t minimum value should be -128")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, u8 == 255, "uint8_t maximum value should be 255")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test pointer types
    intptr_t ptr_as_int = (intptr_t)&i32;
    void *int_as_ptr = (void *)ptr_as_int;

    if (!ASTHRA_TEST_ASSERT(&ctx->base, int_as_ptr == &i32,
                            "Pointer conversion should be reversible")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ffi_c17_compliance_functions(AsthraV12TestContext *ctx) {
    // Test C17 compliant function declarations and calls

    // Test inline functions (C99/C11/C17) - using global function

    int inline_result = inline_add(25, 17);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, inline_result == 42,
                            "Inline function should return 42, got %d", inline_result)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test restrict keyword (C99/C11/C17) - using global function

    int source[5] = {1, 2, 3, 4, 5};
    int destination[5];

    copy_array(destination, source, 5);

    for (int i = 0; i < 5; i++) {
        if (!ASTHRA_TEST_ASSERT(&ctx->base, destination[i] == source[i],
                                "Copied element %d should match source", i)) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test variable length arrays (VLA) - C99/C11/C17
    int vla_size = 5;
    int vla[vla_size];

    for (int i = 0; i < vla_size; i++) {
        vla[i] = i * i;
    }

    for (int i = 0; i < vla_size; i++) {
        if (!ASTHRA_TEST_ASSERT(&ctx->base, vla[i] == i * i, "VLA element %d should be %d, got %d",
                                i, i * i, vla[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test compound literals (C99/C11/C17)
    int *compound_array = (int[]){10, 20, 30, 40, 50};

    for (int i = 0; i < 5; i++) {
        int expected = (i + 1) * 10;
        if (!ASTHRA_TEST_ASSERT(&ctx->base, compound_array[i] == expected,
                                "Compound literal element %d should be %d, got %d", i, expected,
                                compound_array[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}
