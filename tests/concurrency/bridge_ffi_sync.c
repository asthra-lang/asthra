/**
 * Asthra Concurrency Bridge FFI - Synchronization Primitives
 * Mutex creation and synchronization operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_ffi_common.h"

// =============================================================================
// SYNCHRONIZATION PRIMITIVES
// =============================================================================

AsthraConcurrencyMutex* Asthra_mutex_create(const char* name, bool recursive) {
    if (!asthra_concurrency_is_initialized()) {
        return NULL;
    }
    
    AsthraConcurrencyMutex *mutex = malloc(sizeof(AsthraConcurrencyMutex));
    if (!mutex) {
        return NULL;
    }
    
    if (recursive) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mutex->mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    } else {
        pthread_mutex_init(&mutex->mutex, NULL);
    }
    mutex->name = name;
    
    // Register mutex
    pthread_mutex_lock(&bridge_state.mutex);
    
    if (bridge_state.mutex_registry.count < bridge_state.mutex_registry.capacity) {
        bridge_state.mutex_registry.mutexes[bridge_state.mutex_registry.count++] = mutex;
    }
    
    pthread_mutex_unlock(&bridge_state.mutex);
    
    return mutex;
}

AsthraConcurrencyMutex* Asthra_recursive_mutex_create(void) {
    if (!asthra_concurrency_is_initialized()) {
        return NULL;
    }
    
    AsthraConcurrencyMutex *mutex = malloc(sizeof(AsthraConcurrencyMutex));
    if (!mutex) {
        return NULL;
    }
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    
    // Recursive mutex created via attributes
    
    // Register mutex
    pthread_mutex_lock(&bridge_state.mutex);
    
    if (bridge_state.mutex_registry.count < bridge_state.mutex_registry.capacity) {
        bridge_state.mutex_registry.mutexes[bridge_state.mutex_registry.count++] = mutex;
    }
    
    pthread_mutex_unlock(&bridge_state.mutex);
    
    return mutex;
}

void Asthra_mutex_destroy(AsthraConcurrencyMutex* mutex) {
    if (!mutex) {
        return;
    }
    
    pthread_mutex_destroy(&mutex->mutex);
    
    // Unregister mutex
    pthread_mutex_lock(&bridge_state.mutex);
    
    for (size_t i = 0; i < bridge_state.mutex_registry.count; i++) {
        if (bridge_state.mutex_registry.mutexes[i] == mutex) {
            // Shift remaining entries
            for (size_t j = i + 1; j < bridge_state.mutex_registry.count; j++) {
                bridge_state.mutex_registry.mutexes[j - 1] = bridge_state.mutex_registry.mutexes[j];
            }
            bridge_state.mutex_registry.count--;
            break;
        }
    }
    
    pthread_mutex_unlock(&bridge_state.mutex);
    
    free(mutex);
}

AsthraResult Asthra_mutex_lock(AsthraConcurrencyMutex* mutex) {
    if (!mutex) {
        return create_error("Invalid mutex");
    }
    
    int ret = pthread_mutex_lock(&mutex->mutex);
    if (ret != 0) {
        atomic_fetch_add(&bridge_state.stats.mutex_contentions, 1);
        return create_error("Mutex lock failed");
    }
    
    return create_ok();
}

AsthraResult Asthra_mutex_unlock(AsthraConcurrencyMutex* mutex) {
    if (!mutex) {
        return create_error("Invalid mutex");
    }
    
    int ret = pthread_mutex_unlock(&mutex->mutex);
    if (ret != 0) {
        return create_error("Mutex unlock failed");
    }
    
    return create_ok();
}

AsthraResult Asthra_mutex_try_lock(AsthraConcurrencyMutex* mutex) {
    if (!mutex) {
        return create_error("Invalid mutex");
    }
    
    int ret = pthread_mutex_trylock(&mutex->mutex);
    if (ret == EBUSY) {
        atomic_fetch_add(&bridge_state.stats.mutex_contentions, 1);
        return create_error("Mutex is locked");
    } else if (ret != 0) {
        return create_error("Mutex trylock failed");
    }
    
    return create_ok();
}