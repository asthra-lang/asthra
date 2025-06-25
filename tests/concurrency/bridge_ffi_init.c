/**
 * Asthra Concurrency Bridge FFI - Initialization
 * Bridge initialization and cleanup functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_ffi_common.h"

// =============================================================================
// GLOBAL STATE DEFINITION
// =============================================================================

BridgeState bridge_state = {
    .initialized = false,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .next_task_id = 1
};

// =============================================================================
// BRIDGE INITIALIZATION
// =============================================================================

AsthraResult Asthra_concurrency_bridge_init(size_t task_pool_size, size_t queue_size) {
    pthread_mutex_lock(&bridge_state.mutex);
    
    if (bridge_state.initialized) {
        pthread_mutex_unlock(&bridge_state.mutex);
        return create_ok();
    }
    
    // Initialize configuration
    bridge_state.task_pool_size = task_pool_size;
    bridge_state.queue_size = queue_size;
    
    // Initialize registries
    bridge_state.task_registry = NULL;
    bridge_state.callback_queue_head = NULL;
    bridge_state.callback_queue_tail = NULL;
    bridge_state.callback_queue_count = 0;
    bridge_state.thread_registry = NULL;
    
    // Initialize callback condition variable
    pthread_cond_init(&bridge_state.callback_available, NULL);
    
    // Initialize mutex registry
    bridge_state.mutex_registry.capacity = 100;
    bridge_state.mutex_registry.count = 0;
    bridge_state.mutex_registry.mutexes = calloc(bridge_state.mutex_registry.capacity, 
                                                  sizeof(AsthraConcurrencyMutex*));
    
    // Initialize statistics
    memset(&bridge_state.stats, 0, sizeof(AsthraConcurrencyStats));
    
    bridge_state.initialized = true;
    
    pthread_mutex_unlock(&bridge_state.mutex);
    return create_ok();
}

AsthraResult Asthra_concurrency_bridge_init_default(void) {
    return Asthra_concurrency_bridge_init(100, 1000);
}

void Asthra_concurrency_bridge_cleanup(void) {
    pthread_mutex_lock(&bridge_state.mutex);
    
    if (!bridge_state.initialized) {
        pthread_mutex_unlock(&bridge_state.mutex);
        return;
    }
    
    // Clean up task registry
    TaskRegistryEntry *task_entry = bridge_state.task_registry;
    while (task_entry) {
        TaskRegistryEntry *next = task_entry->next;
        // Note: Not freeing the handle itself as it may be owned elsewhere
        free(task_entry);
        task_entry = next;
    }
    
    // Clean up callback queue
    CallbackEntry *callback_entry = bridge_state.callback_queue_head;
    while (callback_entry) {
        CallbackEntry *next = callback_entry->next;
        if (callback_entry->data) {
            free(callback_entry->data);
        }
        free(callback_entry);
        callback_entry = next;
    }
    
    // Clean up thread registry
    ThreadRegistryEntry *thread_entry = bridge_state.thread_registry;
    while (thread_entry) {
        ThreadRegistryEntry *next = thread_entry->next;
        free(thread_entry);
        thread_entry = next;
    }
    
    // Clean up mutex registry
    if (bridge_state.mutex_registry.mutexes) {
        for (size_t i = 0; i < bridge_state.mutex_registry.count; i++) {
            if (bridge_state.mutex_registry.mutexes[i]) {
                pthread_mutex_destroy(&bridge_state.mutex_registry.mutexes[i]->mutex);
                free(bridge_state.mutex_registry.mutexes[i]);
            }
        }
        free(bridge_state.mutex_registry.mutexes);
    }
    
    pthread_cond_destroy(&bridge_state.callback_available);
    
    bridge_state.initialized = false;
    
    pthread_mutex_unlock(&bridge_state.mutex);
}

bool asthra_concurrency_is_initialized(void) {
    pthread_mutex_lock(&bridge_state.mutex);
    bool initialized = bridge_state.initialized;
    pthread_mutex_unlock(&bridge_state.mutex);
    return initialized;
}