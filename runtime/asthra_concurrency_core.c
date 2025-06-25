/**
 * Asthra Concurrency Bridge Core v1.2
 * Core Bridge Implementation with C17 Atomic Operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * IMPLEMENTATION FEATURES:
 * - Bridge initialization and cleanup
 * - C17 atomic utility functions with explicit memory ordering
 * - Global state management with thread-safe operations
 * - Statistics and monitoring infrastructure
 */

#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#ifndef ASTHRA_CONCURRENCY_HAS_C17_THREADS
#define ASTHRA_CONCURRENCY_HAS_C17_THREADS 0
#endif

// C17 modernization includes
#include <inttypes.h>
#include <stdatomic.h>

// Only include threads.h if available
#if ASTHRA_CONCURRENCY_HAS_C17_THREADS
#include <threads.h>
#endif

#include "asthra_concurrency_bridge_modular.h"
#include "asthra_concurrency_core.h"

// =============================================================================
// GLOBAL STATE WITH C17 ATOMICS - PHASE 1.2 ENHANCEMENTS
// =============================================================================

static AsthraConcurrencyBridge g_bridge = {0};
static ASTHRA_CONCURRENCY_THREAD_LOCAL AsthraConcurrencyThreadData *g_thread_data = NULL;

// C17 one-time initialization - Phase 1.2 enhancement
static asthra_concurrency_once_flag_t g_init_once;
static bool g_init_once_initialized = false;
static bool g_bridge_init_done = false;

// =============================================================================
// C17 ATOMIC UTILITY FUNCTIONS - PHASE 1.2 ENHANCEMENTS
// =============================================================================

// C17 atomic increment with explicit memory ordering - Phase 1.2
uint64_t
asthra_concurrency_atomic_increment_counter_explicit(asthra_concurrency_atomic_counter_t *counter,
                                                     memory_order order) {
    return atomic_fetch_add_explicit(counter, 1, order);
}

// C17 atomic decrement with explicit memory ordering - Phase 1.2
uint64_t
asthra_concurrency_atomic_decrement_counter_explicit(asthra_concurrency_atomic_counter_t *counter,
                                                     memory_order order) {
    return atomic_fetch_sub_explicit(counter, 1, order);
}

// C17 atomic load with explicit memory ordering - Phase 1.2
uint64_t
asthra_concurrency_atomic_load_counter_explicit(asthra_concurrency_atomic_counter_t *counter,
                                                memory_order order) {
    return atomic_load_explicit(counter, order);
}

// C17 atomic store with explicit memory ordering - Phase 1.2
void asthra_concurrency_atomic_store_counter_explicit(asthra_concurrency_atomic_counter_t *counter,
                                                      uint64_t value, memory_order order) {
    atomic_store_explicit(counter, value, order);
}

// C17 atomic compare-and-swap for lock-free operations - Phase 1.2
bool asthra_concurrency_atomic_cas_ptr_explicit(_Atomic(void *) *ptr, void **expected,
                                                void *desired, memory_order success_order,
                                                memory_order failure_order) {
    return atomic_compare_exchange_weak_explicit(ptr, expected, desired, success_order,
                                                 failure_order);
}

// C17 atomic size operations for compatibility with different atomic types
size_t asthra_concurrency_atomic_load_size(asthra_concurrency_atomic_size_t *size_ptr) {
    return atomic_load_explicit(size_ptr, ASTHRA_MEMORY_ORDER_ACQUIRE);
}

size_t asthra_concurrency_atomic_increment_size(asthra_concurrency_atomic_size_t *size_ptr) {
    return atomic_fetch_add_explicit(size_ptr, 1, ASTHRA_MEMORY_ORDER_RELAXED);
}

size_t asthra_concurrency_atomic_decrement_size(asthra_concurrency_atomic_size_t *size_ptr) {
    return atomic_fetch_sub_explicit(size_ptr, 1, ASTHRA_MEMORY_ORDER_RELAXED);
}

// Legacy atomic operations for compatibility
uint64_t asthra_concurrency_atomic_increment_counter(asthra_concurrency_atomic_counter_t *counter) {
    return asthra_concurrency_atomic_increment_counter_explicit(counter,
                                                                ASTHRA_MEMORY_ORDER_RELAXED);
}

