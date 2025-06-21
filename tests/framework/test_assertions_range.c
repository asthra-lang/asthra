/**
 * Asthra Programming Language
 * Test Framework - Range Assertions Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Range assertion functions implementation for value bounds checking
 */

#include "test_assertions_range.h"

// =============================================================================
// RANGE ASSERTIONS IMPLEMENTATION
// =============================================================================

bool asthra_test_assert_int_range(AsthraTestContext *context, int value, int min, int max, const char *message, ...) {
    bool result = (value >= min && value <= max);
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
            snprintf(error_msg, 1024, "%s (value: %d, range: [%d, %d])", formatted_msg, value, min, max);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_long_range(AsthraTestContext *context, long value, long min, long max, const char *message, ...) {
    bool result = (value >= min && value <= max);
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
            snprintf(error_msg, 1024, "%s (value: %ld, range: [%ld, %ld])", formatted_msg, value, min, max);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_size_range(AsthraTestContext *context, size_t value, size_t min, size_t max, const char *message, ...) {
    bool result = (value >= min && value <= max);
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
            snprintf(error_msg, 1024, "%s (value: %zu, range: [%zu, %zu])", formatted_msg, value, min, max);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_double_range(AsthraTestContext *context, double value, double min, double max, const char *message, ...) {
    bool result = (value >= min && value <= max);
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
            snprintf(error_msg, 1024, "%s (value: %f, range: [%f, %f])", formatted_msg, value, min, max);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_generic_range(AsthraTestContext *context, void *value, void *min, void *max, const char *message, ...) {
    bool result = (value >= min && value <= max);
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
            snprintf(error_msg, 1024, "%s (value: %p, range: [%p, %p])", formatted_msg, value, min, max);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
}

bool asthra_test_assert_int_gt(AsthraTestContext *context, int actual, int min, const char *message, ...) {
    bool result = (actual > min);
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
            snprintf(error_msg, 1024, "%s (actual: %d, minimum: %d)", formatted_msg, actual, min);
            context->error_message = error_msg;
            context->error_message_allocated = true;
        }
        va_end(args);
    }
    return asthra_test_assert_bool(context, result, NULL);
} 
