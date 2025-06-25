/**
 * Asthra Concurrency Semaphore Implementation
 * Part of the Asthra Runtime Concurrency System
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * DESIGN GOALS:
 * - Counting semaphore for resource management
 * - Timeout support for acquire operations
 * - Monitoring and statistics
 * - Thread-safe operations
 */

#include "../asthra_runtime.h"
#include "asthra_concurrency_atomics.h"
#include "asthra_concurrency_sync.h"
#include <errno.h>
#include <stdlib.h>
#include <time.h>

// Forward declarations for atomic functions
extern size_t asthra_concurrency_atomic_increment_size(asthra_concurrency_atomic_size_t *size_ptr);
extern size_t asthra_concurrency_atomic_decrement_size(asthra_concurrency_atomic_size_t *size_ptr);
extern uint64_t
asthra_concurrency_atomic_increment_counter(asthra_concurrency_atomic_counter_t *counter);

AsthraConcurrencySemaphore *Asthra_semaphore_create(size_t permits) {
    AsthraConcurrencySemaphore *semaphore = malloc(sizeof(AsthraConcurrencySemaphore));
    if (!semaphore) {
        return NULL;
    }

    // Initialize synchronization primitives
    if (!ASTHRA_CONCURRENCY_MUTEX_INIT(&semaphore->mutex)) {
        free(semaphore);
        return NULL;
    }

    if (!ASTHRA_CONCURRENCY_COND_INIT(&semaphore->cond)) {
        ASTHRA_CONCURRENCY_MUTEX_DESTROY(&semaphore->mutex);
        free(semaphore);
        return NULL;
    }

    // Initialize atomic fields using the existing structure fields
    atomic_store(&semaphore->permits, permits);
    atomic_store(&semaphore->max_permits, permits);
    atomic_store(&semaphore->acquire_count, 0);
    atomic_store(&semaphore->release_count, 0);

    semaphore->name = NULL;

    return semaphore;
}

AsthraResult Asthra_semaphore_acquire(AsthraConcurrencySemaphore *semaphore) {
    if (!semaphore) {
        return asthra_result_err_cstr("Invalid semaphore");
    }

    if (!ASTHRA_CONCURRENCY_MUTEX_LOCK(&semaphore->mutex)) {
        return asthra_result_err_cstr("Failed to lock semaphore mutex");
    }

    // Wait until permits are available
    while (atomic_load(&semaphore->permits) == 0) {
        if (!ASTHRA_CONCURRENCY_COND_WAIT(&semaphore->cond, &semaphore->mutex)) {
            ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&semaphore->mutex);
            return asthra_result_err_cstr("Failed to wait on semaphore condition");
        }
    }

    // Acquire a permit
    asthra_concurrency_atomic_decrement_size(&semaphore->permits);
    asthra_concurrency_atomic_increment_counter(&semaphore->acquire_count);

    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&semaphore->mutex);
    return asthra_result_ok_cstr("Semaphore acquired");
}

bool Asthra_semaphore_try_acquire(AsthraConcurrencySemaphore *semaphore) {
    if (!semaphore) {
        return false;
    }

    if (!ASTHRA_CONCURRENCY_MUTEX_LOCK(&semaphore->mutex)) {
        return false;
    }

    bool acquired = false;

    // Check if permits are available
    if (atomic_load(&semaphore->permits) > 0) {
        // Acquire a permit
        asthra_concurrency_atomic_decrement_size(&semaphore->permits);
        asthra_concurrency_atomic_increment_counter(&semaphore->acquire_count);
        acquired = true;
    }

    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&semaphore->mutex);
    return acquired;
}

AsthraResult Asthra_semaphore_acquire_timeout(AsthraConcurrencySemaphore *semaphore,
                                              int64_t timeout_ms, bool *acquired) {
    if (!semaphore || !acquired) {
        return asthra_result_err_cstr("Invalid semaphore or acquired pointer");
    }

    *acquired = false;

    if (!ASTHRA_CONCURRENCY_MUTEX_LOCK(&semaphore->mutex)) {
        return asthra_result_err_cstr("Failed to lock semaphore mutex");
    }

    // Calculate timeout
    struct timespec timeout_spec;
    clock_gettime(CLOCK_REALTIME, &timeout_spec);
    timeout_spec.tv_sec += timeout_ms / 1000;
    timeout_spec.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (timeout_spec.tv_nsec >= 1000000000) {
        timeout_spec.tv_sec++;
        timeout_spec.tv_nsec -= 1000000000;
    }

    // Wait until permits are available or timeout
    while (atomic_load(&semaphore->permits) == 0) {
        int result =
            ASTHRA_CONCURRENCY_COND_TIMEDWAIT(&semaphore->cond, &semaphore->mutex, &timeout_spec);
        if (result == ETIMEDOUT) {
            ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&semaphore->mutex);
            return asthra_result_ok_cstr("Semaphore acquire timeout"); // Timeout, but not an error
        } else if (result != 0) {
            ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&semaphore->mutex);
            return asthra_result_err_cstr("Failed to wait on semaphore condition with timeout");
        }
    }

    // Acquire a permit
    asthra_concurrency_atomic_decrement_size(&semaphore->permits);
    asthra_concurrency_atomic_increment_counter(&semaphore->acquire_count);
    *acquired = true;

    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&semaphore->mutex);
    return asthra_result_ok_cstr("Semaphore acquired with timeout");
}

AsthraResult Asthra_semaphore_release(AsthraConcurrencySemaphore *semaphore) {
    if (!semaphore) {
        return asthra_result_err_cstr("Invalid semaphore");
    }

    if (!ASTHRA_CONCURRENCY_MUTEX_LOCK(&semaphore->mutex)) {
        return asthra_result_err_cstr("Failed to lock semaphore mutex");
    }

    // Check if we can release (don't exceed max permits)
    size_t current_permits = atomic_load(&semaphore->permits);
    size_t max_permits = atomic_load(&semaphore->max_permits);

    if (current_permits >= max_permits) {
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&semaphore->mutex);
        return asthra_result_err_cstr("Cannot release - maximum permits reached");
    }

    // Release a permit
    asthra_concurrency_atomic_increment_size(&semaphore->permits);
    asthra_concurrency_atomic_increment_counter(&semaphore->release_count);

    // Signal waiting threads
    ASTHRA_CONCURRENCY_COND_SIGNAL(&semaphore->cond);

    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&semaphore->mutex);
    return asthra_result_ok_cstr("Semaphore released");
}

size_t Asthra_semaphore_available_permits(const AsthraConcurrencySemaphore *semaphore) {
    if (!semaphore) {
        return 0;
    }
    return atomic_load(&semaphore->permits);
}

void Asthra_semaphore_destroy(AsthraConcurrencySemaphore *semaphore) {
    if (!semaphore) {
        return;
    }

    // Wake up any waiting threads
    ASTHRA_CONCURRENCY_COND_BROADCAST(&semaphore->cond);

    // Destroy synchronization primitives
    ASTHRA_CONCURRENCY_COND_DESTROY(&semaphore->cond);
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&semaphore->mutex);

    free(semaphore);
}
