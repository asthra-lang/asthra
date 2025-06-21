/**
 * Asthra Concurrency Channels - Channels and Communication Primitives
 * Part of the Asthra Runtime Modularization Plan - Phase 2
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * DESIGN GOALS:
 * - Channel definitions and operations
 * - Select operations for multi-channel coordination
 * - Buffered/unbuffered channel support
 * - Callback queue systems for C interop
 */

#ifndef ASTHRA_CONCURRENCY_CHANNELS_H
#define ASTHRA_CONCURRENCY_CHANNELS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "asthra_concurrency_atomics.h"
#include "asthra_concurrency_sync.h"
#include "../asthra_runtime.h"
#include "../asthra_ffi_memory.h"

#ifdef __cplusplus
extern "C" {

// Asthra_channel_len
;


// Asthra_channel_cap
;


// Asthra_channel_is_empty
;


// Asthra_channel_is_full
;


// Asthra_channel_get_info
;


// Asthra_channel_dump_info
;

#endif

// =============================================================================
// CALLBACK SYSTEM FOR C INTEROP
// =============================================================================

/**
 * Callback function signature for C-to-Asthra communication
 */
typedef AsthraResult (*AsthraConcurrencyCallbackFunction)(void *data, size_t data_size, void *context);

/**
 * Callback entry for the atomic callback queue
 */
typedef struct AsthraConcurrencyCallbackEntry {
    AsthraConcurrencyCallbackFunction func; // Callback function
    void *data;                          // Callback data
    size_t data_size;                    // Size of callback data
    void *context;                       // Additional context
    asthra_concurrency_atomic_counter_t timestamp; // Atomic timestamp
    asthra_concurrency_atomic_int priority;        // Atomic priority
    _Atomic(struct AsthraConcurrencyCallbackEntry*) next; // Atomic next pointer
} AsthraConcurrencyCallbackEntry;

/**
 * Lock-free callback queue for C-to-Asthra communication
 */
typedef struct {
    _Atomic(AsthraConcurrencyCallbackEntry*) head; // Atomic queue head
    _Atomic(AsthraConcurrencyCallbackEntry*) tail; // Atomic queue tail
    asthra_concurrency_atomic_size_t size;         // Atomic queue size
    asthra_concurrency_atomic_bool shutdown;       // Atomic shutdown flag
    asthra_concurrency_atomic_counter_t enqueue_count; // Total enqueued
    asthra_concurrency_atomic_counter_t dequeue_count; // Total dequeued
    asthra_concurrency_atomic_counter_t drop_count;    // Dropped callbacks
    asthra_concurrency_mutex_t queue_mutex;       // Hybrid queue mutex
    asthra_concurrency_mutex_t process_mutex;     // Protects processing (optional)
    asthra_concurrency_cond_t process_cond;       // Signals new callbacks
} AsthraConcurrencyCallbackQueue;

// =============================================================================
// CHANNEL DEFINITIONS
// =============================================================================

/**
 * Channel structure for inter-task communication
 */
typedef struct AsthraConcurrencyChannel {
    void *buffer;                        // Circular buffer for messages
    size_t element_size;                 // Size of each element
    size_t buffer_capacity;              // Maximum number of elements
    _Atomic(size_t) head;                // Head index (for reading)
    _Atomic(size_t) tail;                // Tail index (for writing)
    _Atomic(size_t) count;               // Current number of elements
    asthra_concurrency_mutex_t mutex;    // Protects buffer operations
    asthra_concurrency_cond_t not_empty; // Signals when data is available
    asthra_concurrency_cond_t not_full;  // Signals when space is available
    _Atomic(bool) is_closed;             // Channel closed flag
    char name[64];                       // Channel name for debugging
    asthra_concurrency_atomic_counter_t send_count; // Total send operations
    asthra_concurrency_atomic_counter_t recv_count; // Total receive operations
    asthra_concurrency_atomic_counter_t block_count; // Total blocking operations
} AsthraConcurrencyChannel;

// Type compatibility alias for stdlib integration
typedef AsthraConcurrencyChannel AsthraConcurrencyChannelHandle;

/**
 * Select context for multi-channel operations
 */
typedef struct {
    AsthraConcurrencyChannel **channels; // Array of channels to select on
    size_t channel_count;                // Number of channels
    bool *is_send_op;                    // Array indicating send vs recv operations
    void **send_values;                  // Values to send (for send operations)
    void **recv_buffers;                 // Buffers for received values
    int selected_index;                  // Index of selected channel (-1 if timeout)
    uint64_t timeout_ms;                 // Timeout in milliseconds (0 for no timeout)
    asthra_concurrency_atomic_bool ready; // Atomic ready flag
} AsthraConcurrencySelectContext;

// =============================================================================
// CALLBACK QUEUE OPERATIONS
// =============================================================================

/**
 * Initialize the callback system
 * @param max_callbacks Maximum number of queued callbacks
 * @return Result indicating success or failure
 */
AsthraResult Asthra_callback_system_init(size_t max_callbacks);

/**
 * Cleanup the callback system
 */
void Asthra_callback_system_cleanup(void);

/**
 * Enqueue a callback for processing
 * @param func Callback function to execute
 * @param data Data to pass to the callback
 * @param data_size Size of the data
 * @param context Additional context for the callback
 * @param priority Priority of the callback (higher numbers = higher priority)
 * @return Result indicating success or failure
 */
AsthraResult Asthra_enqueue_callback(AsthraConcurrencyCallbackFunction func, void* data, 
                                    size_t data_size, void* context, uint32_t priority);

/**
 * Process queued callbacks
 * @param max_callbacks Maximum number of callbacks to process
 * @return Number of callbacks processed
 */
size_t Asthra_process_callbacks(size_t max_callbacks);

/**
 * Get callback queue statistics
 * @param queue_size Current queue size
 * @param processed_count Total processed count
 * @return Result indicating success or failure
 */
AsthraResult Asthra_get_callback_stats(size_t* queue_size, uint64_t* processed_count);

// =============================================================================
// CHANNEL OPERATIONS
// =============================================================================

/**
 * Create a new channel
 * @param element_size Size of each element in bytes
 * @param buffer_capacity Maximum number of elements (0 for unbuffered)
 * @param name Optional name for debugging
 * @return New channel handle or NULL on failure
 */
AsthraConcurrencyChannel* Asthra_channel_create(size_t element_size, 
                                                size_t buffer_capacity, 
                                                const char* name);

/**
 * Send a value to a channel
 * @param channel Channel to send to
 * @param value Value to send
 * @param timeout_ms Timeout in milliseconds (0 for blocking)
 * @return Result indicating success, timeout, or failure
 */
AsthraResult Asthra_channel_send(AsthraConcurrencyChannel* channel, 
                                const void* value, 
                                uint64_t timeout_ms);

/**
 * Receive a value from a channel
 * @param channel Channel to receive from
 * @param buffer Buffer to store received value
 * @param timeout_ms Timeout in milliseconds (0 for blocking)
 * @return Result indicating success, timeout, or failure
 */
AsthraResult Asthra_channel_recv(AsthraConcurrencyChannel* channel, 
                                void* buffer, 
                                uint64_t timeout_ms);

/**
 * Try to send without blocking
 * @param channel Channel to send to
 * @param value Value to send
 * @return Result indicating success or would-block
 */
AsthraResult Asthra_channel_try_send(AsthraConcurrencyChannel* channel, 
                                     const void* value);

/**
 * Try to receive without blocking
 * @param channel Channel to receive from
 * @param buffer Buffer to store received value
 * @return Result indicating success or would-block
 */
AsthraResult Asthra_channel_try_recv(AsthraConcurrencyChannel* channel, 
                                     void* buffer);

/**
 * Close a channel
 * @param channel Channel to close
 */
void Asthra_channel_close(AsthraConcurrencyChannel* channel);

/**
 * Destroy a channel
 * @param channel Channel to destroy
 */
void Asthra_channel_destroy(AsthraConcurrencyChannel* channel);

/**
 * Check if a channel is closed
 * @param channel Channel to check
 * @return true if closed, false otherwise
 */
bool Asthra_channel_is_closed(AsthraConcurrencyChannel* channel);

/**
 * Get channel statistics
 * @param channel Channel to query
 * @param count Current number of elements
 * @param capacity Maximum capacity
 * @return true if successful, false otherwise
 */
bool Asthra_channel_get_stats(AsthraConcurrencyChannel* channel, 
                             size_t* count, 
                             size_t* capacity);

// =============================================================================
// SELECT OPERATIONS
// =============================================================================

/**
 * Create a select context for multi-channel operations
 * @param max_channels Maximum number of channels to select on
 * @return New select context or NULL on failure
 */
AsthraConcurrencySelectContext* Asthra_select_context_create(size_t max_channels);

/**
 * Add a receive operation to select context
 * @param context Select context
 * @param channel Channel to receive from
 * @param buffer Buffer for received value
 * @return true if successful, false otherwise
 */
bool Asthra_select_add_recv(AsthraConcurrencySelectContext* context,
                           AsthraConcurrencyChannel* channel,
                           void* buffer);

/**
 * Add a send operation to select context
 * @param context Select context
 * @param channel Channel to send to
 * @param value Value to send
 * @return true if successful, false otherwise
 */
bool Asthra_select_add_send(AsthraConcurrencySelectContext* context,
                           AsthraConcurrencyChannel* channel,
                           const void* value);

/**
 * Execute select operation
 * @param context Select context
 * @param timeout_ms Timeout in milliseconds (0 for no timeout)
 * @return Index of selected operation, -1 on timeout, -2 on error
 */
int Asthra_select_execute(AsthraConcurrencySelectContext* context, 
                         uint64_t timeout_ms);

/**
 * Reset select context for reuse
 * @param context Select context to reset
 */
void Asthra_select_context_reset(AsthraConcurrencySelectContext* context);

/**
 * Destroy select context
 * @param context Context to destroy
 */
void Asthra_select_context_destroy(AsthraConcurrencySelectContext* context);

// =============================================================================
// CHANNEL UTILITIES
// =============================================================================

/**
 * Create a string concatenation context for concurrent operations
 */
typedef struct {
    AsthraFFIString *strings;            // Array of strings to concatenate
    size_t string_count;                 // Number of strings
    AsthraMemoryZoneHint zone_hint;      // Target memory zone
    bool preserve_order;                 // Maintain order during concurrent ops
} AsthraConcurrencyStringConcatContext;

/**
 * Create a slice operation context for concurrent operations
 */
typedef struct {
    AsthraFFISliceHeader slice;          // Source slice
    size_t start_index;                  // Start index for operation
    size_t end_index;                    // End index for operation
    void *operation_data;                // Operation-specific data
    AsthraMemoryZoneHint zone_hint;      // Target memory zone
} AsthraConcurrencySliceOpContext;

/**
 * Perform concurrent string concatenation
 * @param context String concatenation context
 * @return Result with concatenated string or error
 */
AsthraResult Asthra_string_concat_concurrent(const AsthraConcurrencyStringConcatContext* context);

/**
 * Perform concurrent string interpolation
 * @param template Template string with placeholders
 * @param args Variant array of arguments
 * @param zone_hint Memory zone hint
 * @return Result with interpolated string or error
 */
AsthraResult Asthra_string_interpolate_concurrent(const char* template, 
                                                 AsthraVariantArray args, 
                                                 AsthraMemoryZoneHint zone_hint);

/**
 * Create a slice concurrently
 * @param element_size Size of each element
 * @param len Initial length
 * @param cap Initial capacity
 * @param zone_hint Memory zone hint
 * @return Result with slice or error
 */
AsthraResult Asthra_slice_create_concurrent(size_t element_size, size_t len, 
                                           size_t cap, AsthraMemoryZoneHint zone_hint);

/**
 * Create a subslice concurrently
 * @param context Slice operation context
 * @return Result with subslice or error
 */
AsthraResult Asthra_slice_subslice_concurrent(const AsthraConcurrencySliceOpContext* context);

/**
 * Get an element from a slice concurrently
 * @param slice Source slice
 * @param index Element index
 * @param out_element Output buffer for element
 * @return Result indicating success or failure
 */
AsthraResult Asthra_slice_get_element_concurrent(AsthraFFISliceHeader slice, 
                                               size_t index, void* out_element);

/**
 * Set an element in a slice concurrently
 * @param slice Target slice
 * @param index Element index
 * @param element Element data to set
 * @return Result indicating success or failure
 */
AsthraResult Asthra_slice_set_element_concurrent(AsthraFFISliceHeader slice, 
                                               size_t index, const void* element);

// =============================================================================
// CHANNEL STATISTICS
// =============================================================================

/**
 * Channel system statistics
 */
typedef struct {
    asthra_concurrency_atomic_counter_t channels_created; // Total channels created
    asthra_concurrency_atomic_counter_t channels_active;  // Currently active channels
    asthra_concurrency_atomic_counter_t total_sends;      // Total send operations
    asthra_concurrency_atomic_counter_t total_receives;   // Total receive operations
    asthra_concurrency_atomic_counter_t blocked_sends;    // Total blocked sends
    asthra_concurrency_atomic_counter_t blocked_receives; // Total blocked receives
    asthra_concurrency_atomic_counter_t callbacks_enqueued; // Callbacks enqueued
    asthra_concurrency_atomic_counter_t callbacks_processed; // Callbacks processed
    asthra_concurrency_atomic_counter_t callbacks_dropped; // Dropped callbacks
} AsthraConcurrencyChannelStats;

/**
 * Get channel system statistics
 * @return Channel statistics structure
 */
AsthraConcurrencyChannelStats Asthra_get_channel_stats(void);

/**
 * Reset channel statistics
 */
void Asthra_reset_channel_stats(void);

// =============================================================================
// ERROR CODES
// =============================================================================

#define ASTHRA_CHANNEL_ERROR_BASE 0x3000

typedef enum {
    ASTHRA_CHANNEL_ERROR_NONE = 0,
    ASTHRA_CHANNEL_ERROR_INIT_FAILED = ASTHRA_CHANNEL_ERROR_BASE + 1,
    ASTHRA_CHANNEL_ERROR_CHANNEL_CLOSED = ASTHRA_CHANNEL_ERROR_BASE + 2,
    ASTHRA_CHANNEL_ERROR_WOULD_BLOCK = ASTHRA_CHANNEL_ERROR_BASE + 3,
    ASTHRA_CHANNEL_ERROR_TIMEOUT = ASTHRA_CHANNEL_ERROR_BASE + 4,
    ASTHRA_CHANNEL_ERROR_QUEUE_FULL = ASTHRA_CHANNEL_ERROR_BASE + 5,
    ASTHRA_CHANNEL_ERROR_INVALID_HANDLE = ASTHRA_CHANNEL_ERROR_BASE + 6,
    ASTHRA_CHANNEL_ERROR_SELECT_FAILED = ASTHRA_CHANNEL_ERROR_BASE + 7,
    ASTHRA_CHANNEL_ERROR_SYSTEM_ERROR = ASTHRA_CHANNEL_ERROR_BASE + 8
} AsthraConcurrencyChannelErrorCode;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_CHANNELS_H 
