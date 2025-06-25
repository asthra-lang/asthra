/**
 * Asthra Programming Language Runtime v1.2
 * Observability and Logging Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_common.h"

// =============================================================================
// OBSERVABILITY TESTS
// =============================================================================

int test_observability(void) {
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization failed");

    // Test logging
    asthra_log(ASTHRA_LOG_INFO, ASTHRA_LOG_CATEGORY_GENERAL, "Test log message: %s", "Hello");
    asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_MEMORY, "Debug message with number: %d", 42);

    // Test runtime statistics
    AsthraRuntimeStats stats = asthra_get_runtime_stats();
    printf("  Runtime stats - Allocations: %" PRIu64 ", Memory usage: %" PRIu64 " bytes\n",
           (uint64_t)stats.total_allocations, (uint64_t)stats.current_memory_usage);

    asthra_runtime_cleanup();
    TEST_PASS("Observability and logging");
}

// Test function declarations for external use
int test_observability(void);
