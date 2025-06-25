/**
 * Asthra Concurrency Bridge - Modular Version (Phase 2 Complete)
 * Umbrella header for all concurrency modules with backward compatibility
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * DESIGN GOALS:
 * - Full backward compatibility with original asthra_concurrency_bridge.h
 * - Clean modular architecture with focused components
 * - Systematic integration of all concurrency subsystems
 * - Enhanced functionality while preserving existing APIs
 */

#ifndef ASTHRA_CONCURRENCY_BRIDGE_MODULAR_H
#define ASTHRA_CONCURRENCY_BRIDGE_MODULAR_H

// =============================================================================
// MODULAR CONCURRENCY COMPONENTS - PHASE 2
// =============================================================================

#include "concurrency/asthra_concurrency_atomics.h"
#include "concurrency/asthra_concurrency_channels.h"
#include "concurrency/asthra_concurrency_patterns.h"
#include "concurrency/asthra_concurrency_sync.h"
#include "concurrency/asthra_concurrency_tasks.h"

#ifdef __cplusplus
extern "C" {

// asthra_safety_log_task_lifecycle_event_enhanced
void asthra_safety_log_task_lifecycle_event_enhanced(uint64_t task_id, AsthraTaskEvent event,;


// asthra_safety_concurrency_errors_cleanup
;

#endif

// =============================================================================
// VERSION AND BUILD INFORMATION
// =============================================================================

#define ASTHRA_CONCURRENCY_BRIDGE_MODULAR_VERSION_MAJOR 2
#define ASTHRA_CONCURRENCY_BRIDGE_MODULAR_VERSION_MINOR 0
#define ASTHRA_CONCURRENCY_BRIDGE_MODULAR_VERSION_PATCH 0
#define ASTHRA_CONCURRENCY_BRIDGE_MODULAR_VERSION "2.0.0"

// Phase 2 completion marker
#define ASTHRA_CONCURRENCY_MODULARIZATION_PHASE_2_COMPLETE 1

// =============================================================================
// UNIFIED STATISTICS STRUCTURE
// =============================================================================

/**
 * Comprehensive concurrency statistics combining all modules
 */
typedef struct {
    // Task system statistics
    AsthraConcurrencyTaskStats task_stats;

    // Synchronization statistics
    AsthraConcurrencySyncStats sync_stats;

    // Channel system statistics
    AsthraConcurrencyChannelStats channel_stats;

    // Pattern system statistics
    AsthraConcurrencyPatternStats pattern_stats;

    // Overall system statistics
    asthra_concurrency_atomic_counter_t total_operations;  // Total operations across all systems
    asthra_concurrency_atomic_counter_t total_errors;      // Total errors across all systems
    asthra_concurrency_atomic_counter_t memory_usage;      // Total memory usage
    asthra_concurrency_atomic_counter_t active_components; // Active components count

    // Callback system statistics
    asthra_concurrency_atomic_counter_t callbacks_enqueued;  // Total callbacks enqueued
    asthra_concurrency_atomic_counter_t callbacks_processed; // Total callbacks processed
    asthra_concurrency_atomic_counter_t callbacks_dropped;   // Total callbacks dropped

    // Task system statistics
    asthra_concurrency_atomic_counter_t tasks_spawned;   // Tasks spawned
    asthra_concurrency_atomic_counter_t tasks_completed; // Tasks completed
    asthra_concurrency_atomic_counter_t tasks_failed;    // Tasks failed
    asthra_concurrency_atomic_counter_t tasks_cancelled; // Tasks cancelled
    asthra_concurrency_atomic_counter_t tasks_timeout;   // Tasks that timed out

    // Thread system statistics
    asthra_concurrency_atomic_counter_t threads_registered;  // Threads registered
    asthra_concurrency_atomic_counter_t gc_roots_registered; // GC roots registered

    // Synchronization statistics
    asthra_concurrency_atomic_counter_t mutex_contentions;  // Mutex contentions
    asthra_concurrency_atomic_counter_t rwlock_contentions; // RWLock contentions

    // Data structure operations
    asthra_concurrency_atomic_counter_t string_operations;    // String operations
    asthra_concurrency_atomic_counter_t slice_operations;     // Slice operations
    asthra_concurrency_atomic_counter_t lock_free_operations; // Lock-free operations

    // Safety and debugging
    asthra_concurrency_atomic_counter_t memory_ordering_violations; // Memory ordering violations
} AsthraConcurrencyStats;

// =============================================================================
// CORE BRIDGE STRUCTURE
// =============================================================================

/**
 * Core concurrency bridge structure with C17 atomics support
 */
typedef struct {
    asthra_concurrency_atomic_bool initialized; // C17 atomic initialization flag
    size_t max_tasks;
    size_t max_callbacks;
    _Atomic(AsthraConcurrencyTaskHandle *) task_registry; // Atomic task registry
    AsthraConcurrencyCallbackQueue global_callback_queue;
    AsthraConcurrencyThreadRegistry thread_registry;
    AsthraConcurrencyStats stats;
    asthra_concurrency_mutex_t global_mutex;          // Hybrid mutex
    asthra_concurrency_atomic_counter_t next_task_id; // C17 atomic task ID counter
    asthra_concurrency_atomic_counter_t init_time;    // Initialization time
} AsthraConcurrencyBridge;

/**
 * Get the global concurrency bridge instance
 * Pointer to the global bridge instance
 */
AsthraConcurrencyBridge* asthra_concurrency_get_bridge(void);

/**
 * Check if concurrency bridge is initialized
 * true if initialized, false otherwise
 */
bool asthra_concurrency_is_initialized(void);

// =============================================================================
// CALLBACK SYSTEM API
// =============================================================================

/**
 * Callback function signature
 */
typedef AsthraResult (*AsthraConcurrencyCallbackFunction)(void* data, size_t data_size, void* context);

/**
 * Callback queue statistics structure
 */
typedef struct {
    size_t current_size;
    size_t max_capacity;
    uint64_t total_enqueued;
    uint64_t total_processed;
    uint64_t total_dropped;
    bool is_shutdown;
    uint64_t oldest_callback_age_ms;
} AsthraConcurrencyCallbackQueueStats;

/**
 * Enqueue a callback for execution
 * func Callback function to execute
 * data Data to pass to the callback
 * data_size Size of the data
 * context Context pointer for the callback
 * priority Priority of the callback (higher = more priority)
 * Result indicating success or failure
 */
AsthraResult Asthra_enqueue_callback(AsthraConcurrencyCallbackFunction func, void* data, 
                                    size_t data_size, void* context, uint32_t priority);

/**
 * Enqueue a priority callback for execution
 * func Callback function to execute
 * data Data to pass to the callback
 * data_size Size of the data
 * context Context pointer for the callback
 * priority Priority of the callback
 * insert_at_front Whether to insert at front for high priority
 * Result indicating success or failure
 */
AsthraResult Asthra_enqueue_priority_callback(AsthraConcurrencyCallbackFunction func, void* data, 
                                             size_t data_size, void* context, uint32_t priority, 
                                             bool insert_at_front);

/**
 * Process callbacks from the queue
 * max_callbacks Maximum number of callbacks to process
 * Number of callbacks processed
 */
size_t Asthra_process_callbacks(size_t max_callbacks);

/**
 * Get callback queue size
 * Current size of the callback queue
 */
size_t Asthra_get_callback_queue_size(void);

/**
 * Check if callback queue is empty
 * true if empty, false otherwise
 */
bool Asthra_is_callback_queue_empty(void);

/**
 * Check if callback queue is full
 * true if full, false otherwise
 */
bool Asthra_is_callback_queue_full(void);

/**
 * Clear all callbacks from the queue
 */
void Asthra_clear_callback_queue(void);

/**
 * Get callback queue statistics
 * Statistics structure
 */
AsthraConcurrencyCallbackQueueStats Asthra_get_callback_queue_stats(void);

/**
 * Dump callback queue statistics to output stream
 * output Output stream to write to
 */
void Asthra_dump_callback_queue_stats(FILE* output);

// =============================================================================
// ERROR HANDLING
// =============================================================================

/**
 * Comprehensive error codes covering all concurrency modules
 */
typedef enum {
    ASTHRA_CONCURRENCY_ERROR_NONE = 0,
    
    // Bridge-level errors (compatible with original)
    ASTHRA_CONCURRENCY_ERROR_INIT_FAILED = 0x0001,
    ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR = 0x0002,
    
    // Task errors (0x1000 range)
    ASTHRA_CONCURRENCY_ERROR_TASK_SPAWN_FAILED = 0x1001,
    ASTHRA_CONCURRENCY_ERROR_TASK_NOT_FOUND = 0x1002,
    ASTHRA_CONCURRENCY_ERROR_TASK_TIMEOUT = 0x1003,
    ASTHRA_CONCURRENCY_ERROR_INVALID_HANDLE = 0x1004,
    
    // Sync errors (0x2000 range)
    ASTHRA_CONCURRENCY_ERROR_THREAD_NOT_REGISTERED = 0x2001,
    ASTHRA_CONCURRENCY_ERROR_MUTEX_TIMEOUT = 0x2002,
    ASTHRA_CONCURRENCY_ERROR_RWLOCK_TIMEOUT = 0x2003,
    
    // Channel errors (0x3000 range)
    ASTHRA_CONCURRENCY_ERROR_CALLBACK_QUEUE_FULL = 0x3001,
    ASTHRA_CONCURRENCY_ERROR_CHANNEL_CLOSED = 0x3002,
    ASTHRA_CONCURRENCY_ERROR_WOULD_BLOCK = 0x3003,
    
    // Pattern errors (0x4000 range)
    ASTHRA_CONCURRENCY_ERROR_POOL_FULL = 0x4001,
    ASTHRA_CONCURRENCY_ERROR_PIPELINE_FAILED = 0x4002,
    ASTHRA_CONCURRENCY_ERROR_WORKER_FAILED = 0x4003
} AsthraConcurrencyErrorCode;

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Get current timestamp in milliseconds
 * Current timestamp
 */
uint64_t asthra_concurrency_get_timestamp_ms(void);

/**
 * Create an error result for concurrency operations
 * code Error code
 * message Error message
 * Error result
 */
AsthraResult asthra_concurrency_create_error_result(AsthraConcurrencyErrorCode code, const char *message);

/**
 * Atomic size operations
 */
size_t asthra_concurrency_atomic_load_size(asthra_concurrency_atomic_size_t *size_ptr);
size_t asthra_concurrency_atomic_increment_size(asthra_concurrency_atomic_size_t *size_ptr);
size_t asthra_concurrency_atomic_decrement_size(asthra_concurrency_atomic_size_t *size_ptr);

/**
 * Atomic counter operations
 */
uint64_t asthra_concurrency_atomic_increment_counter(asthra_concurrency_atomic_counter_t *counter);
uint64_t asthra_concurrency_atomic_decrement_counter(asthra_concurrency_atomic_counter_t *counter);
uint64_t asthra_concurrency_atomic_load_counter(asthra_concurrency_atomic_counter_t *counter);
void asthra_concurrency_atomic_store_counter(asthra_concurrency_atomic_counter_t *counter, uint64_t value);

// =============================================================================
// UNIFIED INITIALIZATION AND CLEANUP
// =============================================================================

/**
 * Initialize the complete concurrency bridge system
 * max_tasks Maximum number of concurrent tasks
 * max_callbacks Maximum number of queued callbacks
 * Result indicating success or failure
 */
AsthraResult Asthra_concurrency_bridge_init(size_t max_tasks, size_t max_callbacks);

/**
 * Cleanup the complete concurrency bridge system
 */
void Asthra_concurrency_bridge_cleanup(void);

/**
 * Get comprehensive concurrency statistics
 * Unified statistics structure
 */
AsthraConcurrencyStats Asthra_get_concurrency_stats(void);

/**
 * Reset all concurrency statistics
 */
void Asthra_reset_concurrency_stats(void);

/**
 * Dump complete concurrency state for debugging
 * output Output stream to write to
 */
void Asthra_dump_concurrency_state(FILE* output);

/**
 * Get a human-readable error message for a concurrency error code
 * error_code Error code to describe
 * Error message string
 */
const char* Asthra_concurrency_error_string(AsthraConcurrencyErrorCode error_code);

// =============================================================================
// MODULE INFORMATION API
// =============================================================================

/**
 * Information about a concurrency module
 */
typedef struct {
    const char *name;
    const char *version;
    const char *description;
    bool is_initialized;
    size_t memory_usage;
    uint64_t operation_count;
} AsthraConcurrencyModuleInfo;

/**
 * Get information about all concurrency modules
 * modules Array to store module information (must have space for 5 modules)
 * count Number of modules (will be set to actual count)
 * Result indicating success or failure
 */
AsthraResult Asthra_get_concurrency_module_info(AsthraConcurrencyModuleInfo* modules, size_t* count);

/**
 * Get information about a specific module
 * module_name Name of the module ("atomics", "tasks", "sync", "channels", "patterns")
 * info Module information structure to fill
 * Result indicating success or failure
 */
AsthraResult Asthra_get_module_info(const char* module_name, AsthraConcurrencyModuleInfo* info);

// =============================================================================
// CONVENIENCE INITIALIZATION FUNCTIONS
// =============================================================================

/**
 * Initialize concurrency bridge with default settings
 * Result indicating success or failure
 */
AsthraResult Asthra_concurrency_bridge_init_default(void);

/**
 * Initialize concurrency bridge with custom configuration
 * config Configuration structure for all modules
 * Result indicating success or failure
 */
typedef struct {
    size_t max_tasks;        // Maximum concurrent tasks
    size_t max_callbacks;    // Maximum queued callbacks
    size_t max_channels;     // Maximum concurrent channels
    size_t max_worker_pools; // Maximum worker pools
    bool enable_statistics;  // Enable detailed statistics
    bool enable_debugging;   // Enable debugging features
} AsthraConcurrencyConfig;

AsthraResult Asthra_concurrency_bridge_init_with_config(const AsthraConcurrencyConfig* config);

// =============================================================================
// BACKWARD COMPATIBILITY MACROS
// =============================================================================

// For code that was using the original asthra_concurrency_bridge.h,
// these macros provide seamless compatibility

// Original initialization function (now calls modular version)
#define ASTHRA_CONCURRENCY_BRIDGE_INIT(max_tasks, max_callbacks)                                   \
    Asthra_concurrency_bridge_init(max_tasks, max_callbacks)

// Original cleanup function (now calls modular version)
#define ASTHRA_CONCURRENCY_BRIDGE_CLEANUP() Asthra_concurrency_bridge_cleanup()

// Original statistics function (now returns unified stats)
#define ASTHRA_GET_CONCURRENCY_STATS() Asthra_get_concurrency_stats()

// =============================================================================
// PHASE 2 COMPLETION VERIFICATION
// =============================================================================

/**
 * Verify that Phase 2 modularization is complete and functional
 * Result indicating success or any missing components
 */
AsthraResult Asthra_verify_phase2_completion(void);

/**
 * Get Phase 2 modularization status
 * atomics_ready Atomics module status
 * tasks_ready Tasks module status
 * sync_ready Synchronization module status
 * channels_ready Channels module status
 * patterns_ready Patterns module status
 * Overall readiness status
 */
bool Asthra_get_phase2_status(bool* atomics_ready, bool* tasks_ready, 
                             bool* sync_ready, bool* channels_ready, 
                             bool* patterns_ready);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_BRIDGE_MODULAR_H
