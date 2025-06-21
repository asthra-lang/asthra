/**
 * Asthra Concurrency Thread Management - Thread Registry Module
 * Thread registration and management for GC integration
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CONCURRENCY_THREAD_REGISTRY_H
#define ASTHRA_CONCURRENCY_THREAD_REGISTRY_H

#include "asthra_concurrency_threads_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// THREAD REGISTRATION API
// =============================================================================

/**
 * Register current C thread with Asthra runtime
 * @return Success result or error details
 */
AsthraResult Asthra_register_c_thread(void);

/**
 * Unregister current C thread from Asthra runtime
 * Cleanup thread-local data and remove from registry
 */
void Asthra_unregister_c_thread(void);

/**
 * Check if current thread is registered
 * @return true if thread is registered, false otherwise
 */
bool Asthra_thread_is_registered(void);

/**
 * Get thread registry statistics
 * @return Current thread registry statistics
 */
AsthraConcurrencyThreadRegistryStats Asthra_get_thread_registry_stats(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_THREAD_REGISTRY_H 
