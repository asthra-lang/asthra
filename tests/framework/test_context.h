/**
 * Asthra Programming Language
 * Test Framework - Context Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test execution context management
 * Enhanced for Testing Framework Standardization Plan Phase 1
 */

#ifndef ASTHRA_TEST_CONTEXT_H
#define ASTHRA_TEST_CONTEXT_H

#include "test_statistics.h"
#include "test_types.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPES AND STRUCTURES
// =============================================================================

// Test execution context
typedef struct AsthraTestContext {
    AsthraTestMetadata metadata;
    AsthraTestResult result;
    uint64_t start_time_ns;
    uint64_t end_time_ns;
    uint64_t duration_ns;
    const char *error_message;    // Back to const char * for string literals
    bool error_message_allocated; // Track if error_message was dynamically allocated
    size_t assertions_in_test;
    AsthraTestStatistics *global_stats;
    void *user_data; // NEW: For adapter pattern support
} AsthraTestContext;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Test context management
AsthraTestContext *asthra_test_context_create(const AsthraTestMetadata *metadata,
                                              AsthraTestStatistics *global_stats);
void asthra_test_context_destroy(AsthraTestContext *context);
void asthra_test_context_start(AsthraTestContext *context);
void asthra_test_context_end(AsthraTestContext *context, AsthraTestResult result);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_CONTEXT_H
