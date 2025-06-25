/**
 * Asthra Programming Language Runtime v1.2
 * Runtime Initialization Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_common.h"

// =============================================================================
// RUNTIME INITIALIZATION TESTS
// =============================================================================

int test_runtime_initialization(void) {
    // Test with default configuration
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization with default config failed");

    // Test runtime information
    const char *version = asthra_runtime_version();
    TEST_ASSERT(version != NULL, "Runtime version should not be NULL");
    TEST_ASSERT(strcmp(version, "1.2.0") == 0, "Runtime version should be 1.2.0");

    const char *build_info = asthra_runtime_build_info();
    TEST_ASSERT(build_info != NULL, "Build info should not be NULL");

    asthra_runtime_cleanup();
    TEST_PASS("Runtime initialization and cleanup");
}

int test_runtime_custom_config(void) {
    AsthraGCConfig config = {.initial_heap_size = 2 * 1024 * 1024, // 2MB
                             .max_heap_size = 32 * 1024 * 1024,    // 32MB
                             .gc_threshold = 0.75,
                             .conservative_mode = true,
                             .concurrent_gc = false};

    int result = asthra_runtime_init(&config);
    TEST_ASSERT(result == 0, "Runtime initialization with custom config failed");

    asthra_runtime_cleanup();
    TEST_PASS("Runtime initialization with custom configuration");
}

// Test function declarations for external use
int test_runtime_initialization(void);
int test_runtime_custom_config(void);
