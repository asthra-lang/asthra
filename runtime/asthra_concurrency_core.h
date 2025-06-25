#ifndef ASTHRA_CONCURRENCY_CORE_H
#define ASTHRA_CONCURRENCY_CORE_H

#include "core/asthra_runtime_core.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

uint64_t
asthra_concurrency_atomic_increment_counter_explicit(asthra_concurrency_atomic_counter_t *counter,
                                                     memory_order order);
uint64_t
asthra_concurrency_atomic_decrement_counter_explicit(asthra_concurrency_atomic_counter_t *counter,
                                                     memory_order order);
uint64_t
asthra_concurrency_atomic_load_counter_explicit(asthra_concurrency_atomic_counter_t *counter,
                                                memory_order order);
void asthra_concurrency_atomic_store_counter_explicit(asthra_concurrency_atomic_counter_t *counter,
                                                      uint64_t value, memory_order order);
bool asthra_concurrency_atomic_cas_ptr_explicit(_Atomic(void *) *ptr, void **expected,
                                                void *desired, memory_order success_order,
                                                memory_order failure_order);
bool asthra_concurrency_atomic_cas_ptr(_Atomic(void *) *ptr, void **expected, void *desired);
bool asthra_concurrency_lock_free_stack_push(AsthraConcurrencyLockFreeStack *stack, void *item);
void *asthra_concurrency_lock_free_stack_pop(AsthraConcurrencyLockFreeStack *stack);
bool asthra_concurrency_create_thread_hybrid(asthra_concurrency_thread_t *thread,
                                             void *(*func)(void *), void *arg,
                                             const AsthraConcurrencyTaskSpawnOptions *options);
AsthraConcurrencyThreadData *asthra_concurrency_get_thread_data(void);
void asthra_concurrency_set_thread_data(AsthraConcurrencyThreadData *data);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_CORE_H
