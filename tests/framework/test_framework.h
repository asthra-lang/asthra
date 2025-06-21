/**
 * Asthra Programming Language
 * C17 Modernized Test Framework
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 5.1: Test Framework Modernization
 * - C17 _Static_assert for compile-time test validation
 * - _Generic for polymorphic test operations
 * - Atomic operations for thread-safe test statistics
 * - Enhanced test configuration with designated initializers
 */

#ifndef ASTHRA_TEST_FRAMEWORK_H
#define ASTHRA_TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include <time.h>
#include <assert.h>

// Include test types first to avoid conflicts
#include "test_types.h"
#include "test_statistics.h"
#include "test_context.h"
#include "test_suite.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// C17 STATIC ASSERTIONS FOR TEST FRAMEWORK VALIDATION
// =============================================================================

// Test framework assumptions validation
_Static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t must be at least 32-bit for test framework");
_Static_assert(sizeof(void*) >= 4, "Pointer size must be at least 32-bit for test framework");
_Static_assert(__STDC_VERSION__ >= 201710L, "C17 standard required for test framework modernization");
_Static_assert(sizeof(atomic_uint_fast64_t) >= sizeof(uint64_t), "Atomic types must support 64-bit counters");
_Static_assert(sizeof(clock_t) > 0, "clock_t must be available for timing measurements");

// Test result type validation
_Static_assert(sizeof(int) >= 4, "int must be at least 32 bits for test results");

// =============================================================================
// TEST FRAMEWORK TYPES AND STRUCTURES
// =============================================================================

// AsthraTestContext is defined in test_context.h

// Test function signature
typedef AsthraTestResult (*AsthraTestFunction)(AsthraTestContext *context);

// =============================================================================
// C17 ATOMIC OPERATIONS FOR THREAD-SAFE STATISTICS
// =============================================================================

// Atomic operations are defined in test_statistics.h

// =============================================================================
// C17 _GENERIC FOR POLYMORPHIC TEST OPERATIONS
// =============================================================================

// Generic test assertion dispatch
#define ASTHRA_TEST_ASSERT(context, condition, ...) \
    _Generic((condition), \
        bool: asthra_test_assert_bool, \
        int: asthra_test_assert_int, \
        long: asthra_test_assert_long, \
        size_t: asthra_test_assert_size, \
        const char*: asthra_test_assert_string, \
        char*: asthra_test_assert_string, \
        void*: asthra_test_assert_pointer, \
        default: asthra_test_assert_generic \
    )(context, condition, __VA_ARGS__)

// Generic test comparison dispatch
#define ASTHRA_TEST_ASSERT_EQ(context, actual, expected, ...) \
    _Generic((actual), \
        bool: asthra_test_assert_bool_eq, \
        int: asthra_test_assert_int_eq, \
        long: asthra_test_assert_long_eq, \
        size_t: asthra_test_assert_size_eq, \
        const char*: asthra_test_assert_string_eq, \
        char*: asthra_test_assert_string_eq, \
        void*: asthra_test_assert_pointer_eq, \
        default: asthra_test_assert_generic_eq \
    )(context, actual, expected, __VA_ARGS__)

// Generic test range checking dispatch
#define ASTHRA_TEST_ASSERT_RANGE(context, value, min, max, ...) \
    _Generic((value), \
        int: asthra_test_assert_int_range, \
        long: asthra_test_assert_long_range, \
        size_t: asthra_test_assert_size_range, \
        double: asthra_test_assert_double_range, \
        default: asthra_test_assert_generic_range \
    )(context, value, min, max, __VA_ARGS__)

// =============================================================================
// TEST FRAMEWORK FUNCTION DECLARATIONS
// =============================================================================

// Test statistics management
AsthraTestStatistics* asthra_test_statistics_create(void);
void asthra_test_statistics_destroy(AsthraTestStatistics *stats);
void asthra_test_statistics_reset(AsthraTestStatistics *stats);
void asthra_test_statistics_print(const AsthraTestStatistics *stats, bool json_format);

// Test suite configuration
AsthraTestSuiteConfig asthra_test_suite_config_default(void);
AsthraTestSuiteConfig asthra_test_suite_config_create(const char *name, const char *description);

