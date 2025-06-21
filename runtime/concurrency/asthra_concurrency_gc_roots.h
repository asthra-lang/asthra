/**
 * Asthra Concurrency Thread Management - GC Root Management Module
 * Thread-local GC root registration and management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CONCURRENCY_GC_ROOTS_H
#define ASTHRA_CONCURRENCY_GC_ROOTS_H

#include "asthra_concurrency_threads_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// THREAD-LOCAL GC ROOT MANAGEMENT API
// =============================================================================

/**
 * Register a GC root with the current thread
 * @param ptr Pointer to register as GC root
 * @return Success result or error details
 */
AsthraResult Asthra_register_thread_gc_root(void *ptr);

/**
 * Unregister a GC root from the current thread
 * @param ptr Pointer to unregister as GC root
 * @return Success result or error details
 */
AsthraResult Asthra_unregister_thread_gc_root(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_GC_ROOTS_H 
