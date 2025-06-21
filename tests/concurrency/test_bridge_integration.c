/**
 * Asthra Enhanced Concurrency Bridge Test Suite - Integration Tests
 * Tests for multiple components working together (tasks + synchronization)
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_test_common.h"

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

bool test_concurrent_task_synchronization(void) {
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");
    
    // Create mutex and test data
    AsthraConcurrencyMutex *mutex = Asthra_mutex_create("sync_test_mutex", false);
    ASSERT(mutex != NULL, "Mutex creation should succeed");
    
    // Allocate test data on heap so all tasks share the same memory
    MutexTestData *test_data = malloc(sizeof(MutexTestData));
    test_data->mutex = mutex;
    test_data->shared_counter = 0;
    test_data->num_increments = 100;
    
    const int num_tasks = 5;
    AsthraConcurrencyTaskHandle *handles[num_tasks];
    
    // Spawn multiple tasks that increment shared counter
    // Pass a pointer to the shared data, not the data itself
    for (int i = 0; i < num_tasks; i++) {
        handles[i] = Asthra_spawn_task(mutex_increment_task, &test_data, 
                                      sizeof(MutexTestData*), NULL);
        ASSERT(handles[i] != NULL, "Task spawn should succeed");
    }
    
    // Wait for all tasks to complete
    for (int i = 0; i < num_tasks; i++) {
        AsthraResult task_result = Asthra_task_get_result(handles[i]);
        ASSERT(asthra_result_is_ok(task_result), "Task should complete successfully");
    }
    
    // Verify final counter value
    int expected_value = num_tasks * test_data->num_increments;
    ASSERT(test_data->shared_counter == expected_value, 
           "Shared counter should have correct final value");
    
    // Cleanup
    for (int i = 0; i < num_tasks; i++) {
        Asthra_task_handle_free(handles[i]);
    }
    Asthra_mutex_destroy(mutex);
    free(test_data);
    Asthra_concurrency_bridge_cleanup();
    
    return true;
} 
