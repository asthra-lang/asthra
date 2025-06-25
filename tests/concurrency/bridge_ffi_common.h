/**
 * Asthra Concurrency Bridge FFI - Common Definitions
 * Shared structures and declarations for bridge FFI implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef BRIDGE_FFI_COMMON_H
#define BRIDGE_FFI_COMMON_H

#include "asthra_ffi_memory.h"
#include "bridge_test_common.h"
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// =============================================================================
// INTERNAL STRUCTURES
// =============================================================================

// Task registry entry
typedef struct TaskRegistryEntry {
    AsthraConcurrencyTaskHandle *handle;
    struct TaskRegistryEntry *next;
} TaskRegistryEntry;

// Callback queue entry
typedef struct CallbackEntry {
    AsthraConcurrencyCallbackFunction func;
    void *data;
    size_t data_size;
    void *context;
    uint32_t priority;
    struct timespec enqueue_time;
    struct CallbackEntry *next;
} CallbackEntry;

// Thread registry entry
typedef struct ThreadRegistryEntry {
    pthread_t thread_id;
    char name[64];
    bool is_registered;
    struct ThreadRegistryEntry *next;
} ThreadRegistryEntry;

// Bridge internal state
typedef struct {
    bool initialized;
    pthread_mutex_t mutex;

    // Configuration
    size_t task_pool_size;
    size_t queue_size;

    // Task management
    TaskRegistryEntry *task_registry;
    atomic_uint_fast64_t next_task_id;

    // Callback queue
    CallbackEntry *callback_queue_head;
    CallbackEntry *callback_queue_tail;
    size_t callback_queue_count;
    pthread_cond_t callback_available;

    // Thread registry
    ThreadRegistryEntry *thread_registry;

    // Statistics
    AsthraConcurrencyStats stats;

    // Synchronization objects registry
    struct {
        AsthraConcurrencyMutex **mutexes;
        size_t count;
        size_t capacity;
    } mutex_registry;

} BridgeState;

// =============================================================================
// GLOBAL STATE
// =============================================================================

extern BridgeState bridge_state;

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

static inline uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static inline AsthraResult create_error(const char *message) {
    AsthraResult result = {.tag = ASTHRA_RESULT_ERR,
                           .data.err = {.error = (void *)(message ? message : "Unknown error"),
                                        .error_size = message ? strlen(message) : 13,
                                        .error_type_id = 0}};
    return result;
}

static inline AsthraResult create_ok(void) {
    AsthraResult result = {.tag = ASTHRA_RESULT_OK};
    return result;
}

#endif // BRIDGE_FFI_COMMON_H