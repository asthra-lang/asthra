/**
 * Asthra Programming Language
 * Test Framework - Basic Type Assertions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Basic type assertion functions for primitives
 */

#ifndef ASTHRA_TEST_ASSERTIONS_BASIC_H
#define ASTHRA_TEST_ASSERTIONS_BASIC_H

#include "test_assertions_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BASIC TYPE ASSERTIONS
// =============================================================================

/**
 * Assert that an integer value is non-zero (truthy)
 * @param context Test context
 * @param value Integer value to test
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if value is non-zero
 */
bool asthra_test_assert_int(AsthraTestContext *context, int value, const char *message, ...);

/**
 * Assert that a long value is non-zero (truthy)
 * @param context Test context
 * @param value Long value to test
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if value is non-zero
 */
bool asthra_test_assert_long(AsthraTestContext *context, long value, const char *message, ...);

/**
 * Assert that a size_t value is non-zero (truthy)
 * @param context Test context
 * @param value Size value to test
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if value is non-zero
 */
bool asthra_test_assert_size(AsthraTestContext *context, size_t value, const char *message, ...);

/**
 * Assert that a string is non-null and non-empty
 * @param context Test context
 * @param value String value to test
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if string is valid and non-empty
 */
bool asthra_test_assert_string(AsthraTestContext *context, const char *value, const char *message,
                               ...);

/**
 * Assert that a pointer is non-null
 * @param context Test context
 * @param value Pointer value to test
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if pointer is non-null
 */
bool asthra_test_assert_pointer(AsthraTestContext *context, void *value, const char *message, ...);

/**
 * Assert that a generic pointer is non-null
 * @param context Test context
 * @param value Generic pointer value to test
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if pointer is non-null
 */
bool asthra_test_assert_generic(AsthraTestContext *context, void *value, const char *message, ...);

/**
 * Assert that a pointer is non-null (alias for compatibility)
 * @param context Test context
 * @param value Pointer value to test
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if pointer is non-null
 */
bool asthra_test_assert_not_null(AsthraTestContext *context, void *value, const char *message, ...);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_ASSERTIONS_BASIC_H
