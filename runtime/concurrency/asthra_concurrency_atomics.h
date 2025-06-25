/**
 * Asthra Concurrency Atomics - C17/C11 Atomic Operations Support
 * Part of the Asthra Runtime Modularization Plan - Phase 2
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * DESIGN GOALS:
 * - C17 modernization with atomic operations
 * - C11/C17 compatibility layer
 * - Lock-free operation wrappers
 * - Memory ordering abstractions
 */

#ifndef ASTHRA_CONCURRENCY_ATOMICS_H
#define ASTHRA_CONCURRENCY_ATOMICS_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// C11/C17 threads support detection
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
#include <threads.h>
#define ASTHRA_HAS_C11_THREADS 1
#else
#define ASTHRA_HAS_C11_THREADS 0
#endif
#else
#define ASTHRA_HAS_C11_THREADS 0
#endif

#include "../asthra_runtime.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// C17 ATOMIC TYPE DEFINITIONS
// =============================================================================

// C17 atomic types for lock-free operations with enhanced memory ordering
typedef atomic_uint_fast64_t asthra_concurrency_atomic_counter_t;
typedef atomic_size_t asthra_concurrency_atomic_size_t;
typedef atomic_bool asthra_concurrency_atomic_bool;
typedef atomic_int asthra_concurrency_atomic_int;
typedef atomic_uintptr_t asthra_concurrency_atomic_ptr_t;

// =============================================================================
// MEMORY ORDERING CONSTANTS
// =============================================================================

// C17 memory ordering for explicit synchronization
#define ASTHRA_MEMORY_ORDER_RELAXED memory_order_relaxed
#define ASTHRA_MEMORY_ORDER_ACQUIRE memory_order_acquire
#define ASTHRA_MEMORY_ORDER_RELEASE memory_order_release
#define ASTHRA_MEMORY_ORDER_ACQ_REL memory_order_acq_rel
#define ASTHRA_MEMORY_ORDER_SEQ_CST memory_order_seq_cst
#define ASTHRA_MEMORY_ORDER_CONSUME memory_order_consume

// =============================================================================
// ATOMIC OPERATION MACROS
// =============================================================================

// C17 atomic operations wrappers with explicit memory ordering
#define ASTHRA_ATOMIC_LOAD_EXPLICIT(ptr, order) atomic_load_explicit(ptr, order)
#define ASTHRA_ATOMIC_STORE_EXPLICIT(ptr, val, order) atomic_store_explicit(ptr, val, order)
#define ASTHRA_ATOMIC_FETCH_ADD_EXPLICIT(ptr, val, order) atomic_fetch_add_explicit(ptr, val, order)
#define ASTHRA_ATOMIC_FETCH_SUB_EXPLICIT(ptr, val, order) atomic_fetch_sub_explicit(ptr, val, order)
#define ASTHRA_ATOMIC_COMPARE_EXCHANGE_WEAK_EXPLICIT(ptr, expected, desired, success_order,        \
                                                     failure_order)                                \
    atomic_compare_exchange_weak_explicit(ptr, expected, desired, success_order, failure_order)
#define ASTHRA_ATOMIC_COMPARE_EXCHANGE_STRONG_EXPLICIT(ptr, expected, desired, success_order,      \
                                                       failure_order)                              \
    atomic_compare_exchange_strong_explicit(ptr, expected, desired, success_order, failure_order)

// Legacy atomic operations for compatibility
#define ASTHRA_ATOMIC_LOAD(ptr) atomic_load(ptr)
#define ASTHRA_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#define ASTHRA_ATOMIC_FETCH_ADD(ptr, val) atomic_fetch_add(ptr, val)
#define ASTHRA_ATOMIC_FETCH_SUB(ptr, val) atomic_fetch_sub(ptr, val)
#define ASTHRA_ATOMIC_COMPARE_EXCHANGE_WEAK(ptr, expected, desired)                                \
    atomic_compare_exchange_weak(ptr, expected, desired)
#define ASTHRA_ATOMIC_COMPARE_EXCHANGE_STRONG(ptr, expected, desired)                              \
    atomic_compare_exchange_strong(ptr, expected, desired)

// =============================================================================
// THREAD-LOCAL STORAGE
// =============================================================================

// C17 thread-local storage for concurrency state
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define ASTHRA_CONCURRENCY_THREAD_LOCAL _Thread_local
#elif defined(__GNUC__)
#define ASTHRA_CONCURRENCY_THREAD_LOCAL __thread
#elif defined(_MSC_VER)
#define ASTHRA_CONCURRENCY_THREAD_LOCAL __declspec(thread)
#else
#define ASTHRA_CONCURRENCY_THREAD_LOCAL
#endif

