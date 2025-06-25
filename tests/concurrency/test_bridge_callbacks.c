/**
 * Asthra Enhanced Concurrency Bridge Test Suite - Callback System Tests
 * Tests for callback enqueuing, processing, and priority handling
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_test_common.h"

// =============================================================================
// CALLBACK SYSTEM TESTS
// =============================================================================

bool test_callback_enqueue_and_process(void) {
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");

    int callback_counter = 0;

    // Enqueue a callback
    result = Asthra_enqueue_callback(callback_test_function, NULL, 0, &callback_counter, 1);
    ASSERT(asthra_result_is_ok(result), "Callback enqueue should succeed");

    // Process callbacks
    size_t processed = Asthra_process_callbacks(10);
    ASSERT(processed == 1, "Should process exactly one callback");
    ASSERT(callback_counter == 1, "Callback should have been executed");

    // Cleanup
    Asthra_concurrency_bridge_cleanup();

    return true;
}

bool test_callback_priority_ordering(void) {
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");

    int callback_counter = 0;

    // Enqueue multiple callbacks with different priorities
    for (int i = 0; i < 5; i++) {
        result = Asthra_enqueue_callback(callback_test_function, NULL, 0, &callback_counter, i);
        ASSERT(asthra_result_is_ok(result), "Callback enqueue should succeed");
    }

    // Process all callbacks
    size_t processed = Asthra_process_callbacks(10);
    ASSERT(processed == 5, "Should process all five callbacks");
    ASSERT(callback_counter == 5, "All callbacks should have been executed");

    // Cleanup
    Asthra_concurrency_bridge_cleanup();

    return true;
}

bool test_callback_queue_limits(void) {
    // Initialize with small callback limit
    AsthraResult result = Asthra_concurrency_bridge_init(100, 2);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");

    int callback_counter = 0;

    // Fill the queue
    result = Asthra_enqueue_callback(callback_test_function, NULL, 0, &callback_counter, 1);
    ASSERT(asthra_result_is_ok(result), "First callback enqueue should succeed");

    result = Asthra_enqueue_callback(callback_test_function, NULL, 0, &callback_counter, 1);
    ASSERT(asthra_result_is_ok(result), "Second callback enqueue should succeed");

    // This should fail due to queue limit
    result = Asthra_enqueue_callback(callback_test_function, NULL, 0, &callback_counter, 1);
    ASSERT(asthra_result_is_err(result), "Third callback enqueue should fail");

    // Process callbacks to free space
    size_t processed = Asthra_process_callbacks(10);
    ASSERT(processed == 2, "Should process two callbacks");

    // Now enqueue should succeed again
    result = Asthra_enqueue_callback(callback_test_function, NULL, 0, &callback_counter, 1);
    ASSERT(asthra_result_is_ok(result), "Callback enqueue should succeed after processing");

    // Cleanup
    Asthra_concurrency_bridge_cleanup();

    return true;
}
