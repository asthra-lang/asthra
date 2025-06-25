/**
 * Asthra Concurrency Task Management v1.2
 * Task Spawning, Execution, and Lifecycle Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * IMPLEMENTATION FEATURES:
 * - Task spawning with C17 threading support
 * - Task lifecycle management (creation, execution, completion)
 * - Task result handling and await functionality
 * - Thread-safe task registry with atomic operations
 */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// C17 modernization includes
#include <stdatomic.h>

#include "asthra_concurrency_bridge_modular.h"

// Forward declarations for internal functions
extern AsthraConcurrencyBridge *asthra_concurrency_get_bridge(void);
extern bool asthra_concurrency_is_initialized(void);
extern AsthraResult asthra_concurrency_create_error_result(AsthraConcurrencyErrorCode code,
                                                           const char *message);
extern uint64_t asthra_concurrency_get_timestamp_ms(void);
extern bool
asthra_concurrency_create_thread_hybrid(asthra_concurrency_thread_t *thread, void *(*func)(void *),
                                        void *arg,
                                        const AsthraConcurrencyTaskSpawnOptions *options);
extern uint64_t
asthra_concurrency_atomic_increment_counter_explicit(asthra_concurrency_atomic_counter_t *counter,
                                                     memory_order order);
extern uint64_t
asthra_concurrency_atomic_increment_counter(asthra_concurrency_atomic_counter_t *counter);

// =============================================================================
// TASK THREAD WRAPPER IMPLEMENTATION
// =============================================================================

static void *task_thread_wrapper(void *arg) {
    AsthraConcurrencyTaskHandle *handle = (AsthraConcurrencyTaskHandle *)arg;
    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();

    // Register thread with GC
    Asthra_register_c_thread();

    // Record start time using atomic operations with explicit memory ordering
    uint64_t start_time = asthra_concurrency_get_timestamp_ms();
    atomic_store_explicit(&handle->creation_time, start_time, ASTHRA_MEMORY_ORDER_RELEASE);

    // Update task state using C17 atomic operations with proper memory ordering
    atomic_store_explicit(&handle->enhanced_state, ASTHRA_TASK_RUNNING,
                          ASTHRA_MEMORY_ORDER_RELEASE);

    // Execute task function
    AsthraResult result;
    if (handle->task_data) {
        // Get the actual task function and arguments
        AsthraConcurrencyTaskFunction func;
        memcpy(&func, handle->task_data, sizeof(AsthraConcurrencyTaskFunction));

        // Get arguments (if any)
        void *args = NULL;
        size_t args_size = handle->task_data_size - sizeof(AsthraConcurrencyTaskFunction);
        if (args_size > 0) {
            args = (char *)handle->task_data + sizeof(AsthraConcurrencyTaskFunction);
        }

        result = func(args, args_size);
    } else {
        int error_code = ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE;
        result = asthra_result_err(&error_code, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_GC);
    }

    // Record completion time
    uint64_t completion_time = asthra_concurrency_get_timestamp_ms();
    atomic_store_explicit(&handle->completion_time, completion_time, ASTHRA_MEMORY_ORDER_RELEASE);

    // Store result safely with hybrid mutex
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&handle->result_mutex);
    handle->base_handle.result = result;

    // Store result pointer atomically for lock-free access
    atomic_store_explicit(&handle->result_ptr, (uintptr_t)&handle->base_handle.result,
                          ASTHRA_MEMORY_ORDER_RELEASE);

    // Use C17 atomic operations for completion state with proper memory ordering
    atomic_store_explicit(&handle->is_complete, true, ASTHRA_MEMORY_ORDER_RELEASE);
    atomic_store_explicit(&handle->enhanced_state,
                          asthra_result_is_ok(result) ? ASTHRA_TASK_COMPLETED : ASTHRA_TASK_FAILED,
                          ASTHRA_MEMORY_ORDER_RELEASE);

    ASTHRA_CONCURRENCY_COND_BROADCAST(&handle->completion_cond);
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&handle->result_mutex);

    // Update statistics using C17 atomic operations with relaxed ordering for performance
    asthra_concurrency_atomic_increment_counter_explicit(&bridge->stats.tasks_completed,
                                                         ASTHRA_MEMORY_ORDER_RELAXED);

    // Unregister thread
    Asthra_unregister_c_thread();

    return NULL;
}

