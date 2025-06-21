/**
 * Asthra Concurrency Thread Management - Condition Variable Module
 * Condition variable creation and signaling primitives
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CONCURRENCY_CONDVAR_H
#define ASTHRA_CONCURRENCY_CONDVAR_H

#include "asthra_concurrency_threads_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONDITION VARIABLE OPERATIONS API
// =============================================================================

/**
 * Create a new condition variable
 * @param name Optional name for debugging (can be NULL)
 * @return Condition variable handle or NULL on failure
 */
AsthraConcurrencyCondVar* Asthra_condvar_create(const char* name);

/**
 * Wait on a condition variable
 * @param condvar Condition variable to wait on
 * @param mutex Mutex to unlock while waiting
 * @return Success result or error details
 */
AsthraResult Asthra_condvar_wait(AsthraConcurrencyCondVar* condvar, AsthraConcurrencyMutex* mutex);

/**
 * Wait on a condition variable with timeout
 * @param condvar Condition variable to wait on
 * @param mutex Mutex to unlock while waiting
 * @param timeout_ms Timeout in milliseconds
 * @return Success result or timeout error
 */
AsthraResult Asthra_condvar_wait_timeout(AsthraConcurrencyCondVar* condvar, 
                                        AsthraConcurrencyMutex* mutex, 
                                        uint64_t timeout_ms);

/**
 * Signal one thread waiting on condition variable
 * @param condvar Condition variable to signal
 * @return Success result or error details
 */
AsthraResult Asthra_condvar_signal(AsthraConcurrencyCondVar* condvar);

/**
 * Signal all threads waiting on condition variable
 * @param condvar Condition variable to broadcast
 * @return Success result or error details
 */
AsthraResult Asthra_condvar_broadcast(AsthraConcurrencyCondVar* condvar);

/**
 * Destroy a condition variable and free resources
 * @param condvar Condition variable to destroy
 */
void Asthra_condvar_destroy(AsthraConcurrencyCondVar* condvar);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_CONDVAR_H 
