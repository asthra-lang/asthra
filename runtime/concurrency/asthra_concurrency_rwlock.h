/**
 * Asthra Concurrency Thread Management - Read-Write Lock Module
 * Read-write lock creation and locking primitives
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CONCURRENCY_RWLOCK_H
#define ASTHRA_CONCURRENCY_RWLOCK_H

#include "asthra_concurrency_threads_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// READ-WRITE LOCK OPERATIONS API
// =============================================================================

/**
 * Create a new read-write lock
 * @param name Optional name for debugging (can be NULL)
 * @return Read-write lock handle or NULL on failure
 */
AsthraConcurrencyRWLock* Asthra_rwlock_create(const char* name);

/**
 * Acquire a read lock (blocks if write lock is held)
 * @param rwlock Read-write lock to acquire
 * @return Success result or error details
 */
AsthraResult Asthra_rwlock_read_lock(AsthraConcurrencyRWLock* rwlock);

/**
 * Acquire a write lock (blocks if any locks are held)
 * @param rwlock Read-write lock to acquire
 * @return Success result or error details
 */
AsthraResult Asthra_rwlock_write_lock(AsthraConcurrencyRWLock* rwlock);

/**
 * Try to acquire a read lock (non-blocking)
 * @param rwlock Read-write lock to try to acquire
 * @return Success result or timeout error
 */
AsthraResult Asthra_rwlock_try_read_lock(AsthraConcurrencyRWLock* rwlock);

/**
 * Try to acquire a write lock (non-blocking)
 * @param rwlock Read-write lock to try to acquire
 * @return Success result or timeout error
 */
AsthraResult Asthra_rwlock_try_write_lock(AsthraConcurrencyRWLock* rwlock);

/**
 * Release a read or write lock
 * @param rwlock Read-write lock to release
 * @return Success result or error details
 */
AsthraResult Asthra_rwlock_unlock(AsthraConcurrencyRWLock* rwlock);

/**
 * Destroy a read-write lock and free resources
 * @param rwlock Read-write lock to destroy
 */
void Asthra_rwlock_destroy(AsthraConcurrencyRWLock* rwlock);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_RWLOCK_H 
