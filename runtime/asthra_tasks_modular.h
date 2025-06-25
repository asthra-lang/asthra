/**
 * Asthra Programming Language Task System Modular v1.2
 * Umbrella Header for Complete Task System API
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This header provides 100% backward compatibility with the original
 * asthra_tasks.c implementation while using the new modular architecture.
 */

#ifndef ASTHRA_TASKS_MODULAR_H
#define ASTHRA_TASKS_MODULAR_H

// =============================================================================
// MODULAR COMPONENTS
// =============================================================================

#include "asthra_tasks_callbacks.h"
#include "asthra_tasks_core.h"
#include "asthra_tasks_scheduler.h"
#include "asthra_tasks_sync.h"
#include "asthra_tasks_threads.h"
#include "asthra_tasks_types.h"

// =============================================================================
// COMPATIBILITY VERIFICATION
// =============================================================================

/*
 * This header ensures that all original asthra_tasks.c functionality
 * is available through the modular system:
 *
 * TASK MANAGEMENT:
 * - asthra_spawn_task()
 * - asthra_spawn_task_with_stack()
 * - asthra_task_await()
 * - asthra_task_is_completed()
 * - asthra_task_detach()
 * - asthra_task_yield()
 * - asthra_log_task_stats()
 *
 * SCHEDULER:
 * - asthra_scheduler_create()
 * - asthra_scheduler_destroy()
 * - asthra_scheduler_run()
 * - asthra_scheduler_stop()
 *
 * SYNCHRONIZATION:
 * - asthra_mutex_create/destroy/lock/trylock/unlock()
 * - asthra_condvar_create/destroy/wait/timedwait/signal/broadcast()
 * - asthra_rwlock_create/destroy/read_lock/write_lock/try_*_lock/*_unlock()
 *
 * CALLBACKS:
 * - asthra_callback_queue_push()
 * - asthra_callback_queue_process()
 *
 * THREADS:
 * - asthra_thread_register()
 * - asthra_thread_unregister()
 * - asthra_thread_is_registered()
 *
 * DATA STRUCTURES:
 * - AsthraTask, AsthraScheduler, AsthraMutex, AsthraCondVar, AsthraRWLock
 * - AsthraCallbackNode, AsthraCallbackQueue
 * - All atomic compatibility macros
 */

#endif // ASTHRA_TASKS_MODULAR_H
