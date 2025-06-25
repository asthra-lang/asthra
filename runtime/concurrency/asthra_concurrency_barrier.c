/**
 * Asthra Concurrency Barrier Implementation
 * Part of the Asthra Runtime Concurrency System
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * DESIGN GOALS:
 * - Thread barrier synchronization primitive
 * - Generation-based barrier reuse
 * - Monitoring and statistics
 * - Thread-safe operations
 */

#include "../asthra_runtime.h"
#include "asthra_concurrency_atomics.h"
#include "asthra_concurrency_sync.h"
#include <errno.h>
#include <stdlib.h>

// Forward declarations for atomic functions
extern size_t asthra_concurrency_atomic_increment_size(asthra_concurrency_atomic_size_t *size_ptr);
extern uint64_t
asthra_concurrency_atomic_increment_counter(asthra_concurrency_atomic_counter_t *counter);

// =============================================================================
// BARRIER IMPLEMENTATION
// =============================================================================

AsthraConcurrencyBarrier *Asthra_barrier_create(size_t party_count) {
    if (party_count == 0) {
        return NULL;
    }

    AsthraConcurrencyBarrier *barrier = malloc(sizeof(AsthraConcurrencyBarrier));
    if (!barrier) {
        return NULL;
    }

    // Initialize synchronization primitives
    if (!ASTHRA_CONCURRENCY_MUTEX_INIT(&barrier->mutex)) {
        free(barrier);
        return NULL;
    }

    if (!ASTHRA_CONCURRENCY_COND_INIT(&barrier->cond)) {
        ASTHRA_CONCURRENCY_MUTEX_DESTROY(&barrier->mutex);
        free(barrier);
        return NULL;
    }

    // Initialize atomic counters using the existing structure fields
    atomic_store(&barrier->party_count, party_count);
    atomic_store(&barrier->waiting_count, 0);
    atomic_store(&barrier->generation, 0);
    atomic_store(&barrier->is_broken, false);

    barrier->name = NULL;

    return barrier;
}

AsthraResult Asthra_barrier_wait(AsthraConcurrencyBarrier *barrier, bool *is_leader) {
    if (!barrier) {
        return asthra_result_err_cstr("Invalid barrier");
    }

    if (is_leader) {
        *is_leader = false;
    }

    if (!ASTHRA_CONCURRENCY_MUTEX_LOCK(&barrier->mutex)) {
        return asthra_result_err_cstr("Failed to lock barrier mutex");
    }

    // Check if barrier is broken
    if (atomic_load(&barrier->is_broken)) {
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&barrier->mutex);
        return asthra_result_err_cstr("Barrier is broken");
    }

    size_t party_count = atomic_load(&barrier->party_count);
    uint64_t current_generation = atomic_load(&barrier->generation);

    // Increment waiting count
    size_t current_waiting = asthra_concurrency_atomic_increment_size(&barrier->waiting_count);

    if (current_waiting == party_count) {
        // Last thread to arrive - become the leader
        if (is_leader) {
            *is_leader = true;
        }

        // Reset waiting count and increment generation
        atomic_store(&barrier->waiting_count, 0);
        asthra_concurrency_atomic_increment_counter(&barrier->generation);

        // Wake up all waiting threads
        ASTHRA_CONCURRENCY_COND_BROADCAST(&barrier->cond);
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&barrier->mutex);

        return asthra_result_ok_cstr("Barrier wait completed - leader");
    } else {
        // Wait for other threads
        while (atomic_load(&barrier->generation) == current_generation &&
               !atomic_load(&barrier->is_broken)) {
            if (!ASTHRA_CONCURRENCY_COND_WAIT(&barrier->cond, &barrier->mutex)) {
                ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&barrier->mutex);
                return asthra_result_err_cstr("Failed to wait on barrier condition");
            }
        }

        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&barrier->mutex);

        // Check if barrier was broken while waiting
        if (atomic_load(&barrier->is_broken)) {
            return asthra_result_err_cstr("Barrier was broken while waiting");
        }

        return asthra_result_ok_cstr("Barrier wait completed - follower");
    }
}

size_t Asthra_barrier_waiting_count(const AsthraConcurrencyBarrier *barrier) {
    if (!barrier) {
        return 0;
    }
    return atomic_load(&barrier->waiting_count);
}

AsthraResult Asthra_barrier_reset(AsthraConcurrencyBarrier *barrier) {
    if (!barrier) {
        return asthra_result_err_cstr("Invalid barrier");
    }

    if (!ASTHRA_CONCURRENCY_MUTEX_LOCK(&barrier->mutex)) {
        return asthra_result_err_cstr("Failed to lock barrier mutex");
    }

    // Mark barrier as broken to wake up waiting threads
    atomic_store(&barrier->is_broken, true);

    // Wake up all waiting threads
    ASTHRA_CONCURRENCY_COND_BROADCAST(&barrier->cond);

    // Reset state
    atomic_store(&barrier->waiting_count, 0);
    asthra_concurrency_atomic_increment_counter(&barrier->generation);
    atomic_store(&barrier->is_broken, false);

    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&barrier->mutex);
    return asthra_result_ok_cstr("Barrier reset completed");
}

void Asthra_barrier_destroy(AsthraConcurrencyBarrier *barrier) {
    if (!barrier) {
        return;
    }

    // Mark as broken to ensure no new waiters
    atomic_store(&barrier->is_broken, true);

    // Wake up any remaining waiters
    ASTHRA_CONCURRENCY_COND_BROADCAST(&barrier->cond);

    // Destroy synchronization primitives
    ASTHRA_CONCURRENCY_COND_DESTROY(&barrier->cond);
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&barrier->mutex);

    free(barrier);
}
