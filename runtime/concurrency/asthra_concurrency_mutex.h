/**
 * Asthra Concurrency Thread Management - Mutex Operations Module
 * Mutex creation, locking, and management primitives
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CONCURRENCY_MUTEX_H
#define ASTHRA_CONCURRENCY_MUTEX_H

#include "asthra_concurrency_threads_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MUTEX OPERATIONS API
// =============================================================================

/**
 * Create a new mutex
 * @param name Optional name for debugging (can be NULL)
 * @param recursive Whether to create a recursive mutex
 * @return Mutex handle or NULL on failure
 */
AsthraConcurrencyMutex* Asthra_mutex_create(const char* name, bool recursive);

/**
 * Lock a mutex (blocks if already locked)
 * @param mutex Mutex to lock
 * @return Success result or error details
 */
AsthraResult Asthra_mutex_lock(AsthraConcurrencyMutex* mutex);

/**
 * Try to lock a mutex (non-blocking)
 * @param mutex Mutex to try to lock
 * @return Success result or timeout error
 */
AsthraResult Asthra_mutex_trylock(AsthraConcurrencyMutex* mutex);

/**
 * Unlock a mutex
 * @param mutex Mutex to unlock
 * @return Success result or error details
 */
AsthraResult Asthra_mutex_unlock(AsthraConcurrencyMutex* mutex);

/**
 * Destroy a mutex and free resources
 * @param mutex Mutex to destroy
 */
void Asthra_mutex_destroy(AsthraConcurrencyMutex* mutex);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_MUTEX_H 