// =============================================================================
// TASK SPAWNING AND MANAGEMENT
// =============================================================================

AsthraConcurrencyTaskHandle *Asthra_spawn_task(AsthraConcurrencyTaskFunction func, void *args,
                                               size_t args_size,
                                               const AsthraConcurrencyTaskSpawnOptions *options) {
    if (!asthra_concurrency_is_initialized()) {
        return NULL;
    }

    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();

    // Allocate task handle
    AsthraConcurrencyTaskHandle *handle = malloc(sizeof(AsthraConcurrencyTaskHandle));
    if (!handle) {
        return NULL;
    }

    // Initialize handle with atomic operations
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&bridge->global_mutex);
    handle->base_handle.task_id = asthra_concurrency_atomic_increment_counter_explicit(
        &bridge->next_task_id, ASTHRA_MEMORY_ORDER_RELAXED);
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_mutex);

    handle->base_handle.task = NULL; // Will be set by task system
    handle->base_handle.state = ASTHRA_TASK_CREATED;
    handle->base_handle.result = asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    handle->base_handle.is_detached = options && options->detached;

    // Initialize C17 atomic fields with proper initialization
    atomic_init(&handle->enhanced_state, ASTHRA_TASK_CREATED);
    atomic_init(&handle->is_complete, false);
    atomic_init(&handle->is_detached, options && options->detached);
    atomic_init(&handle->result_ptr, 0);
    atomic_init(&handle->creation_time, 0);
    atomic_init(&handle->completion_time, 0);

    // Initialize synchronization primitives
    if (!ASTHRA_CONCURRENCY_MUTEX_INIT(&handle->result_mutex) ||
        !ASTHRA_CONCURRENCY_COND_INIT(&handle->completion_cond)) {
        free(handle);
        return NULL;
    }

    // Store task function and arguments together
    size_t total_size = sizeof(AsthraConcurrencyTaskFunction) + args_size;
    handle->task_data = malloc(total_size);
    if (!handle->task_data) {
        ASTHRA_CONCURRENCY_MUTEX_DESTROY(&handle->result_mutex);
        ASTHRA_CONCURRENCY_COND_DESTROY(&handle->completion_cond);
        free(handle);
        return NULL;
    }

    // Copy function pointer and arguments
    memcpy(handle->task_data, &func, sizeof(AsthraConcurrencyTaskFunction));
    if (args && args_size > 0) {
        memcpy((char *)handle->task_data + sizeof(AsthraConcurrencyTaskFunction), args, args_size);
    }
    handle->task_data_size = total_size;

    // Add to registry using atomic operations
    AsthraConcurrencyTaskHandle *old_head =
        atomic_load_explicit(&bridge->task_registry, ASTHRA_MEMORY_ORDER_ACQUIRE);
    do {
        handle->next = old_head;
    } while (!atomic_compare_exchange_weak_explicit(&bridge->task_registry, &old_head, handle,
                                                    ASTHRA_MEMORY_ORDER_RELEASE,
                                                    ASTHRA_MEMORY_ORDER_ACQUIRE));

    // Create thread using hybrid model
    asthra_concurrency_thread_t thread;
    if (!asthra_concurrency_create_thread_hybrid(&thread, task_thread_wrapper, handle, options)) {
        Asthra_task_handle_free(handle);
        return NULL;
    }

    // Detach thread if requested
    if (options && options->detached) {
        ASTHRA_CONCURRENCY_THREAD_DETACH(thread);
    }

    // Update statistics with relaxed ordering for performance
    asthra_concurrency_atomic_increment_counter_explicit(&bridge->stats.tasks_spawned,
                                                         ASTHRA_MEMORY_ORDER_RELAXED);

    return handle;
}

// =============================================================================
// TASK RESULT AND STATUS OPERATIONS
// =============================================================================

