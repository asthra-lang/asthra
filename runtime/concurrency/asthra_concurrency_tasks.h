/**
 * Asthra Concurrency Tasks - Task Management and Execution
 * Part of the Asthra Runtime Modularization Plan - Phase 2
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * DESIGN GOALS:
 * - Task handle definitions and lifecycle management
 * - Task spawn options and configuration
 * - Await functionality for future support
 * - Result handling and timeout management
 */

#ifndef ASTHRA_CONCURRENCY_TASKS_H
#define ASTHRA_CONCURRENCY_TASKS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../asthra_runtime.h"
#include "asthra_concurrency_atomics.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TASK FUNCTION SIGNATURES
// =============================================================================

/**
 * Enhanced task function signature for concurrency bridge
 */
typedef AsthraResult (*AsthraConcurrencyTaskFunction)(void *args, size_t args_size);

// =============================================================================
// TASK HANDLE STRUCTURES
// =============================================================================

/**
 * Enhanced task handle for future await support with C17 atomic state
 */
typedef struct AsthraConcurrencyTaskHandle {
    AsthraTaskHandle base_handle;                        // Base task handle
    asthra_concurrency_atomic_int enhanced_state;        // C17 atomic state
    asthra_concurrency_atomic_bool is_complete;          // C17 atomic completion flag
    asthra_concurrency_atomic_bool is_detached;          // C17 atomic detachment flag
    asthra_concurrency_atomic_ptr_t result_ptr;          // Atomic result pointer
    asthra_concurrency_mutex_t result_mutex;             // Protects result access
    asthra_concurrency_cond_t completion_cond;           // Signals task completion
    void *task_data;                                     // Opaque task data
    size_t task_data_size;                               // Size of task data
    asthra_concurrency_atomic_counter_t creation_time;   // Creation timestamp
    asthra_concurrency_atomic_counter_t completion_time; // Completion timestamp
    struct AsthraConcurrencyTaskHandle *next;            // For linked list management
} AsthraConcurrencyTaskHandle;

/**
 * Task handle with await capabilities
 */
typedef struct AsthraConcurrencyTaskHandleWithAwait {
    AsthraConcurrencyTaskHandle base;       // Base task handle
    _Atomic(bool) awaitable;                // Can be awaited
    _Atomic(bool) awaited;                  // Has been awaited
    asthra_concurrency_cond_t await_cond;   // Condition for await
    asthra_concurrency_mutex_t await_mutex; // Mutex for await
} AsthraConcurrencyTaskHandleWithAwait;

/**
 * Task spawn options structure
 */
typedef struct {
    size_t stack_size;    // Stack size (0 for default)
    int priority;         // Task priority (-20 to 20)
    bool detached;        // Start detached
    const char *name;     // Task name for debugging
    void *affinity_mask;  // CPU affinity (platform-specific)
    uint64_t timeout_ms;  // Task timeout in milliseconds
    bool use_c17_threads; // Prefer C17 threads if available
} AsthraConcurrencyTaskSpawnOptions;

// =============================================================================
// TASK LIFECYCLE FUNCTIONS
// =============================================================================

/**
 * Initialize the task management system
 * @param max_tasks Maximum number of concurrent tasks
 * @return Result indicating success or failure
 */
AsthraResult Asthra_task_system_init(size_t max_tasks);

/**
 * Cleanup the task management system
 */
void Asthra_task_system_cleanup(void);

/**
 * Spawn a new task
 * @param func Task function to execute
 * @param args Arguments for the task function
 * @param args_size Size of arguments
 * @param options Spawn options (can be NULL for defaults)
 * @return Task handle for the spawned task
 */
AsthraConcurrencyTaskHandle *Asthra_spawn_task(AsthraConcurrencyTaskFunction func, void *args,
                                               size_t args_size,
                                               const AsthraConcurrencyTaskSpawnOptions *options);

/**
 * Spawn a task with an awaitable handle
 * @param func Task function to execute
 * @param args Arguments for the task function
 * @param args_size Size of arguments
 * @param options Spawn options (can be NULL for defaults)
 * @return Awaitable task handle for future operations
 */
AsthraConcurrencyTaskHandleWithAwait *
Asthra_spawn_task_with_handle(AsthraConcurrencyTaskFunction func, void *args, size_t args_size,
                              const AsthraConcurrencyTaskSpawnOptions *options);

/**
 * Get the result of a completed task
 * @param handle Task handle to query
 * @return Task result
 */
