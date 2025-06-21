/**
 * Asthra Programming Language
 * Test Framework - Equality Assertions Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of equality assertion functions with detailed error reporting
 */

#include "test_assertions_equality.h"

// =============================================================================
// EQUALITY ASSERTION IMPLEMENTATIONS
// =============================================================================

bool asthra_test_assert_bool_eq(AsthraTestContext *context, bool actual, bool expected, const char *message, ...) {
    bool result = (actual == expected);
    if (!result && message) {
        va_list args;
        va_start(args, message);
        char *error_msg = malloc(1024);
        if (error_msg) {
            char formatted_msg[512];
            // Suppress format-nonliteral warning: message parameter is intentionally variable
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-nonliteral"
            vsnprintf(formatted_msg, sizeof(formatted_msg), message, args);
            #pragma GCC diagnostic pop
            snprintf(error_msg, 1024, "%s (expected: %s, actual: %s)",
                    formatted_msg, expected ? "true" : "false", actual ? "true" : "false");
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_int_eq(AsthraTestContext *context, int actual, int expected, const char *message, ...) {
    bool result = (actual == expected);
    if (!result && message) {
        va_list args;
        va_start(args, message);
        char *error_msg = malloc(1024);
        if (error_msg) {
            char formatted_msg[512];
            // Suppress format-nonliteral warning: message parameter is intentionally variable
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-nonliteral"
            vsnprintf(formatted_msg, sizeof(formatted_msg), message, args);
            #pragma GCC diagnostic pop
            snprintf(error_msg, 1024, "%s (expected: %d, actual: %d)", formatted_msg, expected, actual);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_long_eq(AsthraTestContext *context, long actual, long expected, const char *message, ...) {
    bool result = (actual == expected);
    if (!result && message) {
        va_list args;
        va_start(args, message);
        char *error_msg = malloc(1024);
        if (error_msg) {
            char formatted_msg[512];
            // Suppress format-nonliteral warning: message parameter is intentionally variable
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-nonliteral"
            vsnprintf(formatted_msg, sizeof(formatted_msg), message, args);
            #pragma GCC diagnostic pop
            snprintf(error_msg, 1024, "%s (expected: %ld, actual: %ld)", formatted_msg, expected, actual);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_size_eq(AsthraTestContext *context, size_t actual, size_t expected, const char *message, ...) {
    bool result = (actual == expected);
    if (!result && message) {
        va_list args;
        va_start(args, message);
        char *error_msg = malloc(1024);
        if (error_msg) {
            char formatted_msg[512];
            // Suppress format-nonliteral warning: message parameter is intentionally variable
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-nonliteral"
            vsnprintf(formatted_msg, sizeof(formatted_msg), message, args);
            #pragma GCC diagnostic pop
            snprintf(error_msg, 1024, "%s (expected: %zu, actual: %zu)", formatted_msg, expected, actual);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_string_eq(AsthraTestContext *context, const char *actual, const char *expected, const char *message, ...) {
    bool result = (actual && expected && strcmp(actual, expected) == 0);
    if (!result && message) {
        va_list args;
        va_start(args, message);
        char *error_msg = malloc(1024);
        if (error_msg) {
            char formatted_msg[512];
            // Suppress format-nonliteral warning: message parameter is intentionally variable
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-nonliteral"
            vsnprintf(formatted_msg, sizeof(formatted_msg), message, args);
            #pragma GCC diagnostic pop
            snprintf(error_msg, 1024, "%s (expected: \"%s\", actual: \"%s\")",
                    formatted_msg, expected ? expected : "NULL", actual ? actual : "NULL");
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_pointer_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...) {
    bool result = (actual == expected);
    if (!result && message) {
        va_list args;
        va_start(args, message);
        char *error_msg = malloc(1024);
        if (error_msg) {
            char formatted_msg[512];
            // Suppress format-nonliteral warning: message parameter is intentionally variable
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-nonliteral"
            vsnprintf(formatted_msg, sizeof(formatted_msg), message, args);
            #pragma GCC diagnostic pop
            snprintf(error_msg, 1024, "%s (expected: %p, actual: %p)", formatted_msg, expected, actual);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_generic_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...) {
    return asthra_test_assert_pointer_eq(context, actual, expected, message);
}

// =============================================================================
// CONVENIENCE ALIASES
// =============================================================================

bool asthra_test_assert_str_eq(AsthraTestContext *context, const char *actual, const char *expected, const char *message, ...) {
    return asthra_test_assert_string_eq(context, actual, expected, message);
}

bool asthra_test_assert_ptr_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...) {
    return asthra_test_assert_pointer_eq(context, actual, expected, message);
}

bool asthra_test_assert_ptr_ne(AsthraTestContext *context, void *actual, void *expected, const char *message, ...) {
    bool result = (actual != expected);
    if (!result && message) {
        va_list args;
        va_start(args, message);
        char *error_msg = malloc(1024);
        if (error_msg) {
            char formatted_msg[512];
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-nonliteral"
            vsnprintf(formatted_msg, sizeof(formatted_msg), message, args);
            #pragma GCC diagnostic pop
            snprintf(error_msg, 1024, "%s (expected different pointers, but both are %p)", formatted_msg, actual);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
} 
