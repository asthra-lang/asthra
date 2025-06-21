/**
 * Asthra Programming Language
 * Test Framework - Range Assertions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Range assertion functions for value bounds checking
 */

#ifndef ASTHRA_TEST_ASSERTIONS_RANGE_H
#define ASTHRA_TEST_ASSERTIONS_RANGE_H

#include "test_assertions_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// RANGE ASSERTIONS
// =============================================================================

/**
 * Assert that an integer value is within a specified range
 * @param context Test context
 * @param value Integer value to test
 * @param min Minimum allowed value (inclusive)
 * @param max Maximum allowed value (inclusive)
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if value is within range
 */
bool asthra_test_assert_int_range(AsthraTestContext *context, int value, int min, int max, const char *message, ...);

/**
 * Assert that a long value is within a specified range
 * @param context Test context
 * @param value Long value to test
 * @param min Minimum allowed value (inclusive)
 * @param max Maximum allowed value (inclusive)
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if value is within range
 */
bool asthra_test_assert_long_range(AsthraTestContext *context, long value, long min, long max, const char *message, ...);

/**
 * Assert that a size_t value is within a specified range
 * @param context Test context
 * @param value Size value to test
 * @param min Minimum allowed value (inclusive)
 * @param max Maximum allowed value (inclusive)
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if value is within range
 */
bool asthra_test_assert_size_range(AsthraTestContext *context, size_t value, size_t min, size_t max, const char *message, ...);

/**
 * Assert that a double value is within a specified range
 * @param context Test context
 * @param value Double value to test
 * @param min Minimum allowed value (inclusive)
 * @param max Maximum allowed value (inclusive)
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if value is within range
 */
bool asthra_test_assert_double_range(AsthraTestContext *context, double value, double min, double max, const char *message, ...);

/**
 * Assert that a generic value is within a specified range (pointer comparison)
 * @param context Test context
 * @param value Generic value to test
 * @param min Minimum allowed value (inclusive)
 * @param max Maximum allowed value (inclusive)
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if value is within range
 */
bool asthra_test_assert_generic_range(AsthraTestContext *context, void *value, void *min, void *max, const char *message, ...);

/**
 * Assert that an integer value is greater than a minimum value
 * @param context Test context
 * @param actual Actual integer value
 * @param min Minimum value (exclusive)
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if actual > min
 */
bool asthra_test_assert_int_gt(AsthraTestContext *context, int actual, int min, const char *message, ...);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_ASSERTIONS_RANGE_H 
