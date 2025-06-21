/**
 * Asthra Programming Language - Synchronization Primitives Testing
 * 
 * Phase 4: Core Infrastructure Implementation
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Synchronization primitives testing utilities
 */

#include "concurrency_integration_common.h"

// =============================================================================
// SYNCHRONIZATION PRIMITIVES TESTING
// =============================================================================

/**
 * Initialize synchronization primitives
 */
SynchronizationPrimitives* sync_primitives_create(int max_threads) {
    SynchronizationPrimitives* sync = calloc(1, sizeof(SynchronizationPrimitives));
    if (!sync) return NULL;
    
    if (pthread_mutex_init(&sync->mutex, NULL) != 0) {
        free(sync);
        return NULL;
    }
    
    if (pthread_cond_init(&sync->condition, NULL) != 0) {
        pthread_mutex_destroy(&sync->mutex);
        free(sync);
        return NULL;
    }
    
    if (pthread_rwlock_init(&sync->rwlock, NULL) != 0) {
        pthread_cond_destroy(&sync->condition);
        pthread_mutex_destroy(&sync->mutex);
        free(sync);
        return NULL;
    }
    
    atomic_store(&sync->barrier_ready, false);
    atomic_store(&sync->barrier_count, 0);
    sync->max_threads = max_threads;
    
    return sync;
}

/**
 * Test barrier synchronization
 */
bool sync_primitives_barrier_wait(SynchronizationPrimitives* sync) {
    if (!sync) return false;
    
    int current_count = atomic_fetch_add(&sync->barrier_count, 1) + 1;
    
    if (current_count == sync->max_threads) {
        // Last thread to reach barrier
        atomic_store(&sync->barrier_ready, true);
        return true;
    } else {
        // Wait for barrier to be ready
        while (!atomic_load(&sync->barrier_ready)) {
            sleep_ms(1); // Small delay to avoid busy waiting
        }
        return true;
    }
}

/**
 * Test mutex performance
 */
bool sync_primitives_test_mutex(SynchronizationPrimitives* sync, int iterations) {
    if (!sync) return false;
    
    for (int i = 0; i < iterations; i++) {
        if (pthread_mutex_lock(&sync->mutex) != 0) {
            return false;
        }
        
        // Simulate some work
        volatile int dummy = 0;
        for (int j = 0; j < 100; j++) {
            dummy += j;
        }
        
        if (pthread_mutex_unlock(&sync->mutex) != 0) {
            return false;
        }
    }
    
    return true;
}

/**
 * Test read-write lock performance
 */
bool sync_primitives_test_rwlock_read(SynchronizationPrimitives* sync, int iterations) {
    if (!sync) return false;
    
    for (int i = 0; i < iterations; i++) {
        if (pthread_rwlock_rdlock(&sync->rwlock) != 0) {
            return false;
        }
        
        // Simulate read operation
        sleep_ms(1);
        
        if (pthread_rwlock_unlock(&sync->rwlock) != 0) {
            return false;
        }
    }
    
    return true;
}

bool sync_primitives_test_rwlock_write(SynchronizationPrimitives* sync, int iterations) {
    if (!sync) return false;
    
    for (int i = 0; i < iterations; i++) {
        if (pthread_rwlock_wrlock(&sync->rwlock) != 0) {
            return false;
        }
        
        // Simulate write operation
        sleep_ms(2);
        
        if (pthread_rwlock_unlock(&sync->rwlock) != 0) {
            return false;
        }
    }
    
    return true;
}

/**
 * Destroy synchronization primitives
 */
void sync_primitives_destroy(SynchronizationPrimitives* sync) {
    if (!sync) return;
    
    pthread_rwlock_destroy(&sync->rwlock);
    pthread_cond_destroy(&sync->condition);
    pthread_mutex_destroy(&sync->mutex);
    free(sync);
}