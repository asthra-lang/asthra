/**
 * Asthra Programming Language Task System Core v1.2
 * Core Task Management Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <assert.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "asthra_tasks_core.h"

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

asthra_atomic_counter_t g_next_task_id = 1;

// =============================================================================
// TASK CREATION AND MANAGEMENT
// =============================================================================

void *task_runner(void *arg) {
    AsthraTask *task = (AsthraTask *)arg;
    if (!task)
        return NULL;

    // Update task state
    task->state = ASTHRA_TASK_RUNNING;

    // Execute the task function
    task->result = task->func(task->args);

    // Update task state based on result
    if (asthra_result_is_ok(task->result)) {
        task->state = ASTHRA_TASK_COMPLETED;
    } else {
        task->state = ASTHRA_TASK_FAILED;
    }

    // Update global statistics
    asthra_atomic_stats_update_task_completed();

    return NULL;
}

AsthraTaskHandle asthra_spawn_task(AsthraTaskFunction func, void *args, size_t args_size) {
    return asthra_spawn_task_with_stack(func, args, args_size, 0); // Default stack size
}

AsthraTaskHandle asthra_spawn_task_with_stack(AsthraTaskFunction func, void *args, size_t args_size,
                                              size_t stack_size) {
    // Suppress unused parameter warning for stack_size (future enhancement)
    (void)stack_size;

    AsthraTask *task = asthra_alloc(sizeof(AsthraTask), ASTHRA_ZONE_GC);
    if (!task) {
        AsthraTaskHandle handle = {0};
        return handle;
    }

    // Allocate and copy arguments if provided
    void *task_args = NULL;
    if (args && args_size > 0) {
        task_args = asthra_alloc(args_size, ASTHRA_ZONE_GC);
        if (!task_args) {
            asthra_free(task, ASTHRA_ZONE_GC);
            AsthraTaskHandle handle = {0};
            return handle;
        }
        memcpy(task_args, args, args_size);
    }

    // Initialize task
    task->id = ASTHRA_ATOMIC_FETCH_ADD(&g_next_task_id, 1);
    task->func = func;
    task->args = task_args;
    task->args_size = args_size;
    task->result = asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    task->state = ASTHRA_TASK_CREATED;
    task->detached = false;

    // Create thread
    if (pthread_create(&task->thread, NULL, task_runner, task) != 0) {
        if (task_args)
            asthra_free(task_args, ASTHRA_ZONE_GC);
        asthra_free(task, ASTHRA_ZONE_GC);
        AsthraTaskHandle handle = {0};
        return handle;
    }

    // Update global statistics
    asthra_atomic_stats_update_task_spawned();

    // Create task handle
    AsthraTaskHandle handle = {.task_id = task->id,
                               .task = task,
                               .state = task->state,
                               .result = task->result,
                               .is_detached = false};

    return handle;
}

AsthraResult asthra_task_await(AsthraTaskHandle handle) {
    if (!handle.task || handle.is_detached) {
        return asthra_result_err_cstr("Task handle is invalid or detached");
    }

    AsthraTask *task = handle.task;

    // Wait for thread to complete
    if (pthread_join(task->thread, NULL) != 0) {
        return asthra_result_err_cstr("Failed to join task thread");
    }

    AsthraResult result = task->result;

    // Cleanup task resources
    if (task->args)
        asthra_free(task->args, ASTHRA_ZONE_GC);
    asthra_free(task, ASTHRA_ZONE_GC);

    return result;
}

bool asthra_task_is_completed(AsthraTaskHandle handle) {
    if (!handle.task)
        return false;

    AsthraTask *task = handle.task;
    return (task->state == ASTHRA_TASK_COMPLETED || task->state == ASTHRA_TASK_FAILED);
}

void asthra_task_detach(AsthraTaskHandle handle) {
    if (!handle.task)
        return;

    AsthraTask *task = handle.task;
    task->detached = true;
    pthread_detach(task->thread);
}

void asthra_task_yield(void) {
#if defined(__linux__) || defined(__APPLE__) || defined(__BSD__)
    sched_yield();
#else
    // Fallback - sleep for a very short time
    struct timespec ts = {0, 1000}; // 1 microsecond
    nanosleep(&ts, NULL);
#endif
}

void asthra_log_task_stats(void) {
    AsthraRuntimeStats stats = asthra_get_runtime_stats();
    asthra_log(ASTHRA_LOG_INFO, ASTHRA_LOG_CATEGORY_CONCURRENCY,
               "Task Stats - Spawned: %zu, Completed: %zu", (size_t)stats.tasks_spawned,
               (size_t)stats.tasks_completed);
}