uint64_t asthra_concurrency_atomic_decrement_counter(asthra_concurrency_atomic_counter_t *counter) {
    return asthra_concurrency_atomic_decrement_counter_explicit(counter,
                                                                ASTHRA_MEMORY_ORDER_RELAXED);
}

uint64_t asthra_concurrency_atomic_load_counter(asthra_concurrency_atomic_counter_t *counter) {
    return asthra_concurrency_atomic_load_counter_explicit(counter, ASTHRA_MEMORY_ORDER_ACQUIRE);
}

void asthra_concurrency_atomic_store_counter(asthra_concurrency_atomic_counter_t *counter,
                                             uint64_t value) {
    asthra_concurrency_atomic_store_counter_explicit(counter, value, ASTHRA_MEMORY_ORDER_RELEASE);
}

bool asthra_concurrency_atomic_cas_ptr(_Atomic(void *) *ptr, void **expected, void *desired) {
    return asthra_concurrency_atomic_cas_ptr_explicit(
        ptr, expected, desired, ASTHRA_MEMORY_ORDER_ACQ_REL, ASTHRA_MEMORY_ORDER_ACQUIRE);
}

// =============================================================================
// LOCK-FREE DATA STRUCTURE IMPLEMENTATIONS - PHASE 1.2 NEW FEATURES
// =============================================================================

