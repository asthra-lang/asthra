/**
 * Asthra Programming Language Task System Core v1.2
 * Core Task Management Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_TASKS_CORE_H
#define ASTHRA_TASKS_CORE_H

#include "asthra_tasks_types.h"

// =============================================================================
// TASK CREATION AND MANAGEMENT
// =============================================================================

/**
 * Spawn a new task with default stack size
 */
AsthraTaskHandle asthra_spawn_task(AsthraTaskFunction func, void *args, size_t args_size);

/**
 * Spawn a new task with specified stack size
 */
AsthraTaskHandle asthra_spawn_task_with_stack(AsthraTaskFunction func, void *args, size_t args_size,
                                              size_t stack_size);

/**
 * Wait for a task to complete and get its result
 */
AsthraResult asthra_task_await(AsthraTaskHandle handle);

/**
 * Check if a task has completed
 */
bool asthra_task_is_completed(AsthraTaskHandle handle);

/**
 * Detach a task (fire-and-forget)
 */
void asthra_task_detach(AsthraTaskHandle handle);

/**
 * Yield execution to other tasks
 */
void asthra_task_yield(void);

/**
 * Log task system statistics
 */
void asthra_log_task_stats(void);

// =============================================================================
// INTERNAL FUNCTIONS
// =============================================================================

/**
 * Internal task runner function
 */
void *task_runner(void *arg);

#endif // ASTHRA_TASKS_CORE_H
