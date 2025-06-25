/**
 * Asthra Programming Language
 * Test Framework - Common Assertions Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core assertion function and shared utilities implementation
 */

#include "test_assertions_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CORE ASSERTION FUNCTION IMPLEMENTATION
// =============================================================================

bool asthra_test_assert_bool(AsthraTestContext *context, bool condition, const char *message, ...) {
    if (!context)
        return false;

    context->assertions_in_test++;
    if (context->global_stats) {
        asthra_test_increment_stat(&context->global_stats->assertions_checked, 1);
    }

    if (!condition) {
        if (context->global_stats) {
            asthra_test_increment_stat(&context->global_stats->assertions_failed, 1);
        }

        if (message) {
            va_list args;
            va_start(args, message);
            char *error_msg = malloc(1024);
            if (error_msg) {
// Suppress format-nonliteral warning: message parameter is intentionally variable
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

// =============================================================================
// LOGGING UTILITIES
// =============================================================================

void asthra_test_log(AsthraTestContext *context, const char *message, ...) {
    if (!context || !message)
        return;

    va_list args;
    va_start(args, message);

    // Print to stdout with test context prefix
    printf("[TEST] ");
// Suppress format-nonliteral warning: message parameter is intentionally variable
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    vprintf(message, args);
#pragma GCC diagnostic pop
    printf("\n");

    va_end(args);
}
