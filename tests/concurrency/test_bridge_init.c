/**
 * Asthra Enhanced Concurrency Bridge Test Suite - Initialization Tests
 * Tests for bridge initialization and cleanup functionality
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_test_common.h"

// =============================================================================
// INITIALIZATION TESTS
// =============================================================================

bool test_bridge_initialization(void) {
    // Test successful initialization
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");

    // Test double initialization (should succeed)
    result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Double initialization should succeed");

    // Cleanup
    Asthra_concurrency_bridge_cleanup();

    return true;
}

bool test_bridge_cleanup(void) {
    // Initialize bridge
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");

    // Cleanup should not crash
    Asthra_concurrency_bridge_cleanup();

    // Multiple cleanups should be safe
    Asthra_concurrency_bridge_cleanup();

    return true;
}
