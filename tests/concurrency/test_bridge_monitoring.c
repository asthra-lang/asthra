/**
 * Asthra Enhanced Concurrency Bridge Test Suite - Monitoring Tests
 * Tests for statistics tracking and state monitoring functionality
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_test_common.h"

// =============================================================================
// STATISTICS AND MONITORING TESTS
// =============================================================================

bool test_statistics_tracking(void) {
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");
    
    // Reset statistics
    Asthra_reset_concurrency_stats();
    
    // Get initial statistics
    AsthraConcurrencyStats initial_stats = Asthra_get_concurrency_stats();
    ASSERT(initial_stats.tasks_spawned == 0, "Initial tasks spawned should be 0");
    ASSERT(initial_stats.callbacks_enqueued == 0, "Initial callbacks enqueued should be 0");
    
    // Spawn a task
    int test_value = 1;
    AsthraConcurrencyTaskHandle *handle = Asthra_spawn_task(simple_task_function, &test_value, 
                                                          sizeof(int), NULL);
    ASSERT(handle != NULL, "Task spawn should succeed");
    
    // Wait for completion
    AsthraResult task_result = Asthra_task_get_result(handle);
    ASSERT(asthra_result_is_ok(task_result), "Task should complete successfully");
    
    // Enqueue a callback
    int callback_counter = 0;
    result = Asthra_enqueue_callback(callback_test_function, NULL, 0, 
                                   &callback_counter, 1);
    ASSERT(asthra_result_is_ok(result), "Callback enqueue should succeed");
    
    // Process callback
    size_t processed = Asthra_process_callbacks(1);
    ASSERT(processed == 1, "Should process one callback");
    
    // Check updated statistics
    AsthraConcurrencyStats final_stats = Asthra_get_concurrency_stats();
    ASSERT(final_stats.tasks_spawned >= 1, "Tasks spawned should be at least 1");
    ASSERT(final_stats.tasks_completed >= 1, "Tasks completed should be at least 1");
    ASSERT(final_stats.callbacks_enqueued >= 1, "Callbacks enqueued should be at least 1");
    ASSERT(final_stats.callbacks_processed >= 1, "Callbacks processed should be at least 1");
    
    // Cleanup
    Asthra_task_handle_free(handle);
    Asthra_concurrency_bridge_cleanup();
    
    return true;
}

bool test_state_dump(void) {
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");
    
    // Dump state (should not crash)
    printf("\n--- State Dump Test ---\n");
    Asthra_dump_concurrency_state(stdout);
    printf("--- End State Dump ---\n");
    
    // Cleanup
    Asthra_concurrency_bridge_cleanup();
    
    return true;
} 
