/**
 * Asthra Runtime - Standard Library Concurrency Support
 * 
 * This header provides runtime support functions for the concurrent stdlib modules
 * created as part of Phase 6: Standard Library Migration.
 * 
 * These functions implement the FFI layer between Asthra stdlib concurrency modules
 * and the underlying C runtime system.
 */

#ifndef ASTHRA_STDLIB_CONCURRENCY_SUPPORT_H
#define ASTHRA_STDLIB_CONCURRENCY_SUPPORT_H

#include "asthra_runtime.h"
#include "asthra_concurrency_bridge_modular.h"
#include "asthra_safety_modular.h"
#include "errors/asthra_runtime_errors.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CHANNEL RUNTIME SUPPORT
// =============================================================================

/**
 * Channel operation result codes
 */
typedef enum {
    ASTHRA_CHANNEL_OK = 0,
    ASTHRA_CHANNEL_FULL = 1,
    ASTHRA_CHANNEL_CLOSED = 2,
    ASTHRA_CHANNEL_EMPTY = 3,
    ASTHRA_CHANNEL_ERROR = 4
} asthra_channel_result_t;

/**
 * Channel information structure for monitoring
 */
typedef struct {
    size_t current_count;
    size_t buffer_capacity;
    uint64_t total_sends;
    uint64_t total_receives;
    double utilization_percent;
    bool is_closed;
} AsthraConcurrencyChannelInfo;

// =============================================================================
// API COMPATIBILITY LAYER FOR LEGACY PERFORMANCE TESTS
// =============================================================================

// Include proper runtime result system instead of conflicting result.h
#include "types/asthra_runtime_result.h"

// Type compatibility aliases
typedef AsthraConcurrencyChannel AsthraConcurrencyChannelHandle;
typedef AsthraConcurrencyBarrier AsthraConcurrencyBarrierHandle;
typedef AsthraConcurrencySemaphore AsthraConcurrencySemaphoreHandle;

// Result type compatibility
typedef AsthraResult AsthraConcurrencyResult;

// Result value compatibility (removed - redefined below as integers)

// Function name compatibility macros
#define asthra_channel_create(size) Asthra_channel_create(sizeof(int), size, "test_channel")
#define asthra_channel_send(ch, val, sz) Asthra_channel_send(ch, val, 0)
#define asthra_channel_recv(ch, buf, sz) Asthra_channel_recv(ch, buf, 0)
#define asthra_channel_close(ch) Asthra_channel_close(ch)
#define asthra_channel_destroy(ch) Asthra_channel_destroy(ch)

// Barrier compatibility
#define asthra_barrier_create(count) Asthra_barrier_create(count)
#define asthra_barrier_wait(barrier, is_leader) Asthra_barrier_wait(barrier, is_leader)
#define asthra_barrier_destroy(barrier) asthra_barrier_destroy_impl(barrier)
#define asthra_barrier_thread_count(barrier) asthra_barrier_thread_count_impl(barrier)
#define asthra_barrier_waiting_count(barrier) Asthra_barrier_waiting_count(barrier)

// Semaphore compatibility  
#define asthra_semaphore_create(permits) Asthra_semaphore_create(permits)
#define asthra_semaphore_acquire(sem) Asthra_semaphore_acquire(sem)
#define asthra_semaphore_release(sem) Asthra_semaphore_release(sem)
#define asthra_semaphore_destroy(sem) Asthra_semaphore_destroy(sem)

// Stats compatibility - use existing bridge stats
static inline AsthraConcurrencyResult asthra_concurrency_get_stats(AsthraConcurrencyStats* stats) {
    if (stats) {
        *stats = Asthra_get_concurrency_stats();
        return (AsthraResult){ .tag = ASTHRA_RESULT_OK };
    }
    return (AsthraResult){ .tag = ASTHRA_RESULT_ERR };
}

// Result comparison compatibility - since AsthraResult is a struct, we need to compare the tag
static inline bool asthra_result_equals_success(AsthraResult result) {
    return result.tag == ASTHRA_RESULT_OK;
}

static inline bool asthra_result_equals_error(AsthraResult result) {
    return result.tag == ASTHRA_RESULT_ERR;
}

