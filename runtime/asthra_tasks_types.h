/**
 * Asthra Programming Language Task System Types v1.2
 * Core Data Structures and Type Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_TASKS_TYPES_H
#define ASTHRA_TASKS_TYPES_H

// Define feature test macros for POSIX threads
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// C17 modernization includes
#include <stdalign.h>
#include <stdatomic.h>

#include "asthra_runtime.h"

// =============================================================================
// ATOMIC COMPATIBILITY LAYER
// =============================================================================

// Import atomic compatibility layer from core
#if ASTHRA_HAS_ATOMICS
#define ASTHRA_ATOMIC_LOAD(ptr) atomic_load(ptr)
#define ASTHRA_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#define ASTHRA_ATOMIC_FETCH_ADD(ptr, val) atomic_fetch_add(ptr, val)
#define ASTHRA_ATOMIC_FETCH_SUB(ptr, val) atomic_fetch_sub(ptr, val)
#else
#define ASTHRA_ATOMIC_LOAD(ptr) (*(ptr))
#define ASTHRA_ATOMIC_STORE(ptr, val) (*(ptr) = (val))
#define ASTHRA_ATOMIC_FETCH_ADD(ptr, val) (__sync_fetch_and_add(ptr, val))
#define ASTHRA_ATOMIC_FETCH_SUB(ptr, val) (__sync_fetch_and_sub(ptr, val))
#endif

// =============================================================================
// CORE DATA STRUCTURES
// =============================================================================

/**
 * Task structure containing all task-related information
 */
struct AsthraTask {
    uint64_t id;
    AsthraTaskFunction func;
    void *args;
    size_t args_size;
    AsthraResult result;
    AsthraTaskState state;
    pthread_t thread;
    bool detached;
};

/**
 * Scheduler structure for task management
 */
struct AsthraScheduler {
    AsthraTask **tasks;
    size_t max_tasks;
    asthra_atomic_size_t task_count; // C17 atomic counter
    size_t worker_threads;
    asthra_atomic_bool running; // C17 atomic flag
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

/**
 * Mutex wrapper structure
 */
struct AsthraMutex {
    pthread_mutex_t mutex;
};

/**
 * Condition variable wrapper structure
 */
struct AsthraCondVar {
    pthread_cond_t cond;
};

/**
 * Read-write lock wrapper structure
 */
struct AsthraRWLock {
    pthread_rwlock_t rwlock;
};

/**
 * Callback queue node structure
 */
typedef struct AsthraCallbackNode {
    AsthraCallback callback;
    void *data;
    size_t data_size;
    struct AsthraCallbackNode *next;
} AsthraCallbackNode;

/**
 * Global callback queue structure
 */
typedef struct {
    AsthraCallbackNode *head;
    AsthraCallbackNode *tail;
    pthread_mutex_t mutex;
    asthra_atomic_size_t queue_size;
} AsthraCallbackQueue;

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

extern asthra_atomic_counter_t g_next_task_id;
extern AsthraCallbackQueue g_callback_queue;
extern bool g_callback_queue_initialized;
extern pthread_key_t g_thread_registered_key;
extern bool g_thread_key_initialized;

#endif // ASTHRA_TASKS_TYPES_H
