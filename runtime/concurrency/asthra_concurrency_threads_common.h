/**
 * Asthra Concurrency Thread Management - Common Definitions
 * Shared types, constants, and utilities for modular thread management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CONCURRENCY_THREADS_COMMON_H
#define ASTHRA_CONCURRENCY_THREADS_COMMON_H

#include <assert.h>
#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Use platform compatibility layer instead of direct pthread.h
#include "../asthra_concurrency_bridge_modular.h"
#include "../platform/asthra_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// COMMON FORWARD DECLARATIONS
// =============================================================================

extern AsthraConcurrencyBridge *asthra_concurrency_get_bridge(void);
extern AsthraConcurrencyThreadData *asthra_concurrency_get_thread_data(void);
extern void asthra_concurrency_set_thread_data(AsthraConcurrencyThreadData *data);
extern bool asthra_concurrency_is_initialized(void);
extern AsthraResult asthra_concurrency_create_error_result(AsthraConcurrencyErrorCode code,
                                                           const char *message);
extern uint64_t asthra_concurrency_get_timestamp_ms(void);
extern size_t asthra_concurrency_atomic_load_size(asthra_concurrency_atomic_size_t *size_ptr);
extern size_t asthra_concurrency_atomic_increment_size(asthra_concurrency_atomic_size_t *size_ptr);
extern size_t asthra_concurrency_atomic_decrement_size(asthra_concurrency_atomic_size_t *size_ptr);
extern uint64_t
asthra_concurrency_atomic_increment_counter(asthra_concurrency_atomic_counter_t *counter);

// =============================================================================
// THREAD REGISTRY STATISTICS TYPE
// =============================================================================

typedef struct {
    size_t active_threads;
    uint64_t total_registered;
    size_t total_gc_roots;
    bool is_shutdown;
} AsthraConcurrencyThreadRegistryStats;

// =============================================================================
// COMMON UTILITY FUNCTIONS
// =============================================================================

/**
 * Get thread registry statistics
 * @return Current thread registry statistics
 */
AsthraConcurrencyThreadRegistryStats asthra_concurrency_get_thread_registry_stats_impl(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_THREADS_COMMON_H
