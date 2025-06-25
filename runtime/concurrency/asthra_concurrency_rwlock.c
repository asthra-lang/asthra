/**
 * Asthra Concurrency Thread Management - Read-Write Lock Implementation
 * Read-write lock creation and locking primitives
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_concurrency_rwlock.h"
#include "asthra_concurrency_atomics.h"
#include "asthra_concurrency_sync.h"

// =============================================================================
// READ-WRITE LOCK IMPLEMENTATION
// =============================================================================

AsthraConcurrencyRWLock *Asthra_rwlock_create(const char *name) {
    AsthraConcurrencyRWLock *rwlock = malloc(sizeof(AsthraConcurrencyRWLock));
    if (!rwlock) {
        return NULL;
    }

#if !ASTHRA_HAS_C11_THREADS
    if (pthread_rwlock_init(&rwlock->rwlock, NULL) != 0) {
        free(rwlock);
        return NULL;
    }
#else
    // Initialize mutex and condition variables for C11 threads
    if (!ASTHRA_CONCURRENCY_MUTEX_INIT(&rwlock->rw_mutex) ||
        !ASTHRA_CONCURRENCY_COND_INIT(&rwlock->read_cond) ||
        !ASTHRA_CONCURRENCY_COND_INIT(&rwlock->write_cond)) {
        free(rwlock);
        return NULL;
    }
#endif

    // Note: creation_time field doesn't exist in the structure
    rwlock->name = name ? strdup(name) : NULL;
    atomic_store_explicit(&rwlock->reader_count, 0, ASTHRA_MEMORY_ORDER_RELAXED);
    atomic_store_explicit(&rwlock->writer_active, false, ASTHRA_MEMORY_ORDER_RELAXED);
    atomic_store_explicit(&rwlock->waiting_writers, 0, ASTHRA_MEMORY_ORDER_RELAXED);
    asthra_concurrency_atomic_store_counter(&rwlock->read_lock_count, 0);
    asthra_concurrency_atomic_store_counter(&rwlock->write_lock_count, 0);

    return rwlock;
}

AsthraResult Asthra_rwlock_read_lock(AsthraConcurrencyRWLock *rwlock) {
    if (!rwlock) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid read-write lock handle");
    }

#if !ASTHRA_HAS_C11_THREADS
    int result = pthread_rwlock_rdlock(&rwlock->rwlock);
#else
    // C11 threads implementation: use mutex-based approach
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&rwlock->rw_mutex);
    while (atomic_load_explicit(&rwlock->writer_active, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        ASTHRA_CONCURRENCY_COND_WAIT(&rwlock->read_cond, &rwlock->rw_mutex);
    }
    atomic_fetch_add_explicit(&rwlock->reader_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&rwlock->rw_mutex);
    int result = 0;
#endif
    if (result == 0) {
#if !ASTHRA_HAS_C11_THREADS
        atomic_fetch_add_explicit(&rwlock->reader_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
#endif
        asthra_concurrency_atomic_increment_counter(&rwlock->read_lock_count);
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else {
        if (asthra_concurrency_is_initialized()) {
            AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
            asthra_concurrency_atomic_increment_counter(&bridge->stats.rwlock_contentions);
        }
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR,
                                                      "Read-write lock read lock failed");
    }
}

AsthraResult Asthra_rwlock_write_lock(AsthraConcurrencyRWLock *rwlock) {
    if (!rwlock) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid read-write lock handle");
    }

#if !ASTHRA_HAS_C11_THREADS
    int result = pthread_rwlock_wrlock(&rwlock->rwlock);
#else
    // C11 threads implementation: use mutex-based approach
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&rwlock->rw_mutex);
    atomic_fetch_add_explicit(&rwlock->waiting_writers, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    while (atomic_load_explicit(&rwlock->writer_active, ASTHRA_MEMORY_ORDER_ACQUIRE) ||
           atomic_load_explicit(&rwlock->reader_count, ASTHRA_MEMORY_ORDER_ACQUIRE) > 0) {
        ASTHRA_CONCURRENCY_COND_WAIT(&rwlock->write_cond, &rwlock->rw_mutex);
    }
    atomic_fetch_sub_explicit(&rwlock->waiting_writers, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    atomic_store_explicit(&rwlock->writer_active, true, ASTHRA_MEMORY_ORDER_RELEASE);
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&rwlock->rw_mutex);
    int result = 0;
#endif
    if (result == 0) {
#if !ASTHRA_HAS_C11_THREADS
        atomic_store_explicit(&rwlock->writer_active, true, ASTHRA_MEMORY_ORDER_RELAXED);
#endif
        asthra_concurrency_atomic_increment_counter(&rwlock->write_lock_count);
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else {
        if (asthra_concurrency_is_initialized()) {
            AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
            asthra_concurrency_atomic_increment_counter(&bridge->stats.rwlock_contentions);
        }
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR,
                                                      "Read-write lock write lock failed");
    }
}

AsthraResult Asthra_rwlock_try_read_lock(AsthraConcurrencyRWLock *rwlock) {
    if (!rwlock) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid read-write lock handle");
    }

#if !ASTHRA_HAS_C11_THREADS
    int result = pthread_rwlock_tryrdlock(&rwlock->rwlock);
#else
    // C11 threads implementation: try to acquire read lock
    int result = -1;
    if (ASTHRA_CONCURRENCY_MUTEX_TRYLOCK(&rwlock->rw_mutex)) {
        if (!atomic_load_explicit(&rwlock->writer_active, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
            atomic_fetch_add_explicit(&rwlock->reader_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
            result = 0;
        }
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&rwlock->rw_mutex);
    }
#endif
    if (result == 0) {
#if !ASTHRA_HAS_C11_THREADS
        atomic_fetch_add_explicit(&rwlock->reader_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
#endif
        asthra_concurrency_atomic_increment_counter(&rwlock->read_lock_count);
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else if (result == EBUSY) {
        return asthra_concurrency_create_error_result(
            ASTHRA_CONCURRENCY_ERROR_MUTEX_TIMEOUT,
            "Read-write lock is already locked for writing");
    } else {
        if (asthra_concurrency_is_initialized()) {
            AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
            asthra_concurrency_atomic_increment_counter(&bridge->stats.rwlock_contentions);
        }
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR,
                                                      "Read-write lock try read lock failed");
    }
}

AsthraResult Asthra_rwlock_try_write_lock(AsthraConcurrencyRWLock *rwlock) {
    if (!rwlock) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid read-write lock handle");
    }

#if !ASTHRA_HAS_C11_THREADS
    int result = pthread_rwlock_trywrlock(&rwlock->rwlock);
#else
    // C11 threads implementation: try to acquire write lock
    int result = -1;
    if (ASTHRA_CONCURRENCY_MUTEX_TRYLOCK(&rwlock->rw_mutex)) {
        if (!atomic_load_explicit(&rwlock->writer_active, ASTHRA_MEMORY_ORDER_ACQUIRE) &&
            atomic_load_explicit(&rwlock->reader_count, ASTHRA_MEMORY_ORDER_ACQUIRE) == 0) {
            atomic_store_explicit(&rwlock->writer_active, true, ASTHRA_MEMORY_ORDER_RELEASE);
            result = 0;
        }
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&rwlock->rw_mutex);
    }
#endif
    if (result == 0) {
#if !ASTHRA_HAS_C11_THREADS
        atomic_store_explicit(&rwlock->writer_active, true, ASTHRA_MEMORY_ORDER_RELAXED);
#endif
        asthra_concurrency_atomic_increment_counter(&rwlock->write_lock_count);
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else if (result == EBUSY) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_MUTEX_TIMEOUT,
                                                      "Read-write lock is already locked");
    } else {
        if (asthra_concurrency_is_initialized()) {
            AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
            asthra_concurrency_atomic_increment_counter(&bridge->stats.rwlock_contentions);
        }
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR,
                                                      "Read-write lock try write lock failed");
    }
}

AsthraResult Asthra_rwlock_unlock(AsthraConcurrencyRWLock *rwlock) {
    if (!rwlock) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid read-write lock handle");
    }

    // Check if writer is active and reset it
    bool was_writer = atomic_load_explicit(&rwlock->writer_active, ASTHRA_MEMORY_ORDER_RELAXED);
    if (was_writer) {
        atomic_store_explicit(&rwlock->writer_active, false, ASTHRA_MEMORY_ORDER_RELAXED);
    } else {
        // Must be a reader unlock
        atomic_fetch_sub_explicit(&rwlock->reader_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    }

#if !ASTHRA_HAS_C11_THREADS
    int result = pthread_rwlock_unlock(&rwlock->rwlock);
#else
    // C11 threads implementation: unlock either read or write lock
    int result = 0;
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&rwlock->rw_mutex);
    if (atomic_load_explicit(&rwlock->writer_active, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        // Releasing write lock
        atomic_store_explicit(&rwlock->writer_active, false, ASTHRA_MEMORY_ORDER_RELEASE);
        if (atomic_load_explicit(&rwlock->waiting_writers, ASTHRA_MEMORY_ORDER_ACQUIRE) > 0) {
            ASTHRA_CONCURRENCY_COND_SIGNAL(&rwlock->write_cond);
        } else {
            ASTHRA_CONCURRENCY_COND_BROADCAST(&rwlock->read_cond);
        }
    } else if (atomic_load_explicit(&rwlock->reader_count, ASTHRA_MEMORY_ORDER_ACQUIRE) > 0) {
        // Releasing read lock
        atomic_fetch_sub_explicit(&rwlock->reader_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
        if (atomic_load_explicit(&rwlock->reader_count, ASTHRA_MEMORY_ORDER_ACQUIRE) == 0 &&
            atomic_load_explicit(&rwlock->waiting_writers, ASTHRA_MEMORY_ORDER_ACQUIRE) > 0) {
            ASTHRA_CONCURRENCY_COND_SIGNAL(&rwlock->write_cond);
        }
    } else {
        result = -1; // Error: no lock held
    }
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&rwlock->rw_mutex);
#endif
    if (result == 0) {
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR,
                                                      "Read-write lock unlock failed");
    }
}

void Asthra_rwlock_destroy(AsthraConcurrencyRWLock *rwlock) {
    if (!rwlock) {
        return;
    }

#if !ASTHRA_HAS_C11_THREADS
    pthread_rwlock_destroy(&rwlock->rwlock);
#else
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&rwlock->rw_mutex);
    ASTHRA_CONCURRENCY_COND_DESTROY(&rwlock->read_cond);
    ASTHRA_CONCURRENCY_COND_DESTROY(&rwlock->write_cond);
#endif

    if (rwlock->name) {
        char *mutable_name = (char *)rwlock->name;
        free(mutable_name);
    }

    free(rwlock);
}