// =============================================================================
// THREADING ABSTRACTION LAYER
// =============================================================================

#if ASTHRA_HAS_C11_THREADS
typedef thrd_t asthra_concurrency_thread_t;
typedef mtx_t asthra_concurrency_mutex_t;
typedef cnd_t asthra_concurrency_cond_t;
typedef once_flag asthra_concurrency_once_flag_t;
typedef tss_t asthra_concurrency_tss_t;

#define ASTHRA_CONCURRENCY_THREAD_CREATE(thread, func, arg)                                        \
    (thrd_create(thread, func, arg) == thrd_success)
#define ASTHRA_CONCURRENCY_THREAD_JOIN(thread) thrd_join(thread, NULL)
#define ASTHRA_CONCURRENCY_THREAD_DETACH(thread) thrd_detach(thread)
#define ASTHRA_CONCURRENCY_THREAD_CURRENT() thrd_current()
#define ASTHRA_CONCURRENCY_THREAD_YIELD() thrd_yield()
#define ASTHRA_CONCURRENCY_THREAD_SLEEP(duration) thrd_sleep(duration, NULL)

#define ASTHRA_CONCURRENCY_MUTEX_INIT(mutex) (mtx_init(mutex, mtx_plain) == thrd_success)
#define ASTHRA_CONCURRENCY_MUTEX_LOCK(mutex) (mtx_lock(mutex) == thrd_success)
#define ASTHRA_CONCURRENCY_MUTEX_UNLOCK(mutex) (mtx_unlock(mutex) == thrd_success)
#define ASTHRA_CONCURRENCY_MUTEX_TRYLOCK(mutex) (mtx_trylock(mutex) == thrd_success)
#define ASTHRA_CONCURRENCY_MUTEX_DESTROY(mutex) mtx_destroy(mutex)

#define ASTHRA_CONCURRENCY_COND_INIT(cond) (cnd_init(cond) == thrd_success)
#define ASTHRA_CONCURRENCY_COND_WAIT(cond, mutex) (cnd_wait(cond, mutex) == thrd_success)
#define ASTHRA_CONCURRENCY_COND_TIMEDWAIT(cond, mutex, timeout)                                    \
    (cnd_timedwait(cond, mutex, timeout) == thrd_success ? 0 : ETIMEDOUT)
#define ASTHRA_CONCURRENCY_COND_SIGNAL(cond) (cnd_signal(cond) == thrd_success)
#define ASTHRA_CONCURRENCY_COND_BROADCAST(cond) (cnd_broadcast(cond) == thrd_success)
#define ASTHRA_CONCURRENCY_COND_DESTROY(cond) cnd_destroy(cond)

#define ASTHRA_CONCURRENCY_CALL_ONCE(flag, func) call_once(flag, func)
#define ASTHRA_CONCURRENCY_TSS_CREATE(key, destructor) (tss_create(key, destructor) == thrd_success)
#define ASTHRA_CONCURRENCY_TSS_GET(key) tss_get(key)
#define ASTHRA_CONCURRENCY_TSS_SET(key, value) (tss_set(key, value) == thrd_success)
#else
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>

typedef pthread_t asthra_concurrency_thread_t;
typedef pthread_mutex_t asthra_concurrency_mutex_t;
typedef pthread_cond_t asthra_concurrency_cond_t;
typedef pthread_once_t asthra_concurrency_once_flag_t;
typedef pthread_key_t asthra_concurrency_tss_t;

#define ASTHRA_CONCURRENCY_THREAD_CREATE(thread, func, arg)                                        \
    (pthread_create(thread, NULL, func, arg) == 0)
#define ASTHRA_CONCURRENCY_THREAD_JOIN(thread) pthread_join(thread, NULL)
#define ASTHRA_CONCURRENCY_THREAD_DETACH(thread) pthread_detach(thread)
#define ASTHRA_CONCURRENCY_THREAD_CURRENT() pthread_self()
#define ASTHRA_CONCURRENCY_THREAD_YIELD() sched_yield()
#define ASTHRA_CONCURRENCY_THREAD_SLEEP(duration) nanosleep(duration, NULL)