AsthraResult Asthra_task_get_result(AsthraConcurrencyTaskHandle *handle) {
    if (!handle) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid task handle");
    }

    ASTHRA_CONCURRENCY_MUTEX_LOCK(&handle->result_mutex);
    while (!atomic_load_explicit(&handle->is_complete, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        ASTHRA_CONCURRENCY_COND_WAIT(&handle->completion_cond, &handle->result_mutex);
    }
    AsthraResult result = handle->base_handle.result;
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&handle->result_mutex);

    return result;
}

bool Asthra_task_is_complete(AsthraConcurrencyTaskHandle *handle) {
    if (!handle) {
        return false;
    }
    return atomic_load_explicit(&handle->is_complete, ASTHRA_MEMORY_ORDER_ACQUIRE);
}

AsthraResult Asthra_task_wait_timeout(AsthraConcurrencyTaskHandle *handle, uint64_t timeout_ms) {
    if (!handle) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid task handle");
    }

    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += timeout_ms / 1000;
    timeout.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (timeout.tv_nsec >= 1000000000) {
        timeout.tv_sec++;
        timeout.tv_nsec -= 1000000000;
    }

    ASTHRA_CONCURRENCY_MUTEX_LOCK(&handle->result_mutex);
    int result = 0;
    while (!atomic_load_explicit(&handle->is_complete, ASTHRA_MEMORY_ORDER_ACQUIRE) &&
           result == 0) {
        result = ASTHRA_CONCURRENCY_COND_TIMEDWAIT(&handle->completion_cond, &handle->result_mutex,
                                                   &timeout);
    }

    if (result == ETIMEDOUT) {
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&handle->result_mutex);
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_TASK_TIMEOUT,
                                                      "Task wait timeout");
    }

    AsthraResult task_result = handle->base_handle.result;
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&handle->result_mutex);

    return task_result;
}

AsthraResult Asthra_task_cancel(AsthraConcurrencyTaskHandle *handle) {
    if (!handle) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid task handle");
    }

    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();

    // For this simplified implementation, we'll just mark the task as cancelled
    // In a real implementation, this would send a cancellation signal to the thread
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&handle->result_mutex);
    if (!atomic_load_explicit(&handle->is_complete, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        atomic_store_explicit(&handle->enhanced_state, ASTHRA_TASK_FAILED,
                              ASTHRA_MEMORY_ORDER_RELEASE);
        atomic_store_explicit(&handle->is_complete, true, ASTHRA_MEMORY_ORDER_RELEASE);

        // Create a cancellation error result
        int error_code = ASTHRA_CONCURRENCY_ERROR_TASK_NOT_FOUND; // Use as cancellation error
        handle->base_handle.result =
            asthra_result_err(&error_code, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_GC);

        ASTHRA_CONCURRENCY_COND_BROADCAST(&handle->completion_cond);
        asthra_concurrency_atomic_increment_counter(&bridge->stats.tasks_cancelled);
    }
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&handle->result_mutex);

    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
}

void Asthra_task_detach(AsthraConcurrencyTaskHandle *handle) {
    if (!handle) {
        return;
    }

    ASTHRA_CONCURRENCY_MUTEX_LOCK(&handle->result_mutex);
    atomic_store_explicit(&handle->is_detached, true, ASTHRA_MEMORY_ORDER_RELEASE);
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&handle->result_mutex);
}

void Asthra_task_handle_free(AsthraConcurrencyTaskHandle *handle) {
    if (!handle) {
        return;
    }

    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();

    // Remove from registry using atomic operations
    AsthraConcurrencyTaskHandle *current =
        atomic_load_explicit(&bridge->task_registry, ASTHRA_MEMORY_ORDER_ACQUIRE);
    AsthraConcurrencyTaskHandle *prev = NULL;

    while (current) {
        if (current == handle) {
            if (prev) {
                prev->next = current->next;
            } else {
                // Update the head of the list atomically
                AsthraConcurrencyTaskHandle *next = current->next;
                atomic_store_explicit(&bridge->task_registry, next, ASTHRA_MEMORY_ORDER_RELEASE);
            }
            break;
        }
        prev = current;
        current = current->next;
    }

    // Cleanup synchronization primitives
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&handle->result_mutex);
    ASTHRA_CONCURRENCY_COND_DESTROY(&handle->completion_cond);

    // Free task data
    if (handle->task_data) {
        free(handle->task_data);
    }

    // Don't free GC-managed results - let the GC handle cleanup
    // The result is marked with ASTHRA_OWNERSHIP_GC so it will be cleaned up automatically

    free(handle);
}

