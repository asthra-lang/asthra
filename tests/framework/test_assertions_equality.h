/**
 * Asthra Programming Language
 * Test Framework - Equality Assertions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Equality assertion functions with detailed error messages
 */

#ifndef ASTHRA_TEST_ASSERTIONS_EQUALITY_H
#define ASTHRA_TEST_ASSERTIONS_EQUALITY_H

#include "test_assertions_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// EQUALITY ASSERTIONS
// =============================================================================

/**
 * Assert that two boolean values are equal
 * @param context Test context
 * @param actual Actual boolean value
 * @param expected Expected boolean value
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if values are equal
 */
bool asthra_test_assert_bool_eq(AsthraTestContext *context, bool actual, bool expected, const char *message, ...);

/**
 * Assert that two integer values are equal
 * @param context Test context
 * @param actual Actual integer value
 * @param expected Expected integer value
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if values are equal
 */
bool asthra_test_assert_int_eq(AsthraTestContext *context, int actual, int expected, const char *message, ...);

/**
 * Assert that two long values are equal
 * @param context Test context
 * @param actual Actual long value
 * @param expected Expected long value
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if values are equal
 */
bool asthra_test_assert_long_eq(AsthraTestContext *context, long actual, long expected, const char *message, ...);

/**
 * Assert that two size_t values are equal
 * @param context Test context
 * @param actual Actual size value
 * @param expected Expected size value
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if values are equal
 */
bool asthra_test_assert_size_eq(AsthraTestContext *context, size_t actual, size_t expected, const char *message, ...);

/**
 * Assert that two strings are equal
 * @param context Test context
 * @param actual Actual string value
 * @param expected Expected string value
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if strings are equal
 */
bool asthra_test_assert_string_eq(AsthraTestContext *context, const char *actual, const char *expected, const char *message, ...);

/**
 * Assert that two pointers are equal
 * @param context Test context
 * @param actual Actual pointer value
 * @param expected Expected pointer value
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if pointers are equal
 */
bool asthra_test_assert_pointer_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...);

/**
 * Assert that two generic pointers are equal
 * @param context Test context
 * @param actual Actual generic pointer value
 * @param expected Expected generic pointer value
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if pointers are equal
 */
bool asthra_test_assert_generic_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...);

/**
 * Assert that two strings are equal (alias for compatibility)
 * @param context Test context
 * @param actual Actual string value
 * @param expected Expected string value
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if strings are equal
 */
bool asthra_test_assert_str_eq(AsthraTestContext *context, const char *actual, const char *expected, const char *message, ...);

/**
 * Assert that two pointers are equal (alias for compatibility)
 * @param context Test context
 * @param actual Actual pointer value
 * @param expected Expected pointer value
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if pointers are equal
 */
bool asthra_test_assert_ptr_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...);

/**
 * Assert that two pointers are not equal
 * @param context Test context
 * @param actual Actual pointer value
 * @param expected Expected pointer value (to be different)
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if pointers are not equal
 */
bool asthra_test_assert_ptr_ne(AsthraTestContext *context, void *actual, void *expected, const char *message, ...);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_ASSERTIONS_EQUALITY_H 