// Test context management
AsthraTestContext* asthra_test_context_create(const AsthraTestMetadata *metadata,
                                             AsthraTestStatistics *global_stats);
void asthra_test_context_destroy(AsthraTestContext *context);
void asthra_test_context_start(AsthraTestContext *context);
void asthra_test_context_end(AsthraTestContext *context, AsthraTestResult result);

// Test execution
AsthraTestResult asthra_test_run_single(AsthraTestFunction test_func,
                                       const AsthraTestMetadata *metadata,
                                       AsthraTestStatistics *stats);
AsthraTestResult asthra_test_run_suite(AsthraTestFunction *tests,
                                      const AsthraTestMetadata *metadata_array,
                                      size_t test_count,
                                      const AsthraTestSuiteConfig *config);

// Type-specific assertion implementations for _Generic dispatch
bool asthra_test_assert_bool(AsthraTestContext *context, bool condition, const char *message, ...);
bool asthra_test_assert_int(AsthraTestContext *context, int value, const char *message, ...);
bool asthra_test_assert_long(AsthraTestContext *context, long value, const char *message, ...);
bool asthra_test_assert_size(AsthraTestContext *context, size_t value, const char *message, ...);
bool asthra_test_assert_string(AsthraTestContext *context, const char *value, const char *message, ...);
bool asthra_test_assert_pointer(AsthraTestContext *context, void *value, const char *message, ...);
bool asthra_test_assert_generic(AsthraTestContext *context, void *value, const char *message, ...);

// Type-specific equality assertions for _Generic dispatch
bool asthra_test_assert_bool_eq(AsthraTestContext *context, bool actual, bool expected, const char *message, ...);
bool asthra_test_assert_int_eq(AsthraTestContext *context, int actual, int expected, const char *message, ...);
bool asthra_test_assert_long_eq(AsthraTestContext *context, long actual, long expected, const char *message, ...);
bool asthra_test_assert_size_eq(AsthraTestContext *context, size_t actual, size_t expected, const char *message, ...);
bool asthra_test_assert_string_eq(AsthraTestContext *context, const char *actual, const char *expected, const char *message, ...);
bool asthra_test_assert_pointer_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...);
bool asthra_test_assert_generic_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...);

// Type-specific range assertions for _Generic dispatch
bool asthra_test_assert_int_range(AsthraTestContext *context, int value, int min, int max, const char *message, ...);
bool asthra_test_assert_long_range(AsthraTestContext *context, long value, long min, long max, const char *message, ...);
bool asthra_test_assert_size_range(AsthraTestContext *context, size_t value, size_t min, size_t max, const char *message, ...);
bool asthra_test_assert_double_range(AsthraTestContext *context, double value, double min, double max, const char *message, ...);
bool asthra_test_assert_generic_range(AsthraTestContext *context, void *value, void *min, void *max, const char *message, ...);

// Test failure function
AsthraTestResult asthra_test_fail(AsthraTestContext *context, const char *message);

// Test context utility functions
AsthraTestContext* asthra_test_create_context(AsthraTestMetadata* metadata);
void asthra_test_destroy_context(AsthraTestContext* context);

// Statistics functions are declared in test_statistics.h

// =============================================================================
// CONVENIENCE MACROS FOR TEST DEFINITION
// =============================================================================

// Test definition macro with C17 designated initializers
#define ASTHRA_TEST_DEFINE(test_name, test_severity) \
    static AsthraTestResult test_name(AsthraTestContext *context); \
    static const AsthraTestMetadata test_name##_metadata = { \
        .name = #test_name, \
        .file = __FILE__, \
        .line = __LINE__, \
        .function = #test_name, \
        .severity = test_severity, \
        .timeout_ns = 30000000000ULL, /* 30 seconds */ \
        .skip = false, \
        .skip_reason = NULL \
    }; \
    static AsthraTestResult test_name(AsthraTestContext *context)

// Test skip macro
#define ASTHRA_TEST_SKIP(context, reason) \
    do { \
        (context)->result = ASTHRA_TEST_SKIP; \
        (context)->error_message = reason; \
        return ASTHRA_TEST_SKIP; \
    } while (0)

