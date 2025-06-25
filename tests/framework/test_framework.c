/**
 * Asthra Programming Language
 * Test Framework - Main Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test framework implementation file
 */

#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This file provides a minimal implementation to satisfy build dependencies
// The actual test framework functionality is implemented in the header files
// and other modular source files.

// Placeholder function to ensure the object file is not empty
void asthra_test_framework_init(void) {
    // This function is intentionally empty
    // The test framework is primarily header-based
}

// Test failure function implementation
AsthraTestResult asthra_test_fail(AsthraTestContext *context, const char *message) {
    if (context) {
        context->result = ASTHRA_TEST_FAIL;
        context->error_message = message;
    }
    if (message) {
        fprintf(stderr, "TEST FAILED: %s\n", message);
    }
    return ASTHRA_TEST_FAIL;
}

AsthraTestContext *asthra_test_create_context(AsthraTestMetadata *metadata) {
    AsthraTestContext *context = calloc(1, sizeof(AsthraTestContext));
    if (!context)
        return NULL;

    if (metadata) {
        context->metadata = *metadata;
    } else {
        // Create default metadata
        static AsthraTestMetadata default_metadata = {.name = "default_test",
                                                      .file = __FILE__,
                                                      .line = __LINE__,
                                                      .function = "default_test",
                                                      .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
                                                      .timeout_ns = 5000000000ULL,
                                                      .skip = false,
                                                      .skip_reason = NULL};
        context->metadata = default_metadata;
    }

    context->result = ASTHRA_TEST_PASS;
    context->error_message = NULL;
    context->assertions_in_test = 0;

    return context;
}

void asthra_test_destroy_context(AsthraTestContext *context) {
    if (context) {
        free(context);
    }
}

// Statistics functions are implemented in test_statistics.c
