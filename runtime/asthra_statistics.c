/**
 * Asthra Programming Language - Global Statistics Tracking Implementation
 * Thread-safe atomic statistics collection system
 *
 * Phase 4: TODO Cleanup - Performance Barriers and Statistics Tracking
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_statistics.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global statistics instance
static AsthraGlobalStatistics g_stats = {0};
static bool g_stats_initialized = false;

// =============================================================================
// INITIALIZATION AND CLEANUP
// =============================================================================

void asthra_statistics_init(void) {
    if (g_stats_initialized)
        return;

    // Initialize all atomic counters to zero
    memset(&g_stats, 0, sizeof(AsthraGlobalStatistics));

    // Set initial timestamps
    uint64_t current_time = asthra_statistics_get_time_ns();
    atomic_store(&g_stats.system_start_time_ns, current_time);
    atomic_store(&g_stats.last_update_time_ns, current_time);

    // Enable statistics by default
    atomic_store(&g_stats.statistics_enabled, true);

    // Initialize min collection time to maximum value
    atomic_store(&g_stats.gc.min_collection_time_ns, UINT64_MAX);

    g_stats_initialized = true;
}

void asthra_statistics_shutdown(void) {
    if (!g_stats_initialized)
        return;

    atomic_store(&g_stats.statistics_enabled, false);
    g_stats_initialized = false;
}

void asthra_statistics_set_enabled(bool enabled) {
    if (g_stats_initialized) {
        atomic_store(&g_stats.statistics_enabled, enabled);
    }
}

bool asthra_statistics_is_enabled(void) {
    return g_stats_initialized && atomic_load(&g_stats.statistics_enabled);
}

void asthra_statistics_reset(void) {
    if (!g_stats_initialized)
        return;

    bool was_enabled = atomic_load(&g_stats.statistics_enabled);

    // Reset all counters while preserving initialization state
    memset(&g_stats, 0, sizeof(AsthraGlobalStatistics));

    uint64_t current_time = asthra_statistics_get_time_ns();
    atomic_store(&g_stats.system_start_time_ns, current_time);
    atomic_store(&g_stats.last_update_time_ns, current_time);
    atomic_store(&g_stats.statistics_enabled, was_enabled);
    atomic_store(&g_stats.gc.min_collection_time_ns, UINT64_MAX);
}

AsthraGlobalStatistics asthra_statistics_get_snapshot(void) {
    if (!asthra_statistics_is_enabled()) {
        AsthraGlobalStatistics empty = {0};
        return empty;
    }

    // Create a snapshot by reading all atomic values
    AsthraGlobalStatistics snapshot;

    // Memory statistics
    snapshot.memory.total_allocations = atomic_load(&g_stats.memory.total_allocations);
    snapshot.memory.total_deallocations = atomic_load(&g_stats.memory.total_deallocations);
    snapshot.memory.bytes_allocated = atomic_load(&g_stats.memory.bytes_allocated);
    snapshot.memory.bytes_deallocated = atomic_load(&g_stats.memory.bytes_deallocated);
    snapshot.memory.peak_memory_usage = atomic_load(&g_stats.memory.peak_memory_usage);
    snapshot.memory.current_memory_usage = atomic_load(&g_stats.memory.current_memory_usage);
    snapshot.memory.allocation_failures = atomic_load(&g_stats.memory.allocation_failures);
    snapshot.memory.alignment_allocations = atomic_load(&g_stats.memory.alignment_allocations);

    // GC statistics
    snapshot.gc.collections_performed = atomic_load(&g_stats.gc.collections_performed);
    snapshot.gc.total_collection_time_ns = atomic_load(&g_stats.gc.total_collection_time_ns);
    snapshot.gc.bytes_collected = atomic_load(&g_stats.gc.bytes_collected);
    snapshot.gc.objects_collected = atomic_load(&g_stats.gc.objects_collected);
    snapshot.gc.collection_cycles = atomic_load(&g_stats.gc.collection_cycles);
    snapshot.gc.last_collection_time_ns = atomic_load(&g_stats.gc.last_collection_time_ns);
    snapshot.gc.max_collection_time_ns = atomic_load(&g_stats.gc.max_collection_time_ns);
    snapshot.gc.min_collection_time_ns = atomic_load(&g_stats.gc.min_collection_time_ns);

    // Performance statistics
    snapshot.performance.compilation_time_ns =
        atomic_load(&g_stats.performance.compilation_time_ns);
    snapshot.performance.semantic_analysis_time_ns =
        atomic_load(&g_stats.performance.semantic_analysis_time_ns);
    snapshot.performance.code_generation_time_ns =
        atomic_load(&g_stats.performance.code_generation_time_ns);
    snapshot.performance.optimization_time_ns =
        atomic_load(&g_stats.performance.optimization_time_ns);
    snapshot.performance.files_compiled = atomic_load(&g_stats.performance.files_compiled);
    snapshot.performance.lines_processed = atomic_load(&g_stats.performance.lines_processed);
    snapshot.performance.symbols_created = atomic_load(&g_stats.performance.symbols_created);
    snapshot.performance.total_execution_time_ns =
        atomic_load(&g_stats.performance.total_execution_time_ns);

    // Concurrency statistics
    snapshot.concurrency.threads_created = atomic_load(&g_stats.concurrency.threads_created);
    snapshot.concurrency.threads_destroyed = atomic_load(&g_stats.concurrency.threads_destroyed);
    snapshot.concurrency.active_threads = atomic_load(&g_stats.concurrency.active_threads);
    snapshot.concurrency.synchronization_operations =
        atomic_load(&g_stats.concurrency.synchronization_operations);
    snapshot.concurrency.lock_contentions = atomic_load(&g_stats.concurrency.lock_contentions);
    snapshot.concurrency.barrier_operations = atomic_load(&g_stats.concurrency.barrier_operations);
    snapshot.concurrency.deadlocks_detected = atomic_load(&g_stats.concurrency.deadlocks_detected);
    snapshot.concurrency.race_conditions_detected =
        atomic_load(&g_stats.concurrency.race_conditions_detected);

    // System-wide
    snapshot.system_start_time_ns = atomic_load(&g_stats.system_start_time_ns);
    snapshot.last_update_time_ns = atomic_load(&g_stats.last_update_time_ns);
    snapshot.statistics_enabled = atomic_load(&g_stats.statistics_enabled);

    return snapshot;
}

// =============================================================================
// MEMORY STATISTICS FUNCTIONS
// =============================================================================

void asthra_atomic_stats_update_allocation(size_t size) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.memory.total_allocations, 1);
    atomic_fetch_add(&g_stats.memory.bytes_allocated, size);
    atomic_fetch_add(&g_stats.memory.current_memory_usage, size);

    asthra_atomic_stats_update_peak_memory();
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_update_deallocation(size_t size) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.memory.total_deallocations, 1);
    atomic_fetch_add(&g_stats.memory.bytes_deallocated, size);

    // Safely subtract from current usage
    uint64_t current = atomic_load(&g_stats.memory.current_memory_usage);
    if (current >= size) {
        atomic_fetch_sub(&g_stats.memory.current_memory_usage, size);
    } else {
        atomic_store(&g_stats.memory.current_memory_usage, 0);
    }

    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_record_allocation_failure(void) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.memory.allocation_failures, 1);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_record_aligned_allocation(size_t size, size_t alignment) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.memory.alignment_allocations, 1);
    asthra_atomic_stats_update_allocation(size);
    (void)alignment; // Unused for now, could track alignment statistics later
}

void asthra_atomic_stats_update_peak_memory(void) {
    if (!asthra_statistics_is_enabled())
        return;

    uint64_t current = atomic_load(&g_stats.memory.current_memory_usage);
    uint64_t peak = atomic_load(&g_stats.memory.peak_memory_usage);

    while (current > peak) {
        if (atomic_compare_exchange_weak(&g_stats.memory.peak_memory_usage, &peak, current)) {
            break;
        }
    }
}

// =============================================================================
// GARBAGE COLLECTION STATISTICS FUNCTIONS
// =============================================================================

void asthra_atomic_stats_update_gc_collection(uint64_t collection_time_ns) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.gc.collections_performed, 1);
    atomic_fetch_add(&g_stats.gc.total_collection_time_ns, collection_time_ns);
    atomic_fetch_add(&g_stats.gc.collection_cycles, 1);
    atomic_store(&g_stats.gc.last_collection_time_ns, collection_time_ns);

    // Update max collection time
    uint64_t current_max = atomic_load(&g_stats.gc.max_collection_time_ns);
    while (collection_time_ns > current_max) {
        if (atomic_compare_exchange_weak(&g_stats.gc.max_collection_time_ns, &current_max,
                                         collection_time_ns)) {
            break;
        }
    }

    // Update min collection time
    uint64_t current_min = atomic_load(&g_stats.gc.min_collection_time_ns);
    while (collection_time_ns < current_min) {
        if (atomic_compare_exchange_weak(&g_stats.gc.min_collection_time_ns, &current_min,
                                         collection_time_ns)) {
            break;
        }
    }

    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_record_gc_objects_collected(size_t object_count, size_t bytes_freed) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.gc.objects_collected, object_count);
    atomic_fetch_add(&g_stats.gc.bytes_collected, bytes_freed);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

uint64_t asthra_atomic_stats_gc_start_timer(void) {
    return asthra_statistics_get_time_ns();
}

void asthra_atomic_stats_gc_end_timer(uint64_t start_time_ns) {
    if (!asthra_statistics_is_enabled())
        return;

    uint64_t end_time = asthra_statistics_get_time_ns();
    uint64_t duration = end_time - start_time_ns;
    asthra_atomic_stats_update_gc_collection(duration);
}

// =============================================================================
// PERFORMANCE STATISTICS FUNCTIONS
// =============================================================================

void asthra_atomic_stats_update_compilation(uint64_t time_ns, size_t files, size_t lines) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.performance.compilation_time_ns, time_ns);
    atomic_fetch_add(&g_stats.performance.files_compiled, files);
    atomic_fetch_add(&g_stats.performance.lines_processed, lines);
    atomic_fetch_add(&g_stats.performance.total_execution_time_ns, time_ns);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_update_semantic_analysis(uint64_t time_ns, size_t symbols) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.performance.semantic_analysis_time_ns, time_ns);
    atomic_fetch_add(&g_stats.performance.symbols_created, symbols);
    atomic_fetch_add(&g_stats.performance.total_execution_time_ns, time_ns);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_update_code_generation(uint64_t time_ns) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.performance.code_generation_time_ns, time_ns);
    atomic_fetch_add(&g_stats.performance.total_execution_time_ns, time_ns);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_update_optimization(uint64_t time_ns) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.performance.optimization_time_ns, time_ns);
    atomic_fetch_add(&g_stats.performance.total_execution_time_ns, time_ns);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

// =============================================================================
// CONCURRENCY STATISTICS FUNCTIONS
// =============================================================================

void asthra_atomic_stats_record_thread_created(void) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.concurrency.threads_created, 1);
    atomic_fetch_add(&g_stats.concurrency.active_threads, 1);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_record_thread_destroyed(void) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.concurrency.threads_destroyed, 1);

    // Safely decrement active threads
    uint32_t current = atomic_load(&g_stats.concurrency.active_threads);
    if (current > 0) {
        atomic_fetch_sub(&g_stats.concurrency.active_threads, 1);
    }

    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_record_sync_operation(void) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.concurrency.synchronization_operations, 1);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_record_lock_contention(void) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.concurrency.lock_contentions, 1);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_record_barrier_operation(void) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.concurrency.barrier_operations, 1);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_record_deadlock(void) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.concurrency.deadlocks_detected, 1);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

void asthra_atomic_stats_record_race_condition(void) {
    if (!asthra_statistics_is_enabled())
        return;

    atomic_fetch_add(&g_stats.concurrency.race_conditions_detected, 1);
    atomic_store(&g_stats.last_update_time_ns, asthra_statistics_get_time_ns());
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

uint64_t asthra_statistics_get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

void asthra_statistics_print_report(FILE *output, bool json_format) {
    if (!output)
        return;

    AsthraGlobalStatistics stats = asthra_statistics_get_snapshot();

    if (json_format) {
        char *json = asthra_statistics_export_json();
        if (json) {
            fprintf(output, "%s\n", json);
            free(json);
        }
        return;
    }

    // Human-readable format
    fprintf(output, "=== Asthra Runtime Statistics Report ===\n\n");

    // Memory statistics
    fprintf(output, "Memory Statistics:\n");
    fprintf(output, "  Allocations: %" PRIu64 " (%" PRIu64 " bytes)\n",
            stats.memory.total_allocations, stats.memory.bytes_allocated);
    fprintf(output, "  Deallocations: %" PRIu64 " (%" PRIu64 " bytes)\n",
            stats.memory.total_deallocations, stats.memory.bytes_deallocated);
    fprintf(output, "  Current Usage: %" PRIu64 " bytes\n", stats.memory.current_memory_usage);
    fprintf(output, "  Peak Usage: %" PRIu64 " bytes\n", stats.memory.peak_memory_usage);
    fprintf(output, "  Allocation Failures: %" PRIu32 "\n", stats.memory.allocation_failures);
    fprintf(output, "  Aligned Allocations: %" PRIu32 "\n\n", stats.memory.alignment_allocations);

    // GC statistics
    fprintf(output, "Garbage Collection Statistics:\n");
    fprintf(output, "  Collections: %" PRIu64 "\n", stats.gc.collections_performed);
    fprintf(output, "  Total Time: %.2f ms\n",
            (double)stats.gc.total_collection_time_ns / 1000000.0);
    if (stats.gc.collections_performed > 0) {
        fprintf(output, "  Average Time: %.2f ms\n",
                (double)stats.gc.total_collection_time_ns / (double)stats.gc.collections_performed /
                    1000000.0);
        fprintf(output, "  Min Time: %.2f ms\n",
                (double)stats.gc.min_collection_time_ns / 1000000.0);
        fprintf(output, "  Max Time: %.2f ms\n",
                (double)stats.gc.max_collection_time_ns / 1000000.0);
    }
    fprintf(output, "  Objects Collected: %" PRIu64 "\n", stats.gc.objects_collected);
    fprintf(output, "  Bytes Collected: %" PRIu64 "\n\n", stats.gc.bytes_collected);

    // Performance statistics
    fprintf(output, "Performance Statistics:\n");
    fprintf(output, "  Files Compiled: %" PRIu32 "\n", stats.performance.files_compiled);
    fprintf(output, "  Lines Processed: %" PRIu32 "\n", stats.performance.lines_processed);
    fprintf(output, "  Symbols Created: %" PRIu32 "\n", stats.performance.symbols_created);
    fprintf(output, "  Compilation Time: %.2f ms\n",
            (double)stats.performance.compilation_time_ns / 1000000.0);
    fprintf(output, "  Semantic Analysis Time: %.2f ms\n",
            (double)stats.performance.semantic_analysis_time_ns / 1000000.0);
    fprintf(output, "  Code Generation Time: %.2f ms\n",
            (double)stats.performance.code_generation_time_ns / 1000000.0);
    fprintf(output, "  Optimization Time: %.2f ms\n\n",
            (double)stats.performance.optimization_time_ns / 1000000.0);

    // Concurrency statistics
    fprintf(output, "Concurrency Statistics:\n");
    fprintf(output, "  Threads Created: %" PRIu32 "\n", stats.concurrency.threads_created);
    fprintf(output, "  Active Threads: %" PRIu32 "\n", stats.concurrency.active_threads);
    fprintf(output, "  Sync Operations: %" PRIu64 "\n",
            stats.concurrency.synchronization_operations);
    fprintf(output, "  Lock Contentions: %" PRIu64 "\n", stats.concurrency.lock_contentions);
    fprintf(output, "  Barrier Operations: %" PRIu64 "\n", stats.concurrency.barrier_operations);
    fprintf(output, "  Deadlocks Detected: %" PRIu32 "\n", stats.concurrency.deadlocks_detected);
    fprintf(output, "  Race Conditions: %" PRIu32 "\n\n",
            stats.concurrency.race_conditions_detected);

    // System info
    uint64_t uptime_ns = asthra_statistics_get_time_ns() - stats.system_start_time_ns;
    fprintf(output, "System Information:\n");
    fprintf(output, "  Uptime: %.2f seconds\n", (double)uptime_ns / 1000000000.0);
    fprintf(output, "  Statistics Enabled: %s\n", stats.statistics_enabled ? "Yes" : "No");
}

char *asthra_statistics_export_json(void) {
    AsthraGlobalStatistics stats = asthra_statistics_get_snapshot();

    // Allocate buffer for JSON (estimate size)
    size_t buffer_size = 4096;
    char *json = malloc(buffer_size);
    if (!json)
        return NULL;

    int len = snprintf(
        json, buffer_size,
        "{\n"
        "  \"memory\": {\n"
        "    \"total_allocations\": %" PRIu64 ",\n"
        "    \"total_deallocations\": %" PRIu64 ",\n"
        "    \"bytes_allocated\": %" PRIu64 ",\n"
        "    \"bytes_deallocated\": %" PRIu64 ",\n"
        "    \"current_memory_usage\": %" PRIu64 ",\n"
        "    \"peak_memory_usage\": %" PRIu64 ",\n"
        "    \"allocation_failures\": %" PRIu32 ",\n"
        "    \"alignment_allocations\": %" PRIu32 "\n"
        "  },\n"
        "  \"gc\": {\n"
        "    \"collections_performed\": %" PRIu64 ",\n"
        "    \"total_collection_time_ns\": %" PRIu64 ",\n"
        "    \"bytes_collected\": %" PRIu64 ",\n"
        "    \"objects_collected\": %" PRIu64 ",\n"
        "    \"min_collection_time_ns\": %" PRIu64 ",\n"
        "    \"max_collection_time_ns\": %" PRIu64 "\n"
        "  },\n"
        "  \"performance\": {\n"
        "    \"files_compiled\": %" PRIu32 ",\n"
        "    \"lines_processed\": %" PRIu32 ",\n"
        "    \"symbols_created\": %" PRIu32 ",\n"
        "    \"compilation_time_ns\": %" PRIu64 ",\n"
        "    \"semantic_analysis_time_ns\": %" PRIu64 ",\n"
        "    \"code_generation_time_ns\": %" PRIu64 ",\n"
        "    \"optimization_time_ns\": %" PRIu64 "\n"
        "  },\n"
        "  \"concurrency\": {\n"
        "    \"threads_created\": %" PRIu32 ",\n"
        "    \"active_threads\": %" PRIu32 ",\n"
        "    \"synchronization_operations\": %" PRIu64 ",\n"
        "    \"lock_contentions\": %" PRIu64 ",\n"
        "    \"barrier_operations\": %" PRIu64 ",\n"
        "    \"deadlocks_detected\": %" PRIu32 ",\n"
        "    \"race_conditions_detected\": %" PRIu32 "\n"
        "  },\n"
        "  \"system\": {\n"
        "    \"statistics_enabled\": %s,\n"
        "    \"uptime_ns\": %" PRIu64 "\n"
        "  }\n"
        "}",
        stats.memory.total_allocations, stats.memory.total_deallocations,
        stats.memory.bytes_allocated, stats.memory.bytes_deallocated,
        stats.memory.current_memory_usage, stats.memory.peak_memory_usage,
        stats.memory.allocation_failures, stats.memory.alignment_allocations,
        stats.gc.collections_performed, stats.gc.total_collection_time_ns, stats.gc.bytes_collected,
        stats.gc.objects_collected, stats.gc.min_collection_time_ns,
        stats.gc.max_collection_time_ns, stats.performance.files_compiled,
        stats.performance.lines_processed, stats.performance.symbols_created,
        stats.performance.compilation_time_ns, stats.performance.semantic_analysis_time_ns,
        stats.performance.code_generation_time_ns, stats.performance.optimization_time_ns,
        stats.concurrency.threads_created, stats.concurrency.active_threads,
        stats.concurrency.synchronization_operations, stats.concurrency.lock_contentions,
        stats.concurrency.barrier_operations, stats.concurrency.deadlocks_detected,
        stats.concurrency.race_conditions_detected, stats.statistics_enabled ? "true" : "false",
        asthra_statistics_get_time_ns() - stats.system_start_time_ns);

    if (len >= (int)buffer_size) {
        // Buffer too small, reallocate
        buffer_size = (size_t)len + 1;
        char *new_json = realloc(json, buffer_size);
        if (!new_json) {
            free(json);
            return NULL;
        }
        json = new_json;
        // Re-format with larger buffer
        snprintf(json, buffer_size, "%s", json); // This is safe since len was calculated
    }

    return json;
}

bool asthra_statistics_save_to_file(const char *filename, bool json_format) {
    if (!filename)
        return false;

    FILE *file = fopen(filename, "w");
    if (!file)
        return false;

    asthra_statistics_print_report(file, json_format);
    fclose(file);
    return true;
}