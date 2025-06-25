/**
 * Asthra Programming Language Task System Callbacks v1.2
 * Callback Queue Management Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_TASKS_CALLBACKS_H
#define ASTHRA_TASKS_CALLBACKS_H

#include "asthra_tasks_types.h"

// =============================================================================
// CALLBACK QUEUE OPERATIONS
// =============================================================================

/**
 * Push a callback to the queue
 */
void asthra_callback_queue_push(AsthraCallback callback, void *data, size_t data_size);

/**
 * Process all callbacks in the queue
 */
void asthra_callback_queue_process(void);

// =============================================================================
// INTERNAL FUNCTIONS
// =============================================================================

/**
 * Initialize the callback queue (internal)
 */
void init_callback_queue(void);

#endif // ASTHRA_TASKS_CALLBACKS_H
