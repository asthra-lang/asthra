/**
 * Asthra Programming Language Task System Synchronization v1.2
 * Synchronization Primitives Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_TASKS_SYNC_H
#define ASTHRA_TASKS_SYNC_H

#include "asthra_tasks_types.h"

// =============================================================================
// MUTEX OPERATIONS
// =============================================================================

/**
 * Create a new mutex
 */
AsthraMutex *asthra_mutex_create(void);

/**
 * Destroy a mutex
 */
void asthra_mutex_destroy(AsthraMutex *mutex);

/**
 * Lock a mutex
 */
void asthra_mutex_lock(AsthraMutex *mutex);

/**
 * Try to lock a mutex (non-blocking)
 */
bool asthra_mutex_trylock(AsthraMutex *mutex);

/**
 * Unlock a mutex
 */
void asthra_mutex_unlock(AsthraMutex *mutex);

// =============================================================================
// CONDITION VARIABLE OPERATIONS
// =============================================================================

/**
 * Create a new condition variable
 */
AsthraCondVar *asthra_condvar_create(void);

/**
 * Destroy a condition variable
 */
void asthra_condvar_destroy(AsthraCondVar *condvar);

/**
 * Wait on a condition variable
 */
void asthra_condvar_wait(AsthraCondVar *condvar, AsthraMutex *mutex);

/**
 * Wait on a condition variable with timeout
 */
bool asthra_condvar_timedwait(AsthraCondVar *condvar, AsthraMutex *mutex, uint64_t timeout_ms);

/**
 * Signal a condition variable
 */
void asthra_condvar_signal(AsthraCondVar *condvar);

/**
 * Broadcast to all waiters on a condition variable
 */
void asthra_condvar_broadcast(AsthraCondVar *condvar);

// =============================================================================
// READ-WRITE LOCK OPERATIONS
// =============================================================================

/**
 * Create a new read-write lock
 */
AsthraRWLock *asthra_rwlock_create(void);

/**
 * Destroy a read-write lock
 */
void asthra_rwlock_destroy(AsthraRWLock *rwlock);

/**
 * Acquire read lock
 */
void asthra_rwlock_read_lock(AsthraRWLock *rwlock);

/**
 * Acquire write lock
 */
void asthra_rwlock_write_lock(AsthraRWLock *rwlock);

/**
 * Try to acquire read lock (non-blocking)
 */
bool asthra_rwlock_try_read_lock(AsthraRWLock *rwlock);

/**
 * Try to acquire write lock (non-blocking)
 */
bool asthra_rwlock_try_write_lock(AsthraRWLock *rwlock);

/**
 * Release read lock
 */
void asthra_rwlock_read_unlock(AsthraRWLock *rwlock);

/**
 * Release write lock
 */
void asthra_rwlock_write_unlock(AsthraRWLock *rwlock);

#endif // ASTHRA_TASKS_SYNC_H 
