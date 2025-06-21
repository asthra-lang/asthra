/**
 * Asthra Programming Language
 * Test Framework - Context Management Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test execution context management
 */

#include "test_context.h"
#include <stdlib.h>
#include <stdint.h>

// =============================================================================
// TEST CONTEXT MANAGEMENT
// =============================================================================

AsthraTestContext* asthra_test_context_create(const AsthraTestMetadata *metadata,
                                             AsthraTestStatistics *global_stats) {
    if (!metadata) return NULL;

    AsthraTestContext *context = calloc(1, sizeof(AsthraTestContext));
    if (!context) return NULL;

    // C17 designated initializer for context
    *context = (AsthraTestContext) {
        .metadata = *metadata,
        .result = ASTHRA_TEST_PASS,
        .start_time_ns = 0,
        .end_time_ns = 0,
        .duration_ns = 0,
        .error_message = NULL,
        .error_message_allocated = false,
        .assertions_in_test = 0,
        .global_stats = global_stats,
        .user_data = NULL
    };

    return context;
}

void asthra_test_context_destroy(AsthraTestContext *context) {
    if (context) {
        // Free error message if it was dynamically allocated
        if (context->error_message && context->error_message_allocated) {
            // Safe cast: we know it's actually a mutable string if allocated
            char *mutable_error_msg = (char *)(uintptr_t)context->error_message;
            free(mutable_error_msg);
        }
        free(context);
    }
}

void asthra_test_context_start(AsthraTestContext *context) {
    if (!context) return;

    context->start_time_ns = asthra_test_get_time_ns();
    context->assertions_in_test = 0;

    if (context->global_stats) {
        asthra_test_increment_stat(&context->global_stats->tests_run, 1);
    }
}

void asthra_test_context_end(AsthraTestContext *context, AsthraTestResult result) {
    if (!context) return;

    context->end_time_ns = asthra_test_get_time_ns();
    context->duration_ns = context->end_time_ns - context->start_time_ns;
    context->result = result;

    if (context->global_stats) {
        // Update statistics based on result
        switch (result) {
            case ASTHRA_TEST_PASS:
                asthra_test_increment_stat(&context->global_stats->tests_passed, 1);
                break;
            case ASTHRA_TEST_FAIL:
                asthra_test_increment_stat(&context->global_stats->tests_failed, 1);
                break;
            case ASTHRA_TEST_SKIP:
                asthra_test_increment_stat(&context->global_stats->tests_skipped, 1);
                break;
            case ASTHRA_TEST_ERROR:
                asthra_test_increment_stat(&context->global_stats->tests_error, 1);
                break;
            case ASTHRA_TEST_TIMEOUT:
                asthra_test_increment_stat(&context->global_stats->tests_timeout, 1);
                break;
            default:
                break;
        }

        // Update timing statistics
        asthra_test_increment_stat(&context->global_stats->total_duration_ns, context->duration_ns);

        // Update max duration
        uint64_t current_max = asthra_test_get_stat(&context->global_stats->max_duration_ns);
        if (context->duration_ns > current_max) {
            asthra_test_compare_and_swap_stat(&context->global_stats->max_duration_ns,
                                            &current_max, context->duration_ns);
        }

        // Update min duration
        uint64_t current_min = asthra_test_get_stat(&context->global_stats->min_duration_ns);
        if (context->duration_ns < current_min) {
            asthra_test_compare_and_swap_stat(&context->global_stats->min_duration_ns,
                                            &current_min, context->duration_ns);
        }
    }
}
