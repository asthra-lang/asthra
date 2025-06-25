/**
 * Asthra Programming Language Task System Scheduler v1.2
 * Task Scheduling and Management Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_TASKS_SCHEDULER_H
#define ASTHRA_TASKS_SCHEDULER_H

#include "asthra_tasks_types.h"

// =============================================================================
// SCHEDULER OPERATIONS
// =============================================================================

/**
 * Create a new task scheduler
 */
AsthraScheduler *asthra_scheduler_create(size_t max_tasks, size_t worker_threads);

/**
 * Destroy a task scheduler
 */
void asthra_scheduler_destroy(AsthraScheduler *scheduler);

/**
 * Start the scheduler
 */
void asthra_scheduler_run(AsthraScheduler *scheduler);

/**
 * Stop the scheduler
 */
void asthra_scheduler_stop(AsthraScheduler *scheduler);

#endif // ASTHRA_TASKS_SCHEDULER_H
