/**
 * Asthra Concurrency Thread Management - Mutex Operations Implementation
 * Mutex creation, locking, and management primitives
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_concurrency_mutex.h"

// =============================================================================
// MUTEX IMPLEMENTATION
// =============================================================================

AsthraConcurrencyMutex *Asthra_mutex_create(const char *name, bool recursive) {
    AsthraConcurrencyMutex *mutex = malloc(sizeof(AsthraConcurrencyMutex));
    if (!mutex) {
        return NULL;
    }

    asthra_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) {
        free(mutex);
        return NULL;
    }

    if (recursive) {
        if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
            pthread_mutexattr_destroy(&attr);
            free(mutex);
            return NULL;
        }
    }

    if (pthread_mutex_init(&mutex->mutex, &attr) != 0) {
        pthread_mutexattr_destroy(&attr);
        free(mutex);
        return NULL;
    }

    pthread_mutexattr_destroy(&attr);

    // Initialize metadata
    mutex->name = name ? strdup(name) : NULL;
    asthra_concurrency_atomic_store_counter(&mutex->lock_count, 0);
    asthra_concurrency_atomic_store_counter(&mutex->contention_count, 0);
    asthra_concurrency_atomic_store_counter(&mutex->creation_time,
                                            asthra_concurrency_get_timestamp_ms());
    atomic_store_explicit(&mutex->owner, 0, ASTHRA_MEMORY_ORDER_RELAXED);
    atomic_store_explicit(&mutex->recursion_count, 0, ASTHRA_MEMORY_ORDER_RELAXED);

    return mutex;
}

AsthraResult Asthra_mutex_lock(AsthraConcurrencyMutex *mutex) {
    if (!mutex) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid mutex handle");
    }

    int result = pthread_mutex_lock(&mutex->mutex);
    if (result == 0) {
        atomic_store_explicit(&mutex->owner, pthread_self(), ASTHRA_MEMORY_ORDER_RELAXED);
        atomic_fetch_add_explicit(&mutex->recursion_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
        asthra_concurrency_atomic_increment_counter(&mutex->lock_count);
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else {
        if (asthra_concurrency_is_initialized()) {
            AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
            asthra_concurrency_atomic_increment_counter(&bridge->stats.mutex_contentions);
            asthra_concurrency_atomic_increment_counter(&mutex->contention_count);
        }
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR,
                                                      "Mutex lock failed");
    }
}

AsthraResult Asthra_mutex_trylock(AsthraConcurrencyMutex *mutex) {
    if (!mutex) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid mutex handle");
    }

    int result = pthread_mutex_trylock(&mutex->mutex);
    if (result == 0) {
        atomic_store_explicit(&mutex->owner, pthread_self(), ASTHRA_MEMORY_ORDER_RELAXED);
        atomic_fetch_add_explicit(&mutex->recursion_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
        asthra_concurrency_atomic_increment_counter(&mutex->lock_count);
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else if (result == EBUSY) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_MUTEX_TIMEOUT,
                                                      "Mutex is already locked");
    } else {
        if (asthra_concurrency_is_initialized()) {
            AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
            asthra_concurrency_atomic_increment_counter(&bridge->stats.mutex_contentions);
            asthra_concurrency_atomic_increment_counter(&mutex->contention_count);
        }
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR,
                                                      "Mutex trylock failed");
    }
}

AsthraResult Asthra_mutex_unlock(AsthraConcurrencyMutex *mutex) {
    if (!mutex) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid mutex handle");
    }

    int result = pthread_mutex_unlock(&mutex->mutex);
    if (result == 0) {
        int new_count =
            atomic_fetch_sub_explicit(&mutex->recursion_count, 1, ASTHRA_MEMORY_ORDER_RELAXED) - 1;
        if (new_count == 0) {
            atomic_store_explicit(&mutex->owner, 0, ASTHRA_MEMORY_ORDER_RELAXED);
        }
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR,
                                                      "Mutex unlock failed");
    }
}

void Asthra_mutex_destroy(AsthraConcurrencyMutex *mutex) {
    if (!mutex) {
        return;
    }

    pthread_mutex_destroy(&mutex->mutex);

    if (mutex->name) {
        char *mutable_name = (char *)mutex->name;
        free(mutable_name);
    }

    free(mutex);
}