// Test failure macro with safer variadic handling
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#define ASTHRA_TEST_FAIL(context, message, ...) \
    do { \
        char *error_msg = malloc(1024); \
        if (error_msg) { \
            snprintf(error_msg, 1024, message, ##__VA_ARGS__); \
            (context)->error_message = error_msg; \
            (context)->error_message_allocated = true; \
        } \
        (context)->result = ASTHRA_TEST_FAIL; \
        return ASTHRA_TEST_FAIL; \
    } while (0)
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

// Test success macro
#define ASTHRA_TEST_PASS(context) \
    do { \
        (context)->result = ASTHRA_TEST_PASS; \
        return ASTHRA_TEST_PASS; \
    } while (0)

// =============================================================================
// CONVENIENCE ASSERTION MACROS
// =============================================================================

// Convenience macros for common assertion patterns
#define ASTHRA_TEST_ASSERT_TRUE(context, condition, ...) \
    do { \
        if (!asthra_test_assert_bool_eq(context, (condition), true, __VA_ARGS__)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

#define ASTHRA_TEST_ASSERT_FALSE(context, condition, ...) \
    do { \
        if (!asthra_test_assert_bool_eq(context, (condition), false, __VA_ARGS__)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

#define ASTHRA_TEST_ASSERT_NOT_NULL(context, pointer, ...) \
    do { \
        if (!asthra_test_assert_pointer(context, (void*)(pointer), __VA_ARGS__)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

#define ASTHRA_TEST_ASSERT_NULL(context, pointer, ...) \
    do { \
        if (!asthra_test_assert_pointer_eq(context, (void*)(pointer), NULL, __VA_ARGS__)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

#define ASTHRA_TEST_ASSERT_GT(context, actual, expected, ...) \
    do { \
        if ((actual) <= (expected)) { \
            char *error_msg = malloc(1024); \
            if (error_msg) { \
                snprintf(error_msg, 1024, __VA_ARGS__); \
                (context)->error_message = error_msg; \
                (context)->error_message_allocated = true; \
            } \
            (context)->result = ASTHRA_TEST_FAIL; \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

#define ASTHRA_TEST_ASSERT_LT(context, actual, expected, ...) \
    do { \
        if ((actual) >= (expected)) { \
            char *error_msg = malloc(1024); \
            if (error_msg) { \
                snprintf(error_msg, 1024, __VA_ARGS__); \
                (context)->error_message = error_msg; \
                (context)->error_message_allocated = true; \
            } \
            (context)->result = ASTHRA_TEST_FAIL; \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

#define ASTHRA_TEST_ASSERT_GE(context, actual, expected, ...) \
    do { \
        if ((actual) < (expected)) { \
            char *error_msg = malloc(1024); \
            if (error_msg) { \
                snprintf(error_msg, 1024, __VA_ARGS__); \
                (context)->error_message = error_msg; \
                (context)->error_message_allocated = true; \
            } \
            (context)->result = ASTHRA_TEST_FAIL; \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

#define ASTHRA_TEST_ASSERT_LE(context, actual, expected, ...) \
    do { \
        if ((actual) > (expected)) { \
            char *error_msg = malloc(1024); \
            if (error_msg) { \
                snprintf(error_msg, 1024, __VA_ARGS__); \
                (context)->error_message = error_msg; \
                (context)->error_message_allocated = true; \
            } \
            (context)->result = ASTHRA_TEST_FAIL; \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

#define ASTHRA_TEST_ASSERT_STR_EQ(context, actual, expected, ...) \
    do { \
        if (!asthra_test_assert_string_eq(context, (actual), (expected), __VA_ARGS__)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

#define ASTHRA_TEST_ASSERT_NE(context, actual, expected, ...) \
    do { \
        if ((actual) == (expected)) { \
            char *error_msg = malloc(1024); \
            if (error_msg) { \
                snprintf(error_msg, 1024, __VA_ARGS__); \
                (context)->error_message = error_msg; \
                (context)->error_message_allocated = true; \
            } \
            (context)->result = ASTHRA_TEST_FAIL; \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

// =============================================================================
// TIMING UTILITIES
// =============================================================================

// Timing utilities are defined in test_types.h and test_statistics.h

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_FRAMEWORK_H
