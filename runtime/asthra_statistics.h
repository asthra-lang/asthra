/**
 * Asthra Programming Language - Global Statistics Tracking System
 * Comprehensive performance and memory statistics collection
 *
 * Phase 4: TODO Cleanup - Performance Barriers and Statistics Tracking
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Provides atomic, thread-safe statistics collection for memory allocation,
 * garbage collection, performance metrics, and system-wide monitoring.
 */

#ifndef ASTHRA_STATISTICS_H
#define ASTHRA_STATISTICS_H

#include <stdint.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// STATISTICS STRUCTURES
// =============================================================================

/**
 * Memory allocation statistics
 */
typedef struct {
    atomic_uint_fast64_t total_allocations;
    atomic_uint_fast64_t total_deallocations;
    atomic_uint_fast64_t bytes_allocated;
    atomic_uint_fast64_t bytes_deallocated;
    atomic_uint_fast64_t peak_memory_usage;
    atomic_uint_fast64_t current_memory_usage;
    atomic_uint_fast32_t allocation_failures;
    atomic_uint_fast32_t alignment_allocations;
} AsthraMemoryStatistics;

/**
 * Garbage collection statistics
 */
typedef struct {
    atomic_uint_fast64_t collections_performed;
    atomic_uint_fast64_t total_collection_time_ns;
    atomic_uint_fast64_t bytes_collected;
    atomic_uint_fast64_t objects_collected;
    atomic_uint_fast32_t collection_cycles;
    atomic_uint_fast64_t last_collection_time_ns;
    atomic_uint_fast64_t max_collection_time_ns;
    atomic_uint_fast64_t min_collection_time_ns;
} AsthraGCStatistics;

/**
 * Performance timing statistics
 */
typedef struct {
    atomic_uint_fast64_t compilation_time_ns;
    atomic_uint_fast64_t semantic_analysis_time_ns;
    atomic_uint_fast64_t code_generation_time_ns;
    atomic_uint_fast64_t optimization_time_ns;
    atomic_uint_fast32_t files_compiled;
    atomic_uint_fast32_t lines_processed;
    atomic_uint_fast32_t symbols_created;
    atomic_uint_fast64_t total_execution_time_ns;
} AsthraPerformanceStatistics;

/**
 * Concurrency and threading statistics
 */
typedef struct {
    atomic_uint_fast32_t threads_created;
    atomic_uint_fast32_t threads_destroyed;
    atomic_uint_fast32_t active_threads;
    atomic_uint_fast64_t synchronization_operations;
    atomic_uint_fast64_t lock_contentions;
    atomic_uint_fast64_t barrier_operations;
    atomic_uint_fast32_t deadlocks_detected;
    atomic_uint_fast32_t race_conditions_detected;
} AsthraConcurrencyStatistics;

/**
 * Global statistics aggregation
 */
typedef struct {
    AsthraMemoryStatistics memory;
    AsthraGCStatistics gc;
    AsthraPerformanceStatistics performance;
    AsthraConcurrencyStatistics concurrency;
    
    // System-wide counters
    atomic_uint_fast64_t system_start_time_ns;
    atomic_uint_fast64_t last_update_time_ns;
    atomic_bool statistics_enabled;
} AsthraGlobalStatistics;

// =============================================================================
// STATISTICS INTERFACE
// =============================================================================

/**
 * Initialize the global statistics system
 */
void asthra_statistics_init(void);

/**
 * Shutdown and cleanup statistics system
 */
void asthra_statistics_shutdown(void);

/**
 * Enable or disable statistics collection
 */
void asthra_statistics_set_enabled(bool enabled);

/**
 * Check if statistics are enabled
 */
bool asthra_statistics_is_enabled(void);

/**
 * Reset all statistics to zero
 */
void asthra_statistics_reset(void);

/**
 * Get a snapshot of all statistics
 */
AsthraGlobalStatistics asthra_statistics_get_snapshot(void);

// =============================================================================
// MEMORY STATISTICS FUNCTIONS
// =============================================================================

/**
 * Update allocation statistics
 */
void asthra_atomic_stats_update_allocation(size_t size);

/**
 * Update deallocation statistics
 */
void asthra_atomic_stats_update_deallocation(size_t size);

/**
 * Record allocation failure
 */
void asthra_atomic_stats_record_allocation_failure(void);

/**
 * Record aligned allocation
 */
void asthra_atomic_stats_record_aligned_allocation(size_t size, size_t alignment);

/**
 * Update peak memory usage if current usage is higher
 */
void asthra_atomic_stats_update_peak_memory(void);

// =============================================================================
// GARBAGE COLLECTION STATISTICS FUNCTIONS
// =============================================================================

/**
 * Update GC collection statistics
 */
void asthra_atomic_stats_update_gc_collection(uint64_t collection_time_ns);

/**
 * Record objects collected during GC
 */
void asthra_atomic_stats_record_gc_objects_collected(size_t object_count, size_t bytes_freed);

/**
 * Start GC timing measurement
 */
uint64_t asthra_atomic_stats_gc_start_timer(void);

/**
 * End GC timing measurement and update statistics
 */
void asthra_atomic_stats_gc_end_timer(uint64_t start_time_ns);

// =============================================================================
// PERFORMANCE STATISTICS FUNCTIONS
// =============================================================================

/**
 * Update compilation performance statistics
 */
void asthra_atomic_stats_update_compilation(uint64_t time_ns, size_t files, size_t lines);

/**
 * Update semantic analysis performance
 */
void asthra_atomic_stats_update_semantic_analysis(uint64_t time_ns, size_t symbols);

/**
 * Update code generation performance
 */
void asthra_atomic_stats_update_code_generation(uint64_t time_ns);

/**
 * Update optimization performance
 */
void asthra_atomic_stats_update_optimization(uint64_t time_ns);

// =============================================================================
// CONCURRENCY STATISTICS FUNCTIONS
// =============================================================================

/**
 * Record thread creation
 */
void asthra_atomic_stats_record_thread_created(void);

/**
 * Record thread destruction
 */
void asthra_atomic_stats_record_thread_destroyed(void);

/**
 * Record synchronization operation
 */
void asthra_atomic_stats_record_sync_operation(void);

/**
 * Record lock contention
 */
void asthra_atomic_stats_record_lock_contention(void);

/**
 * Record barrier operation
 */
void asthra_atomic_stats_record_barrier_operation(void);

/**
 * Record deadlock detection
 */
void asthra_atomic_stats_record_deadlock(void);

/**
 * Record race condition detection
 */
void asthra_atomic_stats_record_race_condition(void);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Get current timestamp in nanoseconds
 */
uint64_t asthra_statistics_get_time_ns(void);

/**
 * Print formatted statistics report
 */
void asthra_statistics_print_report(FILE* output, bool json_format);

/**
 * Export statistics to JSON string (caller must free)
 */
char* asthra_statistics_export_json(void);

/**
 * Save statistics to file
 */
bool asthra_statistics_save_to_file(const char* filename, bool json_format);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_STATISTICS_H