// =============================================================================
// TASK HANDLE WITH AWAIT IMPLEMENTATION
// =============================================================================

AsthraConcurrencyTaskHandleWithAwait *
Asthra_spawn_task_with_handle(AsthraConcurrencyTaskFunction func, void *args, size_t args_size,
                              const AsthraConcurrencyTaskSpawnOptions *options) {
    if (!asthra_concurrency_is_initialized()) {
        return NULL;
    }

    // Allocate awaitable task handle
    AsthraConcurrencyTaskHandleWithAwait *handle =
        malloc(sizeof(AsthraConcurrencyTaskHandleWithAwait));
    if (!handle) {
        return NULL;
    }

    // Initialize base handle using existing spawn function
    AsthraConcurrencyTaskHandle *base_handle = Asthra_spawn_task(func, args, args_size, options);
    if (!base_handle) {
        free(handle);
        return NULL;
    }

    // Copy base handle data
    handle->base = *base_handle;
    free(base_handle); // Free the temporary base handle

    // Initialize await-specific fields
    atomic_init(&handle->awaitable, true);
    atomic_init(&handle->awaited, false);

    // Initialize await synchronization
    if (!ASTHRA_CONCURRENCY_MUTEX_INIT(&handle->await_mutex) ||
        !ASTHRA_CONCURRENCY_COND_INIT(&handle->await_cond)) {
        free(handle);
        return NULL;
    }

    return handle;
}

AsthraResult Asthra_task_await(AsthraConcurrencyTaskHandleWithAwait *handle) {
    if (!handle || !atomic_load_explicit(&handle->awaitable, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid or non-awaitable task handle");
    }

    // Mark as awaited
    atomic_store_explicit(&handle->awaited, true, ASTHRA_MEMORY_ORDER_RELEASE);

    // Wait for completion using base handle functionality
    return Asthra_task_get_result(&handle->base);
}

AsthraResult Asthra_task_await_timeout(AsthraConcurrencyTaskHandleWithAwait *handle,
                                       uint64_t timeout_ms) {
    if (!handle || !atomic_load_explicit(&handle->awaitable, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE,
                                                      "Invalid or non-awaitable task handle");
    }

    // Mark as awaited
    atomic_store_explicit(&handle->awaited, true, ASTHRA_MEMORY_ORDER_RELEASE);

    // Wait for completion with timeout using base handle functionality
    return Asthra_task_wait_timeout(&handle->base, timeout_ms);
}

bool Asthra_task_handle_is_awaitable(AsthraConcurrencyTaskHandleWithAwait *handle) {
    if (!handle)
        return false;
    return atomic_load_explicit(&handle->awaitable, ASTHRA_MEMORY_ORDER_ACQUIRE) &&
           !atomic_load_explicit(&handle->awaited, ASTHRA_MEMORY_ORDER_ACQUIRE);
}

void Asthra_task_handle_with_await_free(AsthraConcurrencyTaskHandleWithAwait *handle) {
    if (!handle)
        return;

    // Cleanup await synchronization
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&handle->await_mutex);
    ASTHRA_CONCURRENCY_COND_DESTROY(&handle->await_cond);

    // Free base handle resources (but not the handle itself since it's embedded)
    if (handle->base.task_data) {
        free(handle->base.task_data);
    }

    // Cleanup base handle synchronization primitives
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&handle->base.result_mutex);
    ASTHRA_CONCURRENCY_COND_DESTROY(&handle->base.completion_cond);

    free(handle);
}
