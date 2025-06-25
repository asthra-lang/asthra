/**
 * Asthra Programming Language Task System Threads v1.2
 * Thread Registration and GC Integration Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_TASKS_THREADS_H
#define ASTHRA_TASKS_THREADS_H

#include "asthra_tasks_types.h"

// =============================================================================
// THREAD REGISTRATION FOR GC
// =============================================================================

/**
 * Register current thread with GC system
 */
int asthra_thread_register(void);

/**
 * Unregister current thread from GC system
 */
void asthra_thread_unregister(void);

/**
 * Check if current thread is registered with GC system
 */
bool asthra_thread_is_registered(void);

// =============================================================================
// INTERNAL FUNCTIONS
// =============================================================================

/**
 * Initialize thread-local storage key (internal)
 */
void init_thread_key(void);

#endif // ASTHRA_TASKS_THREADS_H
