/**
 * Asthra Concurrency Callback System v1.2
 * Thread-Safe Callback Queue Management and Processing
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * IMPLEMENTATION FEATURES:
 * - Thread-safe callback queue with atomic operations
 * - Priority-based callback processing
 * - Lock-free enqueue/dequeue operations where possible
 * - Graceful shutdown and cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>

// C17 modernization includes
#include <stdatomic.h>

#include "asthra_concurrency_bridge_modular.h"

// Forward declarations for internal functions
extern AsthraConcurrencyBridge* asthra_concurrency_get_bridge(void);
extern bool asthra_concurrency_is_initialized(void);
extern AsthraResult asthra_concurrency_create_error_result(AsthraConcurrencyErrorCode code, const char *message);
extern uint64_t asthra_concurrency_get_timestamp_ms(void);
extern size_t asthra_concurrency_atomic_load_size(asthra_concurrency_atomic_size_t *size_ptr);
extern size_t asthra_concurrency_atomic_increment_size(asthra_concurrency_atomic_size_t *size_ptr);
extern size_t asthra_concurrency_atomic_decrement_size(asthra_concurrency_atomic_size_t *size_ptr);
extern uint64_t asthra_concurrency_atomic_increment_counter(asthra_concurrency_atomic_counter_t *counter);

// =============================================================================
// CALLBACK SYSTEM IMPLEMENTATION
// =============================================================================

AsthraResult Asthra_enqueue_callback(AsthraConcurrencyCallbackFunction func, void* data, 
                                    size_t data_size, void* context, uint32_t priority) {
    if (!asthra_concurrency_is_initialized()) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INIT_FAILED, 
                                                     "Concurrency bridge not initialized");
    }
    
    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
    
    if (atomic_load_explicit(&bridge->global_callback_queue.shutdown, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INIT_FAILED, 
                                                     "Callback queue is shutting down");
    }
    
    // Check queue size limit
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&bridge->global_callback_queue.queue_mutex);
    if (asthra_concurrency_atomic_load_size(&bridge->global_callback_queue.size) >= bridge->max_callbacks) {
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_CALLBACK_QUEUE_FULL, 
                                                     "Callback queue is full");
    }
    
    // Allocate callback entry
    AsthraConcurrencyCallbackEntry *entry = malloc(sizeof(AsthraConcurrencyCallbackEntry));
    if (!entry) {
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR, 
                                                     "Failed to allocate callback entry");
    }
    
    // Copy data if provided
    void *data_copy = NULL;
    if (data && data_size > 0) {
        data_copy = malloc(data_size);
        if (!data_copy) {
            free(entry);
            ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
            return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR, 
                                                         "Failed to allocate callback data");
        }
        memcpy(data_copy, data, data_size);
    }
    
    // Initialize entry
    entry->func = func;
    entry->data = data_copy;
    entry->data_size = data_size;
    entry->context = context;
    atomic_init(&entry->timestamp, asthra_concurrency_get_timestamp_ms());
    atomic_init(&entry->priority, priority);
    atomic_init(&entry->next, NULL);
    
    // Add to queue
    AsthraConcurrencyCallbackEntry *tail = atomic_load_explicit(&bridge->global_callback_queue.tail, 
                                                               ASTHRA_MEMORY_ORDER_ACQUIRE);
    if (!tail) {
        // Empty queue
        atomic_store_explicit(&bridge->global_callback_queue.head, entry, ASTHRA_MEMORY_ORDER_RELEASE);
        atomic_store_explicit(&bridge->global_callback_queue.tail, entry, ASTHRA_MEMORY_ORDER_RELEASE);
    } else {
        // Add to tail
        atomic_store_explicit(&tail->next, entry, ASTHRA_MEMORY_ORDER_RELEASE);
        atomic_store_explicit(&bridge->global_callback_queue.tail, entry, ASTHRA_MEMORY_ORDER_RELEASE);
    }
    
    asthra_concurrency_atomic_increment_size(&bridge->global_callback_queue.size);
    asthra_concurrency_atomic_increment_counter(&bridge->stats.callbacks_enqueued);
    atomic_fetch_add_explicit(&bridge->global_callback_queue.enqueue_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    
    // Signal waiting processors
    ASTHRA_CONCURRENCY_COND_SIGNAL(&bridge->global_callback_queue.process_cond);
    
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
    
    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
}

size_t Asthra_process_callbacks(size_t max_callbacks) {
    if (!asthra_concurrency_is_initialized()) {
        return 0;
    }
    
    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
    size_t processed = 0;
    
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&bridge->global_callback_queue.process_mutex);
    
    while (processed < max_callbacks && 
           asthra_concurrency_atomic_load_size(&bridge->global_callback_queue.size) > 0) {
        
        ASTHRA_CONCURRENCY_MUTEX_LOCK(&bridge->global_callback_queue.queue_mutex);
        
        // Get head entry
        AsthraConcurrencyCallbackEntry *entry = 
            atomic_load_explicit(&bridge->global_callback_queue.head, ASTHRA_MEMORY_ORDER_ACQUIRE);
        if (!entry) {
            ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
            break;
        }
        
        // Remove from queue
        AsthraConcurrencyCallbackEntry *next = atomic_load_explicit(&entry->next, ASTHRA_MEMORY_ORDER_ACQUIRE);
        atomic_store_explicit(&bridge->global_callback_queue.head, next, ASTHRA_MEMORY_ORDER_RELEASE);
        if (!next) {
            atomic_store_explicit(&bridge->global_callback_queue.tail, NULL, ASTHRA_MEMORY_ORDER_RELEASE);
        }
        asthra_concurrency_atomic_decrement_size(&bridge->global_callback_queue.size);
        atomic_fetch_add_explicit(&bridge->global_callback_queue.dequeue_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
        
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
        
        // Execute callback
        if (entry->func) {
            AsthraResult result = entry->func(entry->data, entry->data_size, entry->context);
            // Don't free GC-managed callback results - let the GC handle cleanup
            // Could log or handle result here if needed
            (void)result; // Suppress unused variable warning
        }
        
        // Cleanup entry
        if (entry->data) {
            free(entry->data);
        }
        free(entry);
        
        processed++;
        asthra_concurrency_atomic_increment_counter(&bridge->stats.callbacks_processed);
    }
    
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.process_mutex);
    
    return processed;
}

// =============================================================================
// CALLBACK QUEUE MANAGEMENT
// =============================================================================

size_t Asthra_get_callback_queue_size(void) {
    if (!asthra_concurrency_is_initialized()) {
        return 0;
    }
    
    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
    return asthra_concurrency_atomic_load_size(&bridge->global_callback_queue.size);
}

bool Asthra_is_callback_queue_empty(void) {
    return Asthra_get_callback_queue_size() == 0;
}

bool Asthra_is_callback_queue_full(void) {
    if (!asthra_concurrency_is_initialized()) {
        return true;
    }
    
    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
    return asthra_concurrency_atomic_load_size(&bridge->global_callback_queue.size) >= bridge->max_callbacks;
}

void Asthra_clear_callback_queue(void) {
    if (!asthra_concurrency_is_initialized()) {
        return;
    }
    
    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
    
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&bridge->global_callback_queue.queue_mutex);
    
    // Process all remaining callbacks to clean up
    AsthraConcurrencyCallbackEntry *current = atomic_load_explicit(&bridge->global_callback_queue.head, 
                                                                  ASTHRA_MEMORY_ORDER_ACQUIRE);
    while (current) {
        AsthraConcurrencyCallbackEntry *next = atomic_load_explicit(&current->next, ASTHRA_MEMORY_ORDER_ACQUIRE);
        
        // Cleanup entry data
        if (current->data) {
            free(current->data);
        }
        free(current);
        
        atomic_fetch_add_explicit(&bridge->global_callback_queue.drop_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
        asthra_concurrency_atomic_increment_counter(&bridge->stats.callbacks_dropped);
        
        current = next;
    }
    
    // Reset queue state
    atomic_store_explicit(&bridge->global_callback_queue.head, NULL, ASTHRA_MEMORY_ORDER_RELEASE);
    atomic_store_explicit(&bridge->global_callback_queue.tail, NULL, ASTHRA_MEMORY_ORDER_RELEASE);
    atomic_store_explicit(&bridge->global_callback_queue.size, 0, ASTHRA_MEMORY_ORDER_RELEASE);
    
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
}

// =============================================================================
// PRIORITY CALLBACK PROCESSING
// =============================================================================

AsthraResult Asthra_enqueue_priority_callback(AsthraConcurrencyCallbackFunction func, void* data, 
                                             size_t data_size, void* context, uint32_t priority, 
                                             bool insert_at_front) {
    if (!asthra_concurrency_is_initialized()) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INIT_FAILED, 
                                                     "Concurrency bridge not initialized");
    }
    
    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
    
    if (atomic_load_explicit(&bridge->global_callback_queue.shutdown, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INIT_FAILED, 
                                                     "Callback queue is shutting down");
    }
    
    // Check queue size limit
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&bridge->global_callback_queue.queue_mutex);
    if (asthra_concurrency_atomic_load_size(&bridge->global_callback_queue.size) >= bridge->max_callbacks) {
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_CALLBACK_QUEUE_FULL, 
                                                     "Callback queue is full");
    }
    
    // Allocate callback entry
    AsthraConcurrencyCallbackEntry *entry = malloc(sizeof(AsthraConcurrencyCallbackEntry));
    if (!entry) {
        ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR, 
                                                     "Failed to allocate callback entry");
    }
    
    // Copy data if provided
    void *data_copy = NULL;
    if (data && data_size > 0) {
        data_copy = malloc(data_size);
        if (!data_copy) {
            free(entry);
            ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
            return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_SYSTEM_ERROR, 
                                                         "Failed to allocate callback data");
        }
        memcpy(data_copy, data, data_size);
    }
    
    // Initialize entry
    entry->func = func;
    entry->data = data_copy;
    entry->data_size = data_size;
    entry->context = context;
    atomic_init(&entry->timestamp, asthra_concurrency_get_timestamp_ms());
    atomic_init(&entry->priority, priority);
    atomic_init(&entry->next, NULL);
    
    if (insert_at_front) {
        // Insert at front for high priority
        AsthraConcurrencyCallbackEntry *head = atomic_load_explicit(&bridge->global_callback_queue.head, 
                                                                   ASTHRA_MEMORY_ORDER_ACQUIRE);
        atomic_store_explicit(&entry->next, head, ASTHRA_MEMORY_ORDER_RELEASE);
        atomic_store_explicit(&bridge->global_callback_queue.head, entry, ASTHRA_MEMORY_ORDER_RELEASE);
        
        if (!head) {
            // Was empty, update tail too
            atomic_store_explicit(&bridge->global_callback_queue.tail, entry, ASTHRA_MEMORY_ORDER_RELEASE);
        }
    } else {
        // Insert at appropriate position based on priority
        // For simplicity, this implementation just adds to the end
        // A full implementation would maintain priority ordering
        AsthraConcurrencyCallbackEntry *tail = atomic_load_explicit(&bridge->global_callback_queue.tail, 
                                                                   ASTHRA_MEMORY_ORDER_ACQUIRE);
        if (!tail) {
            // Empty queue
            atomic_store_explicit(&bridge->global_callback_queue.head, entry, ASTHRA_MEMORY_ORDER_RELEASE);
            atomic_store_explicit(&bridge->global_callback_queue.tail, entry, ASTHRA_MEMORY_ORDER_RELEASE);
        } else {
            // Add to tail
            atomic_store_explicit(&tail->next, entry, ASTHRA_MEMORY_ORDER_RELEASE);
            atomic_store_explicit(&bridge->global_callback_queue.tail, entry, ASTHRA_MEMORY_ORDER_RELEASE);
        }
    }
    
    asthra_concurrency_atomic_increment_size(&bridge->global_callback_queue.size);
    asthra_concurrency_atomic_increment_counter(&bridge->stats.callbacks_enqueued);
    atomic_fetch_add_explicit(&bridge->global_callback_queue.enqueue_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    
    // Signal waiting processors
    ASTHRA_CONCURRENCY_COND_SIGNAL(&bridge->global_callback_queue.process_cond);
    
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
    
    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
}

// =============================================================================
// CALLBACK QUEUE STATISTICS
// =============================================================================

AsthraConcurrencyCallbackQueueStats Asthra_get_callback_queue_stats(void) {
    AsthraConcurrencyCallbackQueueStats stats = {0};
    
    if (!asthra_concurrency_is_initialized()) {
        return stats;
    }
    
    AsthraConcurrencyBridge *bridge = asthra_concurrency_get_bridge();
    
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&bridge->global_callback_queue.queue_mutex);
    
    stats.current_size = asthra_concurrency_atomic_load_size(&bridge->global_callback_queue.size);
    stats.max_capacity = bridge->max_callbacks;
    stats.total_enqueued = atomic_load_explicit(&bridge->global_callback_queue.enqueue_count, 
                                               ASTHRA_MEMORY_ORDER_ACQUIRE);
    stats.total_processed = atomic_load_explicit(&bridge->global_callback_queue.dequeue_count, 
                                                ASTHRA_MEMORY_ORDER_ACQUIRE);
    stats.total_dropped = atomic_load_explicit(&bridge->global_callback_queue.drop_count, 
                                              ASTHRA_MEMORY_ORDER_ACQUIRE);
    stats.is_shutdown = atomic_load_explicit(&bridge->global_callback_queue.shutdown, 
                                            ASTHRA_MEMORY_ORDER_ACQUIRE);
    
    // Calculate oldest callback age
    AsthraConcurrencyCallbackEntry *head = atomic_load_explicit(&bridge->global_callback_queue.head, 
                                                               ASTHRA_MEMORY_ORDER_ACQUIRE);
    if (head) {
        uint64_t current_time = asthra_concurrency_get_timestamp_ms();
        uint64_t head_timestamp = atomic_load_explicit(&head->timestamp, ASTHRA_MEMORY_ORDER_ACQUIRE);
        stats.oldest_callback_age_ms = current_time - head_timestamp;
    } else {
        stats.oldest_callback_age_ms = 0;
    }
    
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&bridge->global_callback_queue.queue_mutex);
    
    return stats;
}

void Asthra_dump_callback_queue_stats(FILE* output) {
    if (!output) {
        output = stdout;
    }
    
    AsthraConcurrencyCallbackQueueStats stats = Asthra_get_callback_queue_stats();
    
    fprintf(output, "=== Callback Queue Statistics ===\n");
    fprintf(output, "Current Size: %zu / %zu\n", stats.current_size, stats.max_capacity);
    fprintf(output, "Total Enqueued: %llu\n", (unsigned long long)stats.total_enqueued);
    fprintf(output, "Total Processed: %llu\n", (unsigned long long)stats.total_processed);
    fprintf(output, "Total Dropped: %llu\n", (unsigned long long)stats.total_dropped);
    fprintf(output, "Is Shutdown: %s\n", stats.is_shutdown ? "Yes" : "No");
    fprintf(output, "Oldest Callback Age: %llu ms\n", (unsigned long long)stats.oldest_callback_age_ms);
    
    if (stats.current_size > 0) {
        double utilization = (double)stats.current_size / (double)stats.max_capacity * 100.0;
        fprintf(output, "Queue Utilization: %.1f%%\n", utilization);
    }
}