AsthraResult Asthra_task_get_result(AsthraConcurrencyTaskHandle *handle);

/**
 * Check if a task is complete
 * @param handle Task handle to check
 * @return true if task is complete, false otherwise
 */
bool Asthra_task_is_complete(AsthraConcurrencyTaskHandle *handle);

/**
 * Wait for a task to complete with timeout
 * @param handle Task handle to wait for
 * @param timeout_ms Timeout in milliseconds
 * @return Task result or timeout error
 */
AsthraResult Asthra_task_wait_timeout(AsthraConcurrencyTaskHandle *handle, uint64_t timeout_ms);

/**
 * Cancel a running task
 * @param handle Task handle to cancel
 * @return Result indicating success or failure
 */
AsthraResult Asthra_task_cancel(AsthraConcurrencyTaskHandle *handle);

/**
 * Detach a task (fire and forget)
 * @param handle Task handle to detach
 */
void Asthra_task_detach(AsthraConcurrencyTaskHandle *handle);

/**
 * Free a task handle
 * @param handle Handle to free
 */
void Asthra_task_handle_free(AsthraConcurrencyTaskHandle *handle);

// =============================================================================
// AWAITABLE TASK OPERATIONS
// =============================================================================

/**
 * Await a task completion
 * @param handle Task handle to await
 * @return Task result
 */
AsthraResult Asthra_task_await(AsthraConcurrencyTaskHandleWithAwait *handle);

/**
 * Await a task completion with timeout
 * @param handle Task handle to await
 * @param timeout_ms Timeout in milliseconds
 * @return Task result or timeout error
 */
AsthraResult Asthra_task_await_timeout(AsthraConcurrencyTaskHandleWithAwait *handle,
                                       uint64_t timeout_ms);

/**
 * Check if a task handle is awaitable
 * @param handle Task handle to check
 * @return true if awaitable, false otherwise
 */
bool Asthra_task_handle_is_awaitable(AsthraConcurrencyTaskHandleWithAwait *handle);

/**
 * Free an awaitable task handle
 * @param handle Handle to free
 */
void Asthra_task_handle_with_await_free(AsthraConcurrencyTaskHandleWithAwait *handle);

// =============================================================================
// TASK STATISTICS AND MONITORING
// =============================================================================

/**
 * Task statistics structure
 */
typedef struct {
    asthra_concurrency_atomic_counter_t tasks_spawned;        // Tasks spawned
    asthra_concurrency_atomic_counter_t tasks_completed;      // Tasks completed
    asthra_concurrency_atomic_counter_t tasks_failed;         // Tasks failed
    asthra_concurrency_atomic_counter_t tasks_cancelled;      // Tasks cancelled
    asthra_concurrency_atomic_counter_t tasks_timeout;        // Tasks that timed out
    asthra_concurrency_atomic_counter_t total_execution_time; // Total execution time
    asthra_concurrency_atomic_counter_t active_tasks;         // Currently active tasks
} AsthraConcurrencyTaskStats;

/**
 * Get task system statistics
 * @return Task statistics structure
 */
AsthraConcurrencyTaskStats Asthra_get_task_stats(void);

/**
 * Reset task statistics
 */
void Asthra_reset_task_stats(void);

// =============================================================================
// ERROR CODES
// =============================================================================

#define ASTHRA_TASK_ERROR_BASE 0x1000

typedef enum {
    ASTHRA_TASK_ERROR_NONE = 0,
    ASTHRA_TASK_ERROR_INIT_FAILED = ASTHRA_TASK_ERROR_BASE + 1,
    ASTHRA_TASK_ERROR_SPAWN_FAILED = ASTHRA_TASK_ERROR_BASE + 2,
    ASTHRA_TASK_ERROR_NOT_FOUND = ASTHRA_TASK_ERROR_BASE + 3,
    ASTHRA_TASK_ERROR_TIMEOUT = ASTHRA_TASK_ERROR_BASE + 4,
    ASTHRA_TASK_ERROR_INVALID_HANDLE = ASTHRA_TASK_ERROR_BASE + 5,
    ASTHRA_TASK_ERROR_ALREADY_COMPLETE = ASTHRA_TASK_ERROR_BASE + 6,
    ASTHRA_TASK_ERROR_CANCELLED = ASTHRA_TASK_ERROR_BASE + 7,
    ASTHRA_TASK_ERROR_SYSTEM_ERROR = ASTHRA_TASK_ERROR_BASE + 8
} AsthraConcurrencyTaskErrorCode;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_TASKS_H
