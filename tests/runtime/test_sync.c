/**
 * Asthra Programming Language Runtime v1.2
 * Synchronization Primitives Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_common.h"

// =============================================================================
// SYNCHRONIZATION PRIMITIVES TESTS
// =============================================================================

int test_synchronization_primitives(void) {
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization failed");

    // Test mutex creation and operations
    AsthraMutex *mutex = asthra_mutex_create();
    TEST_ASSERT(mutex != NULL, "Mutex creation failed");

    asthra_mutex_lock(mutex);
    asthra_mutex_unlock(mutex);
    asthra_mutex_destroy(mutex);

    asthra_runtime_cleanup();
    TEST_PASS("Synchronization primitives");
}

// Test function declarations for external use
int test_synchronization_primitives(void);
