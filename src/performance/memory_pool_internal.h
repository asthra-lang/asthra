#ifndef ASTHRA_MEMORY_POOL_INTERNAL_H
#define ASTHRA_MEMORY_POOL_INTERNAL_H

#include "memory_pool.h"
#include "benchmark.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#ifdef __APPLE__
#include <sys/mman.h>
#include <mach/mach.h>
#elif defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#endif

// =============================================================================
// INTERNAL CONSTANTS
// =============================================================================

// Magic numbers for memory validation (reserved for future use)
#define ASTHRA_MEMORY_POOL_MAGIC 0xDEADBEEF __attribute__((unused))
#define ASTHRA_MEMORY_POOL_FREE_MAGIC 0xFEEDFACE __attribute__((unused))

// Thread-local storage for performance tracking
extern ASTHRA_MEMORY_POOL_THREAD_LOCAL AsthraPoolStatistics *g_thread_local_stats __attribute__((unused));

// Global atomic counters for memory tracking (reserved for future use)
extern _Atomic(uint64_t) g_global_allocations __attribute__((unused));
extern _Atomic(uint64_t) g_global_deallocations __attribute__((unused));
extern _Atomic(size_t) g_global_memory_usage __attribute__((unused));

// Suppress unused variable warnings for global counters
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
// These variables are reserved for future global memory tracking features
static void suppress_unused_warnings(void) __attribute__((unused));
static inline void suppress_unused_warnings(void) {
    (void)g_thread_local_stats;
    (void)g_global_allocations;
    (void)g_global_deallocations;
    (void)g_global_memory_usage;
}
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif // ASTHRA_MEMORY_POOL_INTERNAL_H 
