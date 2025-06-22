/**
 * Asthra Programming Language Runtime Core v1.2
 * Core Runtime Initialization and Global State Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * IMPLEMENTATION FEATURES:
 * - Runtime initialization and cleanup
 * - Global state management with C17 atomics
 * - Error handling system
 * - Runtime statistics and monitoring
 * - Utility functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <assert.h>
#include <inttypes.h>

// C17 modernization includes
#include <stdatomic.h>
#include <stdalign.h>

// Only include threads.h if available
#if ASTHRA_HAS_C11_THREADS
#include <threads.h>
#endif

#include "asthra_runtime.h"

// =============================================================================
// C17 FEATURE COMPATIBILITY LAYER
// =============================================================================

// Atomic operation wrappers for compatibility
#if ASTHRA_HAS_ATOMICS
    #define ASTHRA_ATOMIC_LOAD(ptr) atomic_load(ptr)
    #define ASTHRA_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
    #define ASTHRA_ATOMIC_FETCH_ADD(ptr, val) atomic_fetch_add(ptr, val)
    #define ASTHRA_ATOMIC_FETCH_SUB(ptr, val) atomic_fetch_sub(ptr, val)
    #define ASTHRA_ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired) \
        atomic_compare_exchange_weak(ptr, expected, desired)
    #define ASTHRA_ATOMIC_FLAG_TEST_AND_SET(flag) atomic_flag_test_and_set(flag)
    #define ASTHRA_ATOMIC_FLAG_CLEAR(flag) atomic_flag_clear(flag)
    #define ASTHRA_MEMORY_ORDER_RELAXED memory_order_relaxed
    #define ASTHRA_MEMORY_ORDER_ACQUIRE memory_order_acquire
    #define ASTHRA_MEMORY_ORDER_RELEASE memory_order_release
#else
    // Fallback implementations for non-C17 compilers
    #define ASTHRA_ATOMIC_LOAD(ptr) (*(ptr))
    #define ASTHRA_ATOMIC_STORE(ptr, val) (*(ptr) = (val))
    #define ASTHRA_ATOMIC_FETCH_ADD(ptr, val) (__sync_fetch_and_add(ptr, val))
    #define ASTHRA_ATOMIC_FETCH_SUB(ptr, val) (__sync_fetch_and_sub(ptr, val))
    #define ASTHRA_ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired) \
        (__sync_bool_compare_and_swap(ptr, *(expected), desired))
    #define ASTHRA_ATOMIC_FLAG_TEST_AND_SET(flag) (__sync_lock_test_and_set(&(flag)->flag, true))
    #define ASTHRA_ATOMIC_FLAG_CLEAR(flag) (__sync_lock_release(&(flag)->flag))
    #define ASTHRA_MEMORY_ORDER_RELAXED 0
    #define ASTHRA_MEMORY_ORDER_ACQUIRE 0
    #define ASTHRA_MEMORY_ORDER_RELEASE 0
#endif

// Aligned allocation wrapper
#if ASTHRA_HAS_ALIGNED_ALLOC
    #define ASTHRA_ALIGNED_ALLOC(alignment, size) aligned_alloc(alignment, size)
    #define ASTHRA_ALIGNED_FREE(ptr) free(ptr)
#else
    // Fallback using posix_memalign
    static inline void* asthra_fallback_aligned_alloc(size_t alignment, size_t size) {
        void *ptr = NULL;
        if (posix_memalign(&ptr, alignment, size) != 0) {
            return NULL;
        }
        return ptr;
    }
    #define ASTHRA_ALIGNED_ALLOC(alignment, size) asthra_fallback_aligned_alloc(alignment, size)
    #define ASTHRA_ALIGNED_FREE(ptr) free(ptr)
#endif

// =============================================================================
// GLOBAL RUNTIME STATE WITH C17 ATOMICS
// =============================================================================

typedef struct {
    bool initialized;
    AsthraGCConfig gc_config;
    AsthraCsprng *global_csprng;
    AsthraScheduler *global_scheduler;
    AsthraError last_error;
    AsthraAtomicMemoryStats atomic_stats;  // C17 atomic statistics
    pthread_mutex_t error_mutex;
    
    // Command-line arguments storage
    int argc;
    char **argv;
    AsthraSliceHeader args_slice;  // Cached slice of strings for args() function
    bool args_slice_initialized;    // Whether args_slice has been created
} AsthraRuntimeState;

static AsthraRuntimeState g_runtime = {0};

// Thread-local GC state for C17 optimization
#if ASTHRA_HAS_THREAD_LOCAL
ASTHRA_THREAD_LOCAL AsthraThreadGCState *asthra_thread_gc_state = NULL;

// Thread-local GC state management
static void init_thread_gc_state(void) {
    if (asthra_thread_gc_state == NULL) {
        asthra_thread_gc_state = malloc(sizeof(AsthraThreadGCState));
        if (asthra_thread_gc_state) {
            *asthra_thread_gc_state = (AsthraThreadGCState){
                .gc_roots = NULL,
                .root_count = 0,
                .root_capacity = 0
            };
            #if ASTHRA_HAS_ATOMICS
            atomic_init(&asthra_thread_gc_state->allocations_since_gc, 0);
            #else
            asthra_thread_gc_state->allocations_since_gc = 0;
            #endif
        }
    }
}

static void cleanup_thread_gc_state(void) {
    if (asthra_thread_gc_state) {
        free(asthra_thread_gc_state->gc_roots);
        free(asthra_thread_gc_state);
        asthra_thread_gc_state = NULL;
    }
}
#endif

// =============================================================================
// RUNTIME INITIALIZATION AND CLEANUP WITH C17 FEATURES
// =============================================================================

int asthra_runtime_init(AsthraGCConfig *gc_config) {
    return asthra_runtime_init_with_args(gc_config, 0, NULL);
}

int asthra_runtime_init_with_args(AsthraGCConfig *gc_config, int argc, char **argv) {
    if (g_runtime.initialized) {
        return 0;
    }

    // Initialize default GC config if none provided using C17 compound literal
    if (gc_config) {
        g_runtime.gc_config = *gc_config;
    } else {
        g_runtime.gc_config = ASTHRA_DEFAULT_GC_CONFIG;
    }
    
    // Store command-line arguments
    g_runtime.argc = argc;
    g_runtime.argv = argv;
    g_runtime.args_slice_initialized = false;  // Will be created lazily when needed

    // Initialize atomic statistics with C17 atomic initialization
    #if ASTHRA_HAS_ATOMICS
    atomic_init(&g_runtime.atomic_stats.total_allocations, 0);
    atomic_init(&g_runtime.atomic_stats.total_deallocations, 0);
    atomic_init(&g_runtime.atomic_stats.current_memory_usage, 0);
    atomic_init(&g_runtime.atomic_stats.peak_memory_usage, 0);
    atomic_init(&g_runtime.atomic_stats.gc_collections, 0);
    atomic_init(&g_runtime.atomic_stats.gc_time_ms, 0);
    atomic_init(&g_runtime.atomic_stats.tasks_spawned, 0);
    atomic_init(&g_runtime.atomic_stats.tasks_completed, 0);
    atomic_init(&g_runtime.atomic_stats.ffi_calls, 0);
    atomic_init(&g_runtime.atomic_stats.gc_running, false);
    #else
    g_runtime.atomic_stats.total_allocations = 0;
    g_runtime.atomic_stats.total_deallocations = 0;
    g_runtime.atomic_stats.current_memory_usage = 0;
    g_runtime.atomic_stats.peak_memory_usage = 0;
    g_runtime.atomic_stats.gc_collections = 0;
    g_runtime.atomic_stats.gc_time_ms = 0;
    g_runtime.atomic_stats.tasks_spawned = 0;
    g_runtime.atomic_stats.tasks_completed = 0;
    g_runtime.atomic_stats.ffi_calls = 0;
    g_runtime.atomic_stats.gc_running = false;
    #endif

    // Initialize global mutexes
    if (pthread_mutex_init(&g_runtime.error_mutex, NULL) != 0) {
        return -1;
    }

    // Initialize global CSPRNG
    if (asthra_csprng_init_global() != 0) {
        return -1;
    }

    // Initialize thread-local GC state if supported
    #if ASTHRA_HAS_THREAD_LOCAL
    if (g_runtime.gc_config.use_thread_local_roots) {
        init_thread_gc_state();
    }
    #endif

    g_runtime.initialized = true;
    return 0;
}

void asthra_runtime_cleanup(void) {
    if (!g_runtime.initialized) {
        return;
    }

    // Cleanup args slice if it was created
    if (g_runtime.args_slice_initialized) {
        asthra_slice_free(g_runtime.args_slice);
        g_runtime.args_slice_initialized = false;
    }

    // Cleanup thread-local GC state
    #if ASTHRA_HAS_THREAD_LOCAL
    cleanup_thread_gc_state();
    #endif

    // Stop scheduler if running
    if (g_runtime.global_scheduler) {
        asthra_scheduler_stop(g_runtime.global_scheduler);
        asthra_scheduler_destroy(g_runtime.global_scheduler);
        g_runtime.global_scheduler = NULL;
    }

    // Cleanup CSPRNG
    asthra_csprng_cleanup_global();

    // Force final GC collection
    asthra_gc_collect();

    // Cleanup global mutexes
    pthread_mutex_destroy(&g_runtime.error_mutex);

    g_runtime.initialized = false;
}

// =============================================================================
// ERROR HANDLING IMPLEMENTATION
// =============================================================================

void asthra_set_error(AsthraErrorCode code, const char *message, const char *file, int line, const char *function) {
    pthread_mutex_lock(&g_runtime.error_mutex);
    g_runtime.last_error.code = code;
    strncpy(g_runtime.last_error.message, message ? message : "", sizeof(g_runtime.last_error.message) - 1);
    g_runtime.last_error.file = file;
    g_runtime.last_error.line = line;
    g_runtime.last_error.function = function;
    pthread_mutex_unlock(&g_runtime.error_mutex);
}

AsthraError asthra_get_last_error(void) {
    pthread_mutex_lock(&g_runtime.error_mutex);
    AsthraError error = g_runtime.last_error;
    pthread_mutex_unlock(&g_runtime.error_mutex);
    return error;
}

void asthra_clear_error(void) {
    pthread_mutex_lock(&g_runtime.error_mutex);
    memset(&g_runtime.last_error, 0, sizeof(AsthraError));
    pthread_mutex_unlock(&g_runtime.error_mutex);
}

// =============================================================================
// RUNTIME STATISTICS AND MONITORING
// =============================================================================

AsthraRuntimeStats asthra_get_runtime_stats(void) {
    pthread_mutex_lock(&g_runtime.error_mutex);
    AsthraRuntimeStats stats = {
        .total_allocations = ASTHRA_ATOMIC_LOAD(&g_runtime.atomic_stats.total_allocations),
        .total_deallocations = ASTHRA_ATOMIC_LOAD(&g_runtime.atomic_stats.total_deallocations),
        .current_memory_usage = ASTHRA_ATOMIC_LOAD(&g_runtime.atomic_stats.current_memory_usage),
        .peak_memory_usage = ASTHRA_ATOMIC_LOAD(&g_runtime.atomic_stats.peak_memory_usage),
        .gc_collections = ASTHRA_ATOMIC_LOAD(&g_runtime.atomic_stats.gc_collections),
        .gc_time_ms = ASTHRA_ATOMIC_LOAD(&g_runtime.atomic_stats.gc_time_ms),
        .tasks_spawned = ASTHRA_ATOMIC_LOAD(&g_runtime.atomic_stats.tasks_spawned),
        .tasks_completed = ASTHRA_ATOMIC_LOAD(&g_runtime.atomic_stats.tasks_completed),
        .ffi_calls = ASTHRA_ATOMIC_LOAD(&g_runtime.atomic_stats.ffi_calls)
    };
    pthread_mutex_unlock(&g_runtime.error_mutex);
    return stats;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

const char *asthra_runtime_version(void) {
    return ASTHRA_RUNTIME_VERSION;
}

const char *asthra_runtime_build_info(void) {
    return "Asthra Runtime v" ASTHRA_RUNTIME_VERSION " (C17 modernized)";
}

uint64_t asthra_get_timestamp_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

uint64_t asthra_get_timestamp_ms(void) {
    return asthra_get_timestamp_ns() / 1000000ULL;
}

void asthra_sleep_ms(uint64_t milliseconds) {
    struct timespec ts = {
        .tv_sec = (time_t)(milliseconds / 1000),
        .tv_nsec = (long)((milliseconds % 1000) * 1000000)
    };
    nanosleep(&ts, NULL);
}

void asthra_sleep_ns(uint64_t nanoseconds) {
    struct timespec ts = {
        .tv_sec = (time_t)(nanoseconds / 1000000000ULL),
        .tv_nsec = (long)(nanoseconds % 1000000000ULL)
    };
    nanosleep(&ts, NULL);
}

// =============================================================================
// I/O SUPPORT FUNCTIONS IMPLEMENTATION
// =============================================================================

// Standard stream access functions
FILE *asthra_get_stdin(void) {
    return stdin;
}

FILE *asthra_get_stdout(void) {
    return stdout;
}

FILE *asthra_get_stderr(void) {
    return stderr;
}

// I/O utility functions
void asthra_println(const char *message) {
    if (message) {
        printf("%s\n", message);
        fflush(stdout);
    }
}

void asthra_eprintln(const char *message) {
    if (message) {
        fprintf(stderr, "%s\n", message);
        fflush(stderr);
    }
}

// =============================================================================
// CORE ATOMIC STATISTICS ACCESSORS
// =============================================================================

// Note: Statistics functions moved to asthra_statistics.c to avoid duplicates

void asthra_atomic_stats_update_task_spawned(void) {
    ASTHRA_ATOMIC_FETCH_ADD(&g_runtime.atomic_stats.tasks_spawned, 1);
}

void asthra_atomic_stats_update_task_completed(void) {
    ASTHRA_ATOMIC_FETCH_ADD(&g_runtime.atomic_stats.tasks_completed, 1);
}

void asthra_atomic_stats_update_ffi_call(void) {
    ASTHRA_ATOMIC_FETCH_ADD(&g_runtime.atomic_stats.ffi_calls, 1);
}

// =============================================================================
// THREAD-LOCAL GC ROOT MANAGEMENT
// =============================================================================

#if ASTHRA_HAS_THREAD_LOCAL
void asthra_gc_register_thread_local_root(void *ptr) {
    if (!ptr) return;
    
    init_thread_gc_state();
    if (!asthra_thread_gc_state) return;
    
    // Expand capacity if needed
    if (asthra_thread_gc_state->root_count >= asthra_thread_gc_state->root_capacity) {
        size_t new_capacity = asthra_thread_gc_state->root_capacity ? 
                             asthra_thread_gc_state->root_capacity * 2 : 16;
        void **new_roots = realloc(asthra_thread_gc_state->gc_roots, 
                                  new_capacity * sizeof(void*));
        if (!new_roots) return;
        
        asthra_thread_gc_state->gc_roots = new_roots;
        asthra_thread_gc_state->root_capacity = new_capacity;
    }
    
    asthra_thread_gc_state->gc_roots[asthra_thread_gc_state->root_count++] = ptr;
}

void asthra_gc_unregister_thread_local_root(void *ptr) {
    if (!ptr || !asthra_thread_gc_state) return;
    
    for (size_t i = 0; i < asthra_thread_gc_state->root_count; i++) {
        if (asthra_thread_gc_state->gc_roots[i] == ptr) {
            // Move last element to this position
            asthra_thread_gc_state->root_count--;
            asthra_thread_gc_state->gc_roots[i] = 
                asthra_thread_gc_state->gc_roots[asthra_thread_gc_state->root_count];
            break;
        }
    }
}

void asthra_gc_flush_thread_local_roots(void) {
    if (!asthra_thread_gc_state) return;
    
    // Register all thread-local roots with global GC
    for (size_t i = 0; i < asthra_thread_gc_state->root_count; i++) {
        asthra_gc_register_root(asthra_thread_gc_state->gc_roots[i]);
    }
    
    // Clear thread-local roots
    asthra_thread_gc_state->root_count = 0;
}
#endif 

// =============================================================================
// COMMAND-LINE ARGUMENTS ACCESS
// =============================================================================

AsthraSliceHeader asthra_runtime_get_args(void) {
    if (!g_runtime.initialized) {
        // Return empty slice
        return (AsthraSliceHeader){0};
    }
    
    // Create args slice lazily on first access
    if (!g_runtime.args_slice_initialized && g_runtime.argc > 0 && g_runtime.argv != NULL) {
        // Allocate array for strings
        AsthraString *strings = (AsthraString*)asthra_alloc(sizeof(AsthraString) * g_runtime.argc, ASTHRA_ZONE_GC);
        if (!strings) {
            return (AsthraSliceHeader){0};
        }
        
        // Convert each C string to AsthraString
        for (int i = 0; i < g_runtime.argc; i++) {
            strings[i] = asthra_string_from_cstr(g_runtime.argv[i]);
        }
        
        // Create slice from the array
        g_runtime.args_slice = asthra_slice_from_raw_parts(strings, g_runtime.argc, sizeof(AsthraString), 
                                                           false, ASTHRA_OWNERSHIP_GC);
        g_runtime.args_slice_initialized = true;
    }
    
    return g_runtime.args_slice_initialized ? g_runtime.args_slice : (AsthraSliceHeader){0};
}