// Additional error constants for compatibility - defined as integers for thread return values
#define ASTHRA_CONCURRENCY_SUCCESS 0
#define ASTHRA_CONCURRENCY_ERROR_NULL_POINTER 1
#define ASTHRA_CONCURRENCY_ERROR_INVALID_SIZE 2
#define ASTHRA_CONCURRENCY_ERROR_WOULD_BLOCK 3
#define ASTHRA_CONCURRENCY_ERROR_TIMEOUT 4
#define ASTHRA_CONCURRENCY_ERROR_OUT_OF_MEMORY 5

// Barrier thread count compatibility function
static inline size_t asthra_barrier_thread_count_impl(const AsthraConcurrencyBarrier* barrier) {
    if (!barrier) return 0;
    return barrier->party_count;
}

// Barrier destroy wrapper that returns a result
static inline AsthraResult asthra_barrier_destroy_impl(AsthraConcurrencyBarrier* barrier) {
    if (!barrier) {
        return (AsthraResult){ 
            .tag = ASTHRA_RESULT_ERR,
            .data.err.error = &(int){ASTHRA_CONCURRENCY_ERROR_NULL_POINTER},
            .data.err.error_size = sizeof(int),
            .data.err.error_type_id = 0
        };
    }
    Asthra_barrier_destroy(barrier);
    return (AsthraResult){ .tag = ASTHRA_RESULT_OK };
}

// Helper function to get error code from AsthraResult for integer comparisons
static inline int asthra_result_error_code(AsthraResult result) {
    if (result.tag == ASTHRA_RESULT_ERR && result.data.err.error && result.data.err.error_size == sizeof(int)) {
        return *(int*)result.data.err.error;
    }
    return result.tag == ASTHRA_RESULT_OK ? ASTHRA_CONCURRENCY_SUCCESS : ASTHRA_CONCURRENCY_ERROR_TIMEOUT;
}

// Helper macros for result comparisons in tests
#define ASSERT_RESULT_SUCCESS(result) ASSERT_TRUE(asthra_result_equals_success(result))
#define ASSERT_RESULT_ERROR(result) ASSERT_TRUE(asthra_result_equals_error(result))

// Override ASSERT_EQUAL for AsthraResult comparisons with ASTHRA_CONCURRENCY_SUCCESS
#define ASSERT_CONCURRENCY_SUCCESS(result) ASSERT_TRUE(asthra_result_equals_success(result))

// Field access helpers for legacy performance tests - only use with final_stats variable
#define ASTHRA_STATS_ACTIVE_CHANNELS(stats) (stats).channel_stats.channels_active
#define ASTHRA_STATS_ACTIVE_TASKS(stats) (stats).task_stats.active_tasks
#define ASTHRA_STATS_ACTIVE_BARRIERS(stats) (stats).active_components
#define ASTHRA_STATS_TOTAL_SEND_OPS(stats) (stats).channel_stats.total_sends
#define ASTHRA_STATS_TOTAL_RECV_OPS(stats) (stats).channel_stats.total_receives
#define ASTHRA_STATS_MEMORY_USAGE(stats) (stats).memory_usage

// Redefine comparison macros to work with AsthraResult struct
#undef ASTHRA_CONCURRENCY_SUCCESS
#define ASTHRA_CONCURRENCY_SUCCESS ASTHRA_RESULT_OK

// Override function calls to return simple boolean for easier comparison
#undef asthra_channel_send
#undef asthra_semaphore_acquire  
#undef asthra_semaphore_release

#define asthra_channel_send(ch, val, sz) asthra_result_equals_success(Asthra_channel_send(ch, val, 0))
#define asthra_semaphore_acquire(sem) asthra_result_equals_success(Asthra_semaphore_acquire(sem))
#define asthra_semaphore_release(sem) asthra_result_equals_success(Asthra_semaphore_release(sem))

// Override the result comparison to work properly with the struct
static inline bool asthra_concurrency_result_is_success(AsthraConcurrencyResult result) {
    return asthra_result_equals_success(result);
}