// Lock-free stack implementation using C17 atomics
bool asthra_concurrency_lock_free_stack_push(AsthraConcurrencyLockFreeStack *stack, void *item) {
    if (!stack || !item)
        return false;

    void *old_top = atomic_load_explicit(&stack->top, ASTHRA_MEMORY_ORDER_ACQUIRE);
    void *new_top = item;

    // Store the old top as the next pointer in the new item
    // This assumes the item has a next pointer at the beginning
    *((void **)item) = old_top;

    // Attempt to update the top pointer
    while (!atomic_compare_exchange_weak_explicit(
        &stack->top, &old_top, new_top, ASTHRA_MEMORY_ORDER_RELEASE, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        // Update the next pointer with the new old_top
        *((void **)item) = old_top;
    }

    atomic_fetch_add_explicit(&stack->size, 1, ASTHRA_MEMORY_ORDER_RELAXED);
    atomic_fetch_add_explicit(&stack->push_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);

    return true;
}

void *asthra_concurrency_lock_free_stack_pop(AsthraConcurrencyLockFreeStack *stack) {
    if (!stack)
        return NULL;

    void *old_top = atomic_load_explicit(&stack->top, ASTHRA_MEMORY_ORDER_ACQUIRE);

    while (old_top != NULL) {
        // Get the next pointer from the current top item
        void *new_top = *((void **)old_top);

        // Attempt to update the top pointer
        if (atomic_compare_exchange_weak_explicit(&stack->top, &old_top, new_top,
                                                  ASTHRA_MEMORY_ORDER_RELEASE,
                                                  ASTHRA_MEMORY_ORDER_ACQUIRE)) {
            atomic_fetch_sub_explicit(&stack->size, 1, ASTHRA_MEMORY_ORDER_RELAXED);
            atomic_fetch_add_explicit(&stack->pop_count, 1, ASTHRA_MEMORY_ORDER_RELAXED);
            return old_top;
        }
        // old_top was updated by the failed CAS, try again
    }

    return NULL; // Stack is empty
}

// =============================================================================
// UTILITY FUNCTIONS - PHASE 1.2 ENHANCEMENTS
// =============================================================================

uint64_t asthra_concurrency_get_timestamp_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

AsthraResult asthra_concurrency_create_error_result(AsthraConcurrencyErrorCode code,
                                                    const char *message) {
    AsthraFFIResult ffi_result =
        Asthra_result_err(code, message, "asthra_concurrency_bridge", NULL);

    // Convert FFI result to runtime result
    AsthraResult result;
    if (ffi_result.tag == ASTHRA_FFI_RESULT_ERR) {
        result = asthra_result_err(&ffi_result.data.err.error_code, sizeof(int), ASTHRA_TYPE_I32,
                                   ASTHRA_OWNERSHIP_GC);
    } else {
        result = asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    }
    return result;
}

// Phase 1.2: Thread creation wrapper for hybrid model
bool asthra_concurrency_create_thread_hybrid(asthra_concurrency_thread_t *thread,
                                             void *(*func)(void *), void *arg,
                                             const AsthraConcurrencyTaskSpawnOptions *options) {
#if ASTHRA_CONCURRENCY_HAS_C17_THREADS
    bool prefer_c17 = options && options->use_c17_threads;
    if (prefer_c17) {
        // Use C17 threads if preferred and available
        int (*c17_func)(void *) = (int (*)(void *))func;
        return ASTHRA_CONCURRENCY_THREAD_CREATE(thread, c17_func, arg);
    }
#else
    // Suppress unused parameter warning when C17 threads not available
    (void)options;
#endif

    // Fall back to POSIX threads
    return ASTHRA_CONCURRENCY_THREAD_CREATE(thread, func, arg);
}

// =============================================================================
// BRIDGE INITIALIZATION AND CLEANUP
// =============================================================================

// C17 one-time initialization function - Phase 1.2 enhancement
static void init_bridge_once(void) {
    // Initialize atomic counters using C17 atomic_init
    atomic_init(&g_bridge.initialized, false);
    atomic_init(&g_bridge.next_task_id, 1);
    atomic_init(&g_bridge.task_registry, NULL);
    atomic_init(&g_bridge.init_time, asthra_concurrency_get_timestamp_ms());

    // Initialize statistics with C17 atomic_init - Phase 1.2 enhancements
    atomic_init(&g_bridge.stats.tasks_spawned, 0);
    atomic_init(&g_bridge.stats.tasks_completed, 0);
    atomic_init(&g_bridge.stats.tasks_failed, 0);
    atomic_init(&g_bridge.stats.tasks_cancelled, 0);
    atomic_init(&g_bridge.stats.tasks_timeout, 0);
    atomic_init(&g_bridge.stats.callbacks_enqueued, 0);
    atomic_init(&g_bridge.stats.callbacks_processed, 0);
    atomic_init(&g_bridge.stats.callbacks_dropped, 0);
    atomic_init(&g_bridge.stats.threads_registered, 0);
    atomic_init(&g_bridge.stats.gc_roots_registered, 0);
    atomic_init(&g_bridge.stats.mutex_contentions, 0);
    atomic_init(&g_bridge.stats.rwlock_contentions, 0);
    atomic_init(&g_bridge.stats.string_operations, 0);
    atomic_init(&g_bridge.stats.slice_operations, 0);
    atomic_init(&g_bridge.stats.lock_free_operations, 0);
    atomic_init(&g_bridge.stats.memory_ordering_violations, 0);
}

AsthraResult Asthra_concurrency_bridge_init(size_t max_tasks, size_t max_callbacks) {
    // Simple approach: avoid pthread_once altogether for problematic platforms
    // Use atomic bool for one-time initialization instead
    if (!g_bridge_init_done) {
        // Initialize once flag if not already done
        if (!g_init_once_initialized) {
#if ASTHRA_CONCURRENCY_HAS_C17_THREADS
            g_init_once = ONCE_FLAG_INIT;
            g_init_once_initialized = true;
            // Use C17 call_once for thread-safe initialization
            ASTHRA_CONCURRENCY_CALL_ONCE(&g_init_once, init_bridge_once);
#else
            // For pthread systems with problematic PTHREAD_ONCE_INIT, just call directly
            // This is safe since we check g_bridge_init_done first
            init_bridge_once();
            g_init_once_initialized = true;
#endif
        }
        g_bridge_init_done = true;
    }

    // Check if already initialized using C17 atomic load with acquire ordering
    if (atomic_load_explicit(&g_bridge.initialized, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
    }

    // Initialize global state
    g_bridge.max_tasks = max_tasks;
    g_bridge.max_callbacks = max_callbacks;

    // Initialize hybrid mutex
    if (!ASTHRA_CONCURRENCY_MUTEX_INIT(&g_bridge.global_mutex)) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INIT_FAILED,
                                                      "Failed to initialize global mutex");
    }

    // Initialize callback queue with C17 atomic operations - Phase 1.2 enhancements
    atomic_init(&g_bridge.global_callback_queue.head, NULL);
    atomic_init(&g_bridge.global_callback_queue.tail, NULL);
    atomic_init(&g_bridge.global_callback_queue.size, 0);
    atomic_init(&g_bridge.global_callback_queue.shutdown, false);
    atomic_init(&g_bridge.global_callback_queue.enqueue_count, 0);
    atomic_init(&g_bridge.global_callback_queue.dequeue_count, 0);
    atomic_init(&g_bridge.global_callback_queue.drop_count, 0);

    if (!ASTHRA_CONCURRENCY_MUTEX_INIT(&g_bridge.global_callback_queue.queue_mutex) ||
        !ASTHRA_CONCURRENCY_MUTEX_INIT(&g_bridge.global_callback_queue.process_mutex) ||
        !ASTHRA_CONCURRENCY_COND_INIT(&g_bridge.global_callback_queue.process_cond)) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INIT_FAILED,
                                                      "Failed to initialize callback queue");
    }

    // Initialize thread registry with C17 atomic operations - Phase 1.2 enhancements
    atomic_init(&g_bridge.thread_registry.threads, NULL);
    atomic_init(&g_bridge.thread_registry.thread_count, 0);
    atomic_init(&g_bridge.thread_registry.shutdown, false);
    atomic_init(&g_bridge.thread_registry.total_registered, 0);

    if (!ASTHRA_CONCURRENCY_MUTEX_INIT(&g_bridge.thread_registry.registry_mutex)) {
        return asthra_concurrency_create_error_result(ASTHRA_CONCURRENCY_ERROR_INIT_FAILED,
                                                      "Failed to initialize thread registry");
    }

    // Mark as initialized using C17 atomic store with release ordering
    atomic_store_explicit(&g_bridge.initialized, true, ASTHRA_MEMORY_ORDER_RELEASE);

    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
}

