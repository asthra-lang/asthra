/**
 * Asthra Programming Language Task System Synchronization v1.2
 * Synchronization Primitives Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "asthra_tasks_sync.h"

// =============================================================================
// MUTEX OPERATIONS
// =============================================================================

AsthraMutex *asthra_mutex_create(void) {
    AsthraMutex *mutex = asthra_alloc(sizeof(AsthraMutex), ASTHRA_ZONE_GC);
    if (!mutex)
        return NULL;

    if (pthread_mutex_init(&mutex->mutex, NULL) != 0) {
        asthra_free(mutex, ASTHRA_ZONE_GC);
        return NULL;
    }

    return mutex;
}

void asthra_mutex_destroy(AsthraMutex *mutex) {
    if (mutex) {
        pthread_mutex_destroy(&mutex->mutex);
        asthra_free(mutex, ASTHRA_ZONE_GC);
    }
}

void asthra_mutex_lock(AsthraMutex *mutex) {
    if (mutex)
        pthread_mutex_lock(&mutex->mutex);
}

bool asthra_mutex_trylock(AsthraMutex *mutex) {
    if (!mutex)
        return false;
    return pthread_mutex_trylock(&mutex->mutex) == 0;
}

void asthra_mutex_unlock(AsthraMutex *mutex) {
    if (mutex)
        pthread_mutex_unlock(&mutex->mutex);
}

// =============================================================================
// CONDITION VARIABLE OPERATIONS
// =============================================================================

AsthraCondVar *asthra_condvar_create(void) {
    AsthraCondVar *condvar = asthra_alloc(sizeof(AsthraCondVar), ASTHRA_ZONE_GC);
    if (!condvar)
        return NULL;

    if (pthread_cond_init(&condvar->cond, NULL) != 0) {
        asthra_free(condvar, ASTHRA_ZONE_GC);
        return NULL;
    }

    return condvar;
}

void asthra_condvar_destroy(AsthraCondVar *condvar) {
    if (condvar) {
        pthread_cond_destroy(&condvar->cond);
        asthra_free(condvar, ASTHRA_ZONE_GC);
    }
}

void asthra_condvar_wait(AsthraCondVar *condvar, AsthraMutex *mutex) {
    if (condvar && mutex) {
        pthread_cond_wait(&condvar->cond, &mutex->mutex);
    }
}

bool asthra_condvar_timedwait(AsthraCondVar *condvar, AsthraMutex *mutex, uint64_t timeout_ms) {
    if (!condvar || !mutex)
        return false;

    struct timespec timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_nsec = (timeout_ms % 1000) * 1000000;

    return pthread_cond_timedwait(&condvar->cond, &mutex->mutex, &timeout) == 0;
}

void asthra_condvar_signal(AsthraCondVar *condvar) {
    if (condvar)
        pthread_cond_signal(&condvar->cond);
}

void asthra_condvar_broadcast(AsthraCondVar *condvar) {
    if (condvar)
        pthread_cond_broadcast(&condvar->cond);
}

// =============================================================================
// READ-WRITE LOCK OPERATIONS
// =============================================================================

AsthraRWLock *asthra_rwlock_create(void) {
    AsthraRWLock *rwlock = asthra_alloc(sizeof(AsthraRWLock), ASTHRA_ZONE_GC);
    if (!rwlock)
        return NULL;

    if (pthread_rwlock_init(&rwlock->rwlock, NULL) != 0) {
        asthra_free(rwlock, ASTHRA_ZONE_GC);
        return NULL;
    }

    return rwlock;
}

void asthra_rwlock_destroy(AsthraRWLock *rwlock) {
    if (rwlock) {
        pthread_rwlock_destroy(&rwlock->rwlock);
        asthra_free(rwlock, ASTHRA_ZONE_GC);
    }
}

void asthra_rwlock_read_lock(AsthraRWLock *rwlock) {
    if (rwlock)
        pthread_rwlock_rdlock(&rwlock->rwlock);
}

void asthra_rwlock_write_lock(AsthraRWLock *rwlock) {
    if (rwlock)
        pthread_rwlock_wrlock(&rwlock->rwlock);
}

bool asthra_rwlock_try_read_lock(AsthraRWLock *rwlock) {
    if (!rwlock)
        return false;
    return pthread_rwlock_tryrdlock(&rwlock->rwlock) == 0;
}

bool asthra_rwlock_try_write_lock(AsthraRWLock *rwlock) {
    if (!rwlock)
        return false;
    return pthread_rwlock_trywrlock(&rwlock->rwlock) == 0;
}

void asthra_rwlock_read_unlock(AsthraRWLock *rwlock) {
    if (rwlock)
        pthread_rwlock_unlock(&rwlock->rwlock);
}

void asthra_rwlock_write_unlock(AsthraRWLock *rwlock) {
    if (rwlock)
        pthread_rwlock_unlock(&rwlock->rwlock);
}
