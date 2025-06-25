/**
 * Asthra Programming Language Task System Scheduler v1.2
 * Task Scheduling and Management Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "asthra_tasks_scheduler.h"

// =============================================================================
// SCHEDULER IMPLEMENTATION
// =============================================================================

AsthraScheduler *asthra_scheduler_create(size_t max_tasks, size_t worker_threads) {
    AsthraScheduler *scheduler = asthra_alloc(sizeof(AsthraScheduler), ASTHRA_ZONE_GC);
    if (!scheduler)
        return NULL;

    scheduler->max_tasks = max_tasks;
    scheduler->worker_threads = worker_threads;
#if ASTHRA_HAS_ATOMICS
    atomic_init(&scheduler->task_count, 0);
    atomic_init(&scheduler->running, false);
#else
    scheduler->task_count = 0;
    scheduler->running = false;
#endif

    scheduler->tasks = asthra_alloc(max_tasks * sizeof(AsthraTask *), ASTHRA_ZONE_GC);
    if (!scheduler->tasks) {
        asthra_free(scheduler, ASTHRA_ZONE_GC);
        return NULL;
    }

    if (pthread_mutex_init(&scheduler->mutex, NULL) != 0) {
        asthra_free(scheduler->tasks, ASTHRA_ZONE_GC);
        asthra_free(scheduler, ASTHRA_ZONE_GC);
        return NULL;
    }

    if (pthread_cond_init(&scheduler->cond, NULL) != 0) {
        pthread_mutex_destroy(&scheduler->mutex);
        asthra_free(scheduler->tasks, ASTHRA_ZONE_GC);
        asthra_free(scheduler, ASTHRA_ZONE_GC);
        return NULL;
    }

    return scheduler;
}

void asthra_scheduler_destroy(AsthraScheduler *scheduler) {
    if (!scheduler)
        return;

    // Stop the scheduler if running
    asthra_scheduler_stop(scheduler);

    // Cleanup
    pthread_mutex_destroy(&scheduler->mutex);
    pthread_cond_destroy(&scheduler->cond);
    asthra_free(scheduler->tasks, ASTHRA_ZONE_GC);
    asthra_free(scheduler, ASTHRA_ZONE_GC);
}

void asthra_scheduler_run(AsthraScheduler *scheduler) {
    if (!scheduler)
        return;

    pthread_mutex_lock(&scheduler->mutex);
    ASTHRA_ATOMIC_STORE(&scheduler->running, true);
    pthread_mutex_unlock(&scheduler->mutex);

    // Main scheduler loop
    while (ASTHRA_ATOMIC_LOAD(&scheduler->running)) {
        pthread_mutex_lock(&scheduler->mutex);

        // Check if there are tasks in the queue
        size_t task_count = ASTHRA_ATOMIC_LOAD(&scheduler->task_count);
        if (task_count == 0) {
            // Wait for tasks or stop signal
            pthread_cond_wait(&scheduler->cond, &scheduler->mutex);
            pthread_mutex_unlock(&scheduler->mutex);
            continue;
        }

        // Find a task to execute
        AsthraTask *task = NULL;
        for (size_t i = 0; i < scheduler->max_tasks; i++) {
            if (scheduler->tasks && scheduler->tasks[i] &&
                scheduler->tasks[i]->state == ASTHRA_TASK_CREATED) {
                task = scheduler->tasks[i];
                task->state = ASTHRA_TASK_RUNNING;
                break;
            }
        }

        pthread_mutex_unlock(&scheduler->mutex);

        // Execute the task if we got one
        if (task) {
            // Execute the task function
            if (task->func) {
                task->result = task->func(task->args);
            }

            // Set task status to completed
            task->state = ASTHRA_TASK_COMPLETED;

            // Decrement task count
            ASTHRA_ATOMIC_FETCH_SUB(&scheduler->task_count, 1);
        }
    }
}

void asthra_scheduler_stop(AsthraScheduler *scheduler) {
    if (!scheduler)
        return;

    pthread_mutex_lock(&scheduler->mutex);
    ASTHRA_ATOMIC_STORE(&scheduler->running, false);
    pthread_cond_broadcast(&scheduler->cond);
    pthread_mutex_unlock(&scheduler->mutex);
}