void Asthra_concurrency_bridge_cleanup(void) {
    if (!atomic_load_explicit(&g_bridge.initialized, ASTHRA_MEMORY_ORDER_ACQUIRE)) {
        return;
    }

    // Signal shutdown using release ordering
    atomic_store_explicit(&g_bridge.global_callback_queue.shutdown, true,
                          ASTHRA_MEMORY_ORDER_RELEASE);
    atomic_store_explicit(&g_bridge.thread_registry.shutdown, true, ASTHRA_MEMORY_ORDER_RELEASE);

    // Cleanup callback queue
    ASTHRA_CONCURRENCY_COND_BROADCAST(&g_bridge.global_callback_queue.process_cond);

    // Cleanup remaining tasks using atomic load
    AsthraConcurrencyTaskHandle *current =
        atomic_load_explicit(&g_bridge.task_registry, ASTHRA_MEMORY_ORDER_ACQUIRE);
    while (current) {
        AsthraConcurrencyTaskHandle *next = current->next;
        Asthra_task_handle_free(current);
        current = next;
    }

    // Cleanup thread registry
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&g_bridge.thread_registry.registry_mutex);
    AsthraConcurrencyThreadData *thread_data =
        atomic_load_explicit(&g_bridge.thread_registry.threads, ASTHRA_MEMORY_ORDER_ACQUIRE);
    while (thread_data) {
        AsthraConcurrencyThreadData *next = thread_data->next;
        if (thread_data->gc_roots) {
            free(thread_data->gc_roots);
        }
        free(thread_data);
        thread_data = next;
    }
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&g_bridge.thread_registry.registry_mutex);

    // Destroy mutexes and condition variables
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&g_bridge.global_mutex);
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&g_bridge.global_callback_queue.queue_mutex);
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&g_bridge.global_callback_queue.process_mutex);
    ASTHRA_CONCURRENCY_COND_DESTROY(&g_bridge.global_callback_queue.process_cond);
    ASTHRA_CONCURRENCY_MUTEX_DESTROY(&g_bridge.thread_registry.registry_mutex);

    // Mark as uninitialized
    atomic_store_explicit(&g_bridge.initialized, false, ASTHRA_MEMORY_ORDER_RELEASE);
}

// =============================================================================
// STATISTICS AND MONITORING
// =============================================================================

