/**
 * Asthra Programming Language Compiler
 * Static Analysis Integration Test Suite - Buffer Operations Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 5.2: Buffer Operations Tests
 * - Test buffer copy operations
 * - Validate buffer append functionality
 * - Test buffer equality comparisons
 */

#include "test_static_analysis_common.h"

// =============================================================================
// BUFFER OPERATIONS TESTS
// =============================================================================

AsthraTestResult test_buffer_operations(AsthraTestContext *context) {
    asthra_test_context_start(context);

    // Test buffer copy
    const char *src = "Hello, Buffer!";
    char dest[32];
    size_t src_size = strlen(src);

    size_t copied = asthra_buffer_copy(dest, sizeof(dest), src, src_size);

    if (!asthra_test_assert_size_eq(context, copied, src_size, "Should copy all source bytes")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, memory_equal(dest, src, src_size),
                                 "Copied buffer should match source")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test buffer append (binary operation, not string operation)
    char buffer[32];
    const char *part1 = "Hello, ";
    const char *part2 = "World!";
    size_t part1_size = strlen(part1);
    size_t part2_size = strlen(part2);

    // First copy part1 to buffer
    size_t copied1 = asthra_buffer_copy(buffer, sizeof(buffer), part1, part1_size);

    // Then append part2
    size_t appended = asthra_buffer_append(buffer, sizeof(buffer), copied1, part2, part2_size);

    if (!asthra_test_assert_size_eq(context, appended, part2_size,
                                    "Should append all source bytes")) {
        return ASTHRA_TEST_FAIL;
    }

    // Check the combined buffer content (binary comparison)
    char expected_combined[32];
    memcpy(expected_combined, part1, part1_size);
    memcpy(expected_combined + part1_size, part2, part2_size);
    size_t total_size = part1_size + part2_size;

    if (!asthra_test_assert_bool(context, memory_equal(buffer, expected_combined, total_size),
                                 "Appended buffer should match expected content")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test buffer equality
    const char *buf1 = "Test Buffer";
    const char *buf2 = "Test Buffer";
    const char *buf3 = "Different";

    bool equal1 = asthra_buffer_equal(buf1, strlen(buf1), buf2, strlen(buf2));
    bool equal2 = asthra_buffer_equal(buf1, strlen(buf1), buf3, strlen(buf3));

    if (!asthra_test_assert_bool(context, equal1, "Identical buffers should be equal")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, !equal2, "Different buffers should not be equal")) {
        return ASTHRA_TEST_FAIL;
    }

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}
