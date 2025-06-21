/**
 * Asthra Concurrency Thread Management - Condition Variable Implementation
 * Condition variable creation and signaling primitives
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_concurrency_condvar.h"

// =============================================================================
// CONDITION VARIABLE IMPLEMENTATION
// =============================================================================

AsthraConcurrencyCondVar* Asthra_condvar_create(const char* name) {
    AsthraConcurrencyCondVar *condvar = malloc(sizeof(AsthraConcurrencyCondVar));
    if (!condvar) {
        return NULL;
    }
    
    if (pthread_cond_init(&condvar->cond, NULL) != 0) {
        free(condvar);
        return NULL;
    }
    
    // Initialize metadata
    condvar->name = name ? strdup(name) : NULL;
    asthra_concurrency_atomic_store_counter(&condvar->signal_count, 0);
    asthra_concurrency_atomic_store_counter(&condvar->broadcast_count, 0);
    atomic_store_explicit(&condvar->waiter_count, 0, ASTHRA_MEMORY_ORDER_RELAXED);
    
    return condvar;
}

AsthraResult Asthra_condvar_wait(AsthraConcurrencyCondVar* condvar, AsthraConcurrencyMutex* mutex) {
    if (!condvar || !mutex) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE, 
                                                     "Invalid condition variable or mutex handle");
    }
    
    atomic_fetch_add_explicit(&condvar->waiter_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    
    int result = pthread_cond_wait(&condvar->cond, &mutex->mutex);
    atomic_fetch_sub_explicit(&condvar->waiter_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    if (result == 0) {
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR, 
                                                     "Condition variable wait failed");
    }
}

AsthraResult Asthra_condvar_wait_timeout(AsthraConcurrencyCondVar* condvar, 
                                        AsthraConcurrencyMutex* mutex, 
                                        uint64_t timeout_ms) {
    if (!condvar || !mutex) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE, 
                                                     "Invalid condition variable or mutex handle");
    }
    
    struct timespec timeout;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    
    timeout.tv_sec = now.tv_sec + (timeout_ms / 1000);
    timeout.tv_nsec = now.tv_nsec + ((timeout_ms % 1000) * 1000000);
    
    if (timeout.tv_nsec >= 1000000000) {
        timeout.tv_sec += 1;
        timeout.tv_nsec -= 1000000000;
    }
    
    atomic_fetch_add_explicit(&condvar->waiter_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    
    int result = pthread_cond_timedwait(&condvar->cond, &mutex->mutex, &timeout);
    atomic_fetch_sub_explicit(&condvar->waiter_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    if (result == 0) {
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else if (result == ETIMEDOUT) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_MUTEX_TIMEOUT, 
                                                     "Condition variable wait timed out");
    } else {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR, 
                                                     "Condition variable timed wait failed");
    }
}

AsthraResult Asthra_condvar_signal(AsthraConcurrencyCondVar* condvar) {
    if (!condvar) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE, 
                                                     "Invalid condition variable handle");
    }
    
    asthra_concurrency_atomic_increment_counter(&condvar->signal_count);
    
    int result = pthread_cond_signal(&condvar->cond);
    if (result == 0) {
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR, 
                                                     "Condition variable signal failed");
    }
}

AsthraResult Asthra_condvar_broadcast(AsthraConcurrencyCondVar* condvar) {
    if (!condvar) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE, 
                                                     "Invalid condition variable handle");
    }
    
    asthra_concurrency_atomic_increment_counter(&condvar->signal_count);
    
    int result = pthread_cond_broadcast(&condvar->cond);
    if (result == 0) {
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    } else {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR, 
                                                     "Condition variable broadcast failed");
    }
}

void Asthra_condvar_destroy(AsthraConcurrencyCondVar* condvar) {
    if (!condvar) {
        return;
    }
    
    pthread_cond_destroy(&condvar->cond);
    
    if (condvar->name) {
        char* mutable_name = (char*)condvar->name;
        free(mutable_name);
    }
    
    free(condvar);
} 