// Initialization and cleanup compatibility
#define asthra_concurrency_initialize() (AsthraResult){ .tag = ASTHRA_RESULT_OK }
#define asthra_concurrency_cleanup() Asthra_concurrency_bridge_cleanup()

/**
 * Create a new channel (matches existing signature)
 */
AsthraConcurrencyChannel* Asthra_stdlib_channel_create(size_t element_size, size_t buffer_capacity, const char* name);

/**
 * Create a new unbuffered channel (convenience wrapper)
 */
AsthraConcurrencyChannel* Asthra_stdlib_channel_create_unbuffered(size_t element_size);

/**
 * Create a new buffered channel with specified capacity (convenience wrapper)
 */
AsthraConcurrencyChannel* Asthra_stdlib_channel_create_buffered(size_t element_size, size_t capacity);

/**
 * Send a value through the channel (blocking, stdlib interface)
 */
AsthraResult Asthra_stdlib_channel_send(
    AsthraConcurrencyChannel* channel,
    const void* value,
    uint64_t timeout_ms
);

/**
 * Send a value through the channel (non-blocking, stdlib interface)
 */
AsthraResult Asthra_stdlib_channel_try_send(
    AsthraConcurrencyChannel* channel,
    const void* value
);

/**
 * Receive a value from the channel (blocking, stdlib interface)
 */
AsthraResult Asthra_stdlib_channel_recv(
    AsthraConcurrencyChannel* channel,
    void* value_out,
    uint64_t timeout_ms
);

/**
 * Receive a value from the channel (non-blocking, stdlib interface)
 */
AsthraResult Asthra_stdlib_channel_try_recv(
    AsthraConcurrencyChannel* channel,
    void* value_out
);

/**
 * Close the channel (stdlib interface)
 */
void Asthra_stdlib_channel_close(AsthraConcurrencyChannel* channel);

/**
 * Check if channel is closed (stdlib interface)
 */
bool Asthra_stdlib_channel_is_closed(const AsthraConcurrencyChannel* channel);

/**
 * Get channel statistics (stdlib interface)
 */
bool Asthra_stdlib_channel_get_stats(const AsthraConcurrencyChannel* channel, 
                                     size_t* count, size_t* capacity);

/**
 * Get channel information for monitoring (stdlib interface)
 */
AsthraConcurrencyChannelInfo Asthra_stdlib_channel_get_info(const AsthraConcurrencyChannel* channel);

// =============================================================================
// SELECT OPERATION SUPPORT
// =============================================================================

/**
 * Stdlib select operation types
 */
typedef enum {
    ASTHRA_STDLIB_SELECT_SEND = 0,
    ASTHRA_STDLIB_SELECT_RECV = 1,
    ASTHRA_STDLIB_SELECT_TIMEOUT = 2,
    ASTHRA_STDLIB_SELECT_DEFAULT = 3
} asthra_stdlib_select_op_type_t;

/**
 * Stdlib select operation descriptor
 */
typedef struct {
    asthra_stdlib_select_op_type_t type;
    AsthraConcurrencyChannel* channel;
    void* data;
    size_t data_size;
    int64_t timeout_ms;
} asthra_stdlib_select_op_t;

/**
 * Stdlib select result types
 */
typedef enum {
    ASTHRA_STDLIB_SELECT_SEND_OK = 0,
    ASTHRA_STDLIB_SELECT_RECV_OK = 1,
    ASTHRA_STDLIB_SELECT_RESULT_TIMEOUT = 2,
    ASTHRA_STDLIB_SELECT_RESULT_DEFAULT = 3,
    ASTHRA_STDLIB_SELECT_ERROR = 4
} asthra_stdlib_select_result_type_t;

/**
 * Stdlib select result structure
 */
typedef struct {
    asthra_stdlib_select_result_type_t type;
    size_t channel_index;
    void* received_data;
    size_t received_size;
    char error_message[256];
} asthra_stdlib_select_result_t;

/**
 * Execute a select operation (stdlib interface)
 */
asthra_stdlib_select_result_t Asthra_stdlib_select_execute(
    asthra_stdlib_select_op_t* operations,
    size_t op_count,
    int64_t timeout_ms,
    bool has_default
);

// =============================================================================
// BARRIER SUPPORT
// =============================================================================

