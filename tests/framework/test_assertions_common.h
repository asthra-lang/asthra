/**
 * Asthra Programming Language
 * Test Framework - Common Assertions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core assertion function and shared utilities
 */

#ifndef ASTHRA_TEST_ASSERTIONS_COMMON_H
#define ASTHRA_TEST_ASSERTIONS_COMMON_H

#include "test_framework.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CORE ASSERTION FUNCTION
// =============================================================================

/**
 * Core assertion function that all other assertions delegate to
 * @param context Test context for tracking assertions and errors
 * @param condition The condition to assert
 * @param message Optional error message format string
 * @param ... Variable arguments for message formatting
 * @return true if assertion passes, false otherwise
 */
bool asthra_test_assert_bool(AsthraTestContext *context, bool condition, const char *message, ...);

/**
 * Logging function for test output
 * @param context Test context
 * @param message Message format string
 * @param ... Variable arguments for message formatting
 */
void asthra_test_log(AsthraTestContext *context, const char *message, ...);

// =============================================================================
// SHARED ERROR MESSAGE FORMATTING MACROS
// =============================================================================

/**
 * Macro to format error messages with expected vs actual values
 */
#define FORMAT_ERROR_MESSAGE(context, message, args, format_str, ...)                              \
    do {                                                                                           \
        if (message) {                                                                             \
            va_list args_copy;                                                                     \
            va_start(args_copy, message);                                                          \
            char *error_msg = malloc(1024);                                                        \
            if (error_msg) {                                                                       \
                char formatted_msg[512];                                                           \
                _Pragma("GCC diagnostic push")                                                     \
                    _Pragma("GCC diagnostic ignored \"-Wformat-nonliteral\"")                      \
                        vsnprintf(formatted_msg, sizeof(formatted_msg), message, args_copy);       \
                _Pragma("GCC diagnostic pop")                                                      \
                    snprintf(error_msg, 1024, format_str, formatted_msg, __VA_ARGS__);             \
                context->error_message = error_msg;                                                \
                context->error_message_allocated = true;                                           \
            }                                                                                      \
            va_end(args_copy);                                                                     \
        }                                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_ASSERTIONS_COMMON_H
