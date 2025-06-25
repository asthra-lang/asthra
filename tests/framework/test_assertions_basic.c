/**
 * Asthra Programming Language
 * Test Framework - Basic Type Assertions Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of basic type assertion functions
 */

#include "test_assertions_basic.h"

// =============================================================================
// BASIC TYPE ASSERTION IMPLEMENTATIONS
// =============================================================================

bool asthra_test_assert_int(AsthraTestContext *context, int value, const char *message, ...) {
    return asthra_test_assert_bool(context, value != 0, message);
}

bool asthra_test_assert_long(AsthraTestContext *context, long value, const char *message, ...) {
    return asthra_test_assert_bool(context, value != 0, message);
}

bool asthra_test_assert_size(AsthraTestContext *context, size_t value, const char *message, ...) {
    return asthra_test_assert_bool(context, value != 0, message);
}

bool asthra_test_assert_string(AsthraTestContext *context, const char *value, const char *message,
                               ...) {
    return asthra_test_assert_bool(context, value != NULL && strlen(value) > 0, message);
}

bool asthra_test_assert_pointer(AsthraTestContext *context, void *value, const char *message, ...) {
    return asthra_test_assert_bool(context, value != NULL, message);
}

bool asthra_test_assert_generic(AsthraTestContext *context, void *value, const char *message, ...) {
    return asthra_test_assert_bool(context, value != NULL, message);
}

bool asthra_test_assert_not_null(AsthraTestContext *context, void *value, const char *message,
                                 ...) {
    if (!context)
        return false;

    context->assertions_in_test++;
    if (context->global_stats) {
        asthra_test_increment_stat(&context->global_stats->assertions_checked, 1);
    }

    if (value == NULL) {
        if (context->global_stats) {
            asthra_test_increment_stat(&context->global_stats->assertions_failed, 1);
        }

        if (message) {
            va_list args;
            va_start(args, message);
            char *error_msg = malloc(1024);
            if (error_msg) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
                vsnprintf(error_msg, 1024, message, args);
#pragma GCC diagnostic pop
                context->error_message = error_msg;
                context->error_message_allocated = true;
            }
            va_end(args);
        }
        return false;
    }
    return true;
}

bool asthra_test_assert_null(AsthraTestContext *context, void *value, const char *message, ...) {
    if (!context)
        return false;

    context->assertions_in_test++;
    if (context->global_stats) {
        asthra_test_increment_stat(&context->global_stats->assertions_checked, 1);
    }

    if (value != NULL) {
        if (context->global_stats) {
            asthra_test_increment_stat(&context->global_stats->assertions_failed, 1);
        }

        if (message) {
            va_list args;
            va_start(args, message);
            char *error_msg = malloc(1024);
            if (error_msg) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
                vsnprintf(error_msg, 1024, message, args);
#pragma GCC diagnostic pop
                context->error_message = error_msg;
                context->error_message_allocated = true;
            }
            va_end(args);
        }
        return false;
    }
    return true;
}