#define ASTHRA_CONCURRENCY_MUTEX_INIT(mutex) (pthread_mutex_init(mutex, NULL) == 0)
#define ASTHRA_CONCURRENCY_MUTEX_LOCK(mutex) (pthread_mutex_lock(mutex) == 0)
#define ASTHRA_CONCURRENCY_MUTEX_UNLOCK(mutex) (pthread_mutex_unlock(mutex) == 0)
#define ASTHRA_CONCURRENCY_MUTEX_TRYLOCK(mutex) (pthread_mutex_trylock(mutex) == 0)
#define ASTHRA_CONCURRENCY_MUTEX_DESTROY(mutex) pthread_mutex_destroy(mutex)

#define ASTHRA_CONCURRENCY_COND_INIT(cond) (pthread_cond_init(cond, NULL) == 0)
#define ASTHRA_CONCURRENCY_COND_WAIT(cond, mutex) (pthread_cond_wait(cond, mutex) == 0)
#define ASTHRA_CONCURRENCY_COND_TIMEDWAIT(cond, mutex, timeout)                                    \
    pthread_cond_timedwait(cond, mutex, timeout)
#define ASTHRA_CONCURRENCY_COND_SIGNAL(cond) (pthread_cond_signal(cond) == 0)
#define ASTHRA_CONCURRENCY_COND_BROADCAST(cond) (pthread_cond_broadcast(cond) == 0)
#define ASTHRA_CONCURRENCY_COND_DESTROY(cond) pthread_cond_destroy(cond)

#define ASTHRA_CONCURRENCY_CALL_ONCE(flag, func) pthread_once(flag, func)
#define ASTHRA_CONCURRENCY_TSS_CREATE(key, destructor) (pthread_key_create(key, destructor) == 0)
#define ASTHRA_CONCURRENCY_TSS_GET(key) pthread_getspecific(key)
#define ASTHRA_CONCURRENCY_TSS_SET(key, value) (pthread_setspecific(key, value) == 0)
#endif

// =============================================================================
// COMPILE-TIME ASSERTIONS
// =============================================================================

// C17 static assertions for concurrency assumptions
#ifndef ASTHRA_STATIC_ASSERT
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define ASTHRA_STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)
#else
#define ASTHRA_STATIC_ASSERT(expr, msg)                                                            \
    typedef char asthra_static_assert_##__LINE__[(expr) ? 1 : -1]
#endif
#endif

ASTHRA_STATIC_ASSERT(sizeof(asthra_concurrency_atomic_counter_t) >= sizeof(uint64_t),
                     "Atomic counter must be at least 64-bit");

#if defined(ATOMIC_BOOL_LOCK_FREE)
ASTHRA_STATIC_ASSERT(ATOMIC_BOOL_LOCK_FREE == 2, "Atomic bool must be lock-free");
#endif

#if defined(ATOMIC_POINTER_LOCK_FREE)
ASTHRA_STATIC_ASSERT(ATOMIC_POINTER_LOCK_FREE >= 1, "Atomic pointers should be lock-free");
#endif

// =============================================================================
// LOCK-FREE DATA STRUCTURES
// =============================================================================

/**
 * Lock-free stack implementation
 */
typedef struct AsthraConcurrencyLockFreeStack {
    _Atomic(void *) top;                            // Atomic top pointer
    asthra_concurrency_atomic_size_t size;          // Atomic size counter
    asthra_concurrency_atomic_counter_t push_count; // Total pushes
    asthra_concurrency_atomic_counter_t pop_count;  // Total pops
} AsthraConcurrencyLockFreeStack;

/**
 * Lock-free queue implementation
 */
typedef struct AsthraConcurrencyLockFreeQueue {
    _Atomic(void *) head;                              // Atomic head pointer
    _Atomic(void *) tail;                              // Atomic tail pointer
    asthra_concurrency_atomic_size_t size;             // Atomic size counter
    asthra_concurrency_atomic_counter_t enqueue_count; // Total enqueues
    asthra_concurrency_atomic_counter_t dequeue_count; // Total dequeues
} AsthraConcurrencyLockFreeQueue;

// =============================================================================
// VERSION INFORMATION
// =============================================================================

#define ASTHRA_CONCURRENCY_ATOMICS_VERSION_MAJOR 1
#define ASTHRA_CONCURRENCY_ATOMICS_VERSION_MINOR 0
#define ASTHRA_CONCURRENCY_ATOMICS_VERSION_PATCH 0
#define ASTHRA_CONCURRENCY_ATOMICS_VERSION "1.0.0"

// Feature detection
#ifndef ASTHRA_CONCURRENCY_HAS_C17_THREADS
#define ASTHRA_CONCURRENCY_HAS_C17_THREADS ASTHRA_HAS_C11_THREADS
#endif
#define ASTHRA_CONCURRENCY_HAS_LOCK_FREE_PRIMITIVES 1

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_ATOMICS_H
