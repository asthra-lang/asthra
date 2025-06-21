#ifndef ASTHRA_CONCURRENCY_CORE_H
#define ASTHRA_CONCURRENCY_CORE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "asthra_concurrency_atomics.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ATOMIC COUNTER OPERATIONS
// =============================================================================

// Increment atomic counter with explicit memory ordering
uint64_t asthra_concurrency_atomic_increment_counter_explicit(
    asthra_concurrency_atomic_counter_t *counter,
    memory_order order
);

// Decrement atomic counter with explicit memory ordering
uint64_t asthra_concurrency_atomic_decrement_counter_explicit(
    asthra_concurrency_atomic_counter_t *counter,
    memory_order order
);

// Load atomic counter with explicit memory ordering
uint64_t asthra_concurrency_atomic_load_counter_explicit(
    asthra_concurrency_atomic_counter_t *counter,
    memory_order order
);

// Store atomic counter with explicit memory ordering
void asthra_concurrency_atomic_store_counter_explicit(
    asthra_concurrency_atomic_counter_t *counter,
    uint64_t value,
    memory_order order
);

// =============================================================================
// ATOMIC POINTER OPERATIONS
// =============================================================================

// Compare and swap pointer with explicit memory ordering
bool asthra_concurrency_atomic_cas_ptr_explicit(
    _Atomic(void*) *ptr,
    void **expected,
    void *desired,
    memory_order success_order,
    memory_order failure_order
);

// =============================================================================
// LOCK-FREE STACK OPERATIONS
// =============================================================================

// Forward declarations for lock-free stack operations
// These need to be properly defined based on the implementation
struct asthra_concurrency_lock_free_stack;
typedef struct asthra_concurrency_lock_free_stack asthra_concurrency_lock_free_stack_t;

bool asthra_concurrency_lock_free_stack_push(
    asthra_concurrency_lock_free_stack_t *stack,
    void *data
);

void* asthra_concurrency_lock_free_stack_pop(
    asthra_concurrency_lock_free_stack_t *stack
);

// =============================================================================
// THREAD OPERATIONS
// =============================================================================

// Create a hybrid thread with platform-specific optimizations
bool asthra_concurrency_create_thread_hybrid(
    asthra_concurrency_thread_t *thread,
    void *(*func)(void*),
    void *arg,
    const char *name
);

// Get thread-specific data
void* asthra_concurrency_get_thread_data(void);

// Set thread-specific data
void asthra_concurrency_set_thread_data(void *data);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_CORE_H