/**
 * Create a new barrier for N parties
 */
AsthraConcurrencyBarrier* Asthra_stdlib_barrier_create(size_t party_count);

/**
 * Wait at the barrier until all parties arrive
 * Returns true if this thread is the "leader" (last to arrive)
 */
AsthraResult Asthra_stdlib_barrier_wait(AsthraConcurrencyBarrier* barrier, bool* is_leader);

/**
 * Get the number of parties currently waiting at the barrier
 */
size_t Asthra_stdlib_barrier_waiting_count(const AsthraConcurrencyBarrier* barrier);

/**
 * Reset the barrier to be reused
 */
AsthraResult Asthra_stdlib_barrier_reset(AsthraConcurrencyBarrier* barrier);

/**
 * Destroy a barrier
 */
void Asthra_stdlib_barrier_destroy(AsthraConcurrencyBarrier* barrier);

// =============================================================================
// SEMAPHORE SUPPORT
// =============================================================================

/**
 * Create a new semaphore with specified permits
 */
AsthraConcurrencySemaphore* Asthra_stdlib_semaphore_create(size_t permits);

/**
 * Acquire a permit from the semaphore (blocking)
 */
AsthraResult Asthra_stdlib_semaphore_acquire(AsthraConcurrencySemaphore* semaphore);

/**
 * Try to acquire a permit (non-blocking)
 * Returns true if acquired, false if not available
 */
bool Asthra_stdlib_semaphore_try_acquire(AsthraConcurrencySemaphore* semaphore);

/**
 * Acquire a permit with timeout
 * Returns true if acquired, false if timeout occurred
 */
AsthraResult Asthra_stdlib_semaphore_acquire_timeout(
    AsthraConcurrencySemaphore* semaphore,
    int64_t timeout_ms,
    bool* acquired
);

/**
 * Release a permit back to the semaphore
 */
AsthraResult Asthra_stdlib_semaphore_release(AsthraConcurrencySemaphore* semaphore);

/**
 * Get the current number of available permits
 */
size_t Asthra_stdlib_semaphore_available_permits(const AsthraConcurrencySemaphore* semaphore);

/**
 * Destroy a semaphore
 */
void Asthra_stdlib_semaphore_destroy(AsthraConcurrencySemaphore* semaphore);

// =============================================================================
// MUTEX SUPPORT
// =============================================================================

/**
 * Create a new mutex
 */
AsthraConcurrencyMutex* Asthra_stdlib_mutex_create(void);

/**
 * Lock the mutex (blocking)
 */
AsthraResult Asthra_stdlib_mutex_lock(AsthraConcurrencyMutex* mutex);

/**
 * Try to lock the mutex (non-blocking)
 * Returns true if locked, false if already locked
 */
bool Asthra_stdlib_mutex_try_lock(AsthraConcurrencyMutex* mutex);

/**
 * Unlock the mutex
 */
AsthraResult Asthra_stdlib_mutex_unlock(AsthraConcurrencyMutex* mutex);

/**
 * Destroy a mutex
 */
void Asthra_stdlib_mutex_destroy(AsthraConcurrencyMutex* mutex);

// =============================================================================
// CONDITION VARIABLE SUPPORT
// =============================================================================

/**
 * Create a new condition variable
 */
AsthraConcurrencyCondVar* Asthra_stdlib_condvar_create(void);

/**
 * Wait on the condition variable with a mutex
 */
AsthraResult Asthra_stdlib_condvar_wait(
    AsthraConcurrencyCondVar* condvar,
    AsthraConcurrencyMutex* mutex
);

/**
 * Wait on the condition variable with timeout
 * Returns true if notified, false if timeout occurred
 */
AsthraResult Asthra_stdlib_condvar_wait_timeout(
    AsthraConcurrencyCondVar* condvar,
    AsthraConcurrencyMutex* mutex,
    int64_t timeout_ms,
    bool* notified
);

/**
 * Notify one waiting thread
 */
AsthraResult Asthra_stdlib_condvar_notify_one(AsthraConcurrencyCondVar* condvar);

/**
 * Notify all waiting threads
 */
AsthraResult Asthra_stdlib_condvar_notify_all(AsthraConcurrencyCondVar* condvar);

/**
 * Destroy a condition variable
 */
void Asthra_stdlib_condvar_destroy(AsthraConcurrencyCondVar* condvar);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Get current time in milliseconds
 */
uint64_t Asthra_stdlib_get_current_time_ms(void);

/**
 * Sleep for specified milliseconds
 */
AsthraResult Asthra_stdlib_sleep_ms(uint64_t milliseconds);

/**
 * Get system CPU count for optimal worker pool sizing
 */
size_t Asthra_stdlib_get_cpu_count(void);

/**
 * Create a task handle for spawn_with_handle operations
 */
AsthraConcurrencyTaskHandle* Asthra_stdlib_create_task_handle(void);

/**
 * Check if a task has completed
 */
bool Asthra_stdlib_task_is_complete(const AsthraConcurrencyTaskHandle* handle);

/**
 * Wait for a task to complete and get its result
 */
AsthraResult Asthra_stdlib_task_await(
    AsthraConcurrencyTaskHandle* handle,
    void* result_out,
    size_t result_size
);

// =============================================================================
// ERROR HANDLING AND DIAGNOSTICS
// =============================================================================

/**
 * Get the last error message for the current thread
 */
const char* Asthra_stdlib_get_last_concurrency_error(void);

/**
 * Set an error message for the current thread
 */
void Asthra_stdlib_set_concurrency_error(const char* error_message);

/**
 * Clear the error state for the current thread
 */
void Asthra_stdlib_clear_concurrency_error(void);

/**
 * Get concurrency runtime statistics
 */
typedef struct {
    size_t active_channels;
    size_t active_tasks;
    size_t active_barriers;
    size_t active_semaphores;
    size_t active_mutexes;
    size_t total_memory_allocated;
    double cpu_utilization_percent;
} asthra_concurrency_stats_t;

asthra_concurrency_stats_t Asthra_stdlib_get_concurrency_stats(void);

// =============================================================================
// INITIALIZATION AND CLEANUP
// =============================================================================

/**
 * Initialize the stdlib concurrency runtime
 * Must be called before using any concurrency features
 */
AsthraResult Asthra_stdlib_init_concurrency(void);

/**
 * Cleanup the stdlib concurrency runtime
 * Should be called at program exit
 */
void Asthra_stdlib_cleanup_concurrency(void);

/**
 * Check if the stdlib concurrency runtime is initialized
 */
bool Asthra_stdlib_is_concurrency_initialized(void);

#ifdef __cplusplus
}
#endif

// Duplicate definitions removed - using compatibility layer defined earlier

// Timeout function compatibility
#define asthra_channel_send_timeout(ch, data, size, timeout) Asthra_channel_send(ch, data, timeout)
#define asthra_channel_recv_timeout(ch, data, size, timeout) Asthra_channel_recv(ch, data, timeout)

// Duplicate compatibility macros removed - using definitions from earlier in file

// Init/cleanup function compatibility macros
#undef asthra_concurrency_cleanup
#define asthra_concurrency_init() Asthra_stdlib_init_concurrency()
#define asthra_concurrency_cleanup() Asthra_stdlib_cleanup_concurrency()

// AsthraResult struct comparison helpers
#define ASTHRA_RESULT_IS_OK(result) ((result).tag == ASTHRA_RESULT_OK)
#define ASTHRA_RESULT_IS_ERR(result) ((result).tag == ASTHRA_RESULT_ERR)

// Statistics field access helpers  
#undef ASTHRA_STATS_ACTIVE_CHANNELS
#define ASTHRA_STATS_ACTIVE_CHANNELS(stats) ((stats)->active_channels)
#define ASTHRA_STATS_TOTAL_MESSAGES(stats) ((stats)->total_messages_sent)
#define ASTHRA_STATS_BLOCKED_OPERATIONS(stats) ((stats)->blocked_operations)

// Additional result constants for compatibility
#define ASTHRA_RESULT_ALREADY_INITIALIZED ASTHRA_RESULT_OK

#endif // ASTHRA_STDLIB_CONCURRENCY_SUPPORT_H 
