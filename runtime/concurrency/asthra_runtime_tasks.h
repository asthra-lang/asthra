/**
 * Asthra Programming Language Runtime v1.2 - Concurrency Tasks Module
 * Task System and Async Operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides task management functionality including
 * task spawning, scheduling, and async operations.
 */

#ifndef ASTHRA_RUNTIME_TASKS_H
#define ASTHRA_RUNTIME_TASKS_H

#include "../core/asthra_runtime_core.h"
#include "../types/asthra_runtime_result.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TASK SYSTEM TYPE DEFINITIONS
// =============================================================================

typedef struct AsthraTask AsthraTask;
typedef struct AsthraTaskHandle AsthraTaskHandle;
typedef struct AsthraScheduler AsthraScheduler;

typedef enum {
    ASTHRA_TASK_CREATED,
    ASTHRA_TASK_RUNNING,
    ASTHRA_TASK_SUSPENDED,
    ASTHRA_TASK_COMPLETED,
    ASTHRA_TASK_FAILED
} AsthraTaskState;

typedef AsthraResult (*AsthraTaskFunction)(void *args);

struct AsthraTaskHandle {
    uint64_t task_id;
    AsthraTask *task;
    AsthraTaskState state;
    AsthraResult result;
    bool is_detached;
};

// =============================================================================
// TASK OPERATIONS
// =============================================================================

// Task operations
AsthraTaskHandle asthra_spawn_task(AsthraTaskFunction func, void *args, size_t args_size);
AsthraTaskHandle asthra_spawn_task_with_stack(AsthraTaskFunction func, void *args, size_t args_size,
                                              size_t stack_size);
AsthraResult asthra_task_await(AsthraTaskHandle handle);
bool asthra_task_is_completed(AsthraTaskHandle handle);
void asthra_task_detach(AsthraTaskHandle handle);
void asthra_task_yield(void);

// =============================================================================
// SCHEDULER OPERATIONS
// =============================================================================

// Scheduler operations
AsthraScheduler *asthra_scheduler_create(size_t max_tasks, size_t worker_threads);
void asthra_scheduler_destroy(AsthraScheduler *scheduler);
void asthra_scheduler_run(AsthraScheduler *scheduler);
void asthra_scheduler_stop(AsthraScheduler *scheduler);

// =============================================================================
// CALLBACK QUEUE SYSTEM
// =============================================================================

// Callback queue
typedef void (*AsthraCallback)(void *data);
void asthra_callback_queue_push(AsthraCallback callback, void *data, size_t data_size);
void asthra_callback_queue_process(void);

// =============================================================================
// THREAD REGISTRATION
// =============================================================================

// Thread registration
int asthra_thread_register(void);
void asthra_thread_unregister(void);
bool asthra_thread_is_registered(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_TASKS_H
