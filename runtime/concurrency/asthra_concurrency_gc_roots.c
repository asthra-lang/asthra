/**
 * Asthra Concurrency Thread Management - GC Root Management Implementation
 * Thread-local GC root registration and management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_concurrency_gc_roots.h"

// =============================================================================
// THREAD-LOCAL GC ROOT MANAGEMENT
// =============================================================================

AsthraResult Asthra_register_thread_gc_root(void *ptr) {
    if (!ptr) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE, 
                                                     "Invalid GC root pointer");
    }
    
    AsthraConcurrencyThreadData *thread_data = asthra_concurrency_get_thread_data();
    if (!thread_data) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INIT_FAILED, 
                                                     "Thread not registered");
    }
    
    // Expand capacity if needed
    size_t current_count = asthra_concurrency_atomic_load_size(&thread_data->gc_root_count);
    if (current_count >= thread_data->gc_root_capacity) {
        size_t new_capacity = thread_data->gc_root_capacity ? 
                             thread_data->gc_root_capacity * 2 : 16;
        void **new_roots = realloc(thread_data->gc_roots, new_capacity * sizeof(void*));
        if (!new_roots) {
            return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR, 
                                                         "Failed to expand GC roots array");
        }
        
        thread_data->gc_roots = new_roots;
        thread_data->gc_root_capacity = new_capacity;
    }
    
    // Add to thread-local roots
    size_t index = asthra_concurrency_atomic_increment_size(&thread_data->gc_root_count) - 1;
    thread_data->gc_roots[index] = ptr;
    
    // Also register with global GC
    asthra_gc_register_root(ptr);
    
    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
    asthra_concurrency_atomic_increment_counter(&bridge->stats.gc_roots_registered);
    
    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
}

AsthraResult Asthra_unregister_thread_gc_root(void *ptr) {
    if (!ptr) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE, 
                                                     "Invalid GC root pointer");
    }
    
    AsthraConcurrencyThreadData *thread_data = asthra_concurrency_get_thread_data();
    if (!thread_data) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INIT_FAILED, 
                                                     "Thread not registered");
    }
    
    // Find and remove from thread-local roots
    size_t current_count = asthra_concurrency_atomic_load_size(&thread_data->gc_root_count);
    bool found = false;
    
    for (size_t i = 0; i < current_count; i++) {
        if (thread_data->gc_roots[i] == ptr) {
            // Move last element to this position
            thread_data->gc_roots[i] = thread_data->gc_roots[current_count - 1];
            asthra_concurrency_atomic_decrement_size(&thread_data->gc_root_count);
            found = true;
            break;
        }
    }
    
    if (!found) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE, 
                                                     "GC root not found in thread registry");
    }
    
    // Also unregister from global GC
    asthra_gc_unregister_root(ptr);
    
    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
} 