AsthraConcurrencyStats Asthra_get_concurrency_stats(void) {
    AsthraConcurrencyStats stats;
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&g_bridge.global_mutex);
    stats = g_bridge.stats;
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&g_bridge.global_mutex);
    return stats;
}

void Asthra_reset_concurrency_stats(void) {
    ASTHRA_CONCURRENCY_MUTEX_LOCK(&g_bridge.global_mutex);
    memset((void *)&g_bridge.stats, 0, sizeof(AsthraConcurrencyStats));
    ASTHRA_CONCURRENCY_MUTEX_UNLOCK(&g_bridge.global_mutex);
}

void Asthra_dump_concurrency_state(FILE *output) {
    if (!output) {
        output = stdout;
    }

    fprintf(output, "=== Asthra Concurrency Bridge State ===\n");
    fprintf(output, "Initialized: %s\n",
            atomic_load_explicit(&g_bridge.initialized, ASTHRA_MEMORY_ORDER_ACQUIRE) ? "Yes"
                                                                                     : "No");
    fprintf(output, "Max Tasks: %zu\n", g_bridge.max_tasks);
    fprintf(output, "Max Callbacks: %zu\n", g_bridge.max_callbacks);
    fprintf(output, "Next Task ID: %" PRIu64 "\n",
            asthra_concurrency_atomic_load_counter(&g_bridge.next_task_id));

    fprintf(output, "\n=== Statistics ===\n");
    AsthraConcurrencyStats stats = Asthra_get_concurrency_stats();
    fprintf(output, "Tasks Spawned: %" PRIu64 "\n",
            asthra_concurrency_atomic_load_counter(&stats.tasks_spawned));
    fprintf(output, "Tasks Completed: %" PRIu64 "\n",
            asthra_concurrency_atomic_load_counter(&stats.tasks_completed));
    fprintf(output, "Tasks Failed: %" PRIu64 "\n",
            asthra_concurrency_atomic_load_counter(&stats.tasks_failed));
    fprintf(output, "Callbacks Enqueued: %" PRIu64 "\n",
            asthra_concurrency_atomic_load_counter(&stats.callbacks_enqueued));
    fprintf(output, "Callbacks Processed: %" PRIu64 "\n",
            asthra_concurrency_atomic_load_counter(&stats.callbacks_processed));
    fprintf(output, "Threads Registered: %" PRIu64 "\n",
            asthra_concurrency_atomic_load_counter(&stats.threads_registered));
    fprintf(output, "Mutex Contentions: %" PRIu64 "\n",
            asthra_concurrency_atomic_load_counter(&stats.mutex_contentions));

    fprintf(output, "\n=== Callback Queue ===\n");
    fprintf(output, "Queue Size: %zu\n",
            asthra_concurrency_atomic_load_size(&g_bridge.global_callback_queue.size));
    fprintf(
        output, "Shutdown: %s\n",
        atomic_load_explicit(&g_bridge.global_callback_queue.shutdown, ASTHRA_MEMORY_ORDER_ACQUIRE)
            ? "Yes"
            : "No");

    fprintf(output, "\n=== Thread Registry ===\n");
    fprintf(output, "Registered Threads: %zu\n",
            asthra_concurrency_atomic_load_size(&g_bridge.thread_registry.thread_count));
    fprintf(output, "Registry Shutdown: %s\n",
            atomic_load_explicit(&g_bridge.thread_registry.shutdown, ASTHRA_MEMORY_ORDER_ACQUIRE)
                ? "Yes"
                : "No");
}

// =============================================================================
// BRIDGE STATE ACCESS FUNCTIONS
// =============================================================================

// Provide access to global bridge state for other modules
AsthraConcurrencyBridge *asthra_concurrency_get_bridge(void) {
    return &g_bridge;
}

AsthraConcurrencyThreadData *asthra_concurrency_get_thread_data(void) {
    return g_thread_data;
}

void asthra_concurrency_set_thread_data(AsthraConcurrencyThreadData *data) {
    g_thread_data = data;
}

bool asthra_concurrency_is_initialized(void) {
    return atomic_load_explicit(&g_bridge.initialized, ASTHRA_MEMORY_ORDER_ACQUIRE);
}
