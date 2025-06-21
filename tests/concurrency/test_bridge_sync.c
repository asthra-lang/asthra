/**
 * Asthra Enhanced Concurrency Bridge Test Suite - Synchronization Tests
 * Tests for mutex operations and thread synchronization primitives
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_test_common.h"

// =============================================================================
// SYNCHRONIZATION TESTS
// =============================================================================

bool test_mutex_basic_operations(void) {
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");
    
    // Create mutex
    AsthraConcurrencyMutex *mutex = Asthra_mutex_create("test_mutex", false);
    ASSERT(mutex != NULL, "Mutex creation should succeed");
    
    // Lock mutex
    result = Asthra_mutex_lock(mutex);
    ASSERT(asthra_result_is_ok(result), "Mutex lock should succeed");
    
    // Try lock should fail
    result = Asthra_mutex_trylock(mutex);
    ASSERT(asthra_result_is_err(result), "Mutex trylock should fail when already locked");
    
    // Unlock mutex
    result = Asthra_mutex_unlock(mutex);
    ASSERT(asthra_result_is_ok(result), "Mutex unlock should succeed");
    
    // Try lock should now succeed
    result = Asthra_mutex_trylock(mutex);
    ASSERT(asthra_result_is_ok(result), "Mutex trylock should succeed when unlocked");
    
    // Unlock again
    result = Asthra_mutex_unlock(mutex);
    ASSERT(asthra_result_is_ok(result), "Mutex unlock should succeed");
    
    // Destroy mutex
    Asthra_mutex_destroy(mutex);
    
    // Cleanup
    Asthra_concurrency_bridge_cleanup();
    
    return true;
}

bool test_recursive_mutex(void) {
    AsthraResult result = Asthra_concurrency_bridge_init(100, 1000);
    ASSERT(asthra_result_is_ok(result), "Bridge initialization should succeed");
    
    // Create recursive mutex
    AsthraConcurrencyMutex *mutex = Asthra_mutex_create("recursive_test_mutex", true);
    ASSERT(mutex != NULL, "Recursive mutex creation should succeed");
    
    // Lock multiple times
    result = Asthra_mutex_lock(mutex);
    ASSERT(asthra_result_is_ok(result), "First mutex lock should succeed");
    
    result = Asthra_mutex_lock(mutex);
    ASSERT(asthra_result_is_ok(result), "Second mutex lock should succeed");
    
    result = Asthra_mutex_lock(mutex);
    ASSERT(asthra_result_is_ok(result), "Third mutex lock should succeed");
    
    // Unlock same number of times
    result = Asthra_mutex_unlock(mutex);
    ASSERT(asthra_result_is_ok(result), "First mutex unlock should succeed");
    
    result = Asthra_mutex_unlock(mutex);
    ASSERT(asthra_result_is_ok(result), "Second mutex unlock should succeed");
    
    result = Asthra_mutex_unlock(mutex);
    ASSERT(asthra_result_is_ok(result), "Third mutex unlock should succeed");
    
    // Destroy mutex
    Asthra_mutex_destroy(mutex);
    
    // Cleanup
    Asthra_concurrency_bridge_cleanup();
    
    return true;
} 
