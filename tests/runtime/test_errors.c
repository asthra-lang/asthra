/**
 * Asthra Programming Language Runtime v1.2
 * Error Handling Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_common.h"

// =============================================================================
// ERROR HANDLING TESTS
// =============================================================================

int test_error_handling(void) {
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization failed");

    // Clear any existing errors
    asthra_clear_error();

    // Test setting an error
    asthra_set_error(ASTHRA_ERROR_INVALID_ARGUMENT, "Test error message", __FILE__, __LINE__,
                     __func__);

    // Test getting the error
    AsthraError error = asthra_get_last_error();
    TEST_ASSERT(error.code == ASTHRA_ERROR_INVALID_ARGUMENT, "Error code should match");
    TEST_ASSERT(strcmp(error.message, "Test error message") == 0, "Error message should match");
    TEST_ASSERT(error.line == __LINE__ - 6, "Error line should match");

    // Test clearing the error
    asthra_clear_error();
    error = asthra_get_last_error();
    TEST_ASSERT(error.code == ASTHRA_ERROR_NONE, "Error should be cleared");

    asthra_runtime_cleanup();
    TEST_PASS("Error handling");
}

// Test function declarations for external use
int test_error_handling(void);
