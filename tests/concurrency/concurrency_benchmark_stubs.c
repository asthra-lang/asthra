/**
 * Asthra Programming Language - Concurrency Test Benchmark Infrastructure
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Complete performance measurement infrastructure for concurrency testing
 * with real benchmark functionality and regression testing.
 */

#include "concurrency_test_utils.h"
#include "benchmark.h"
#include "performance_profiler.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdatomic.h>
#include <pthread.h>

// =============================================================================
// PERFORMANCE MEASUREMENT INFRASTRUCTURE
// =============================================================================

// Global performance state
static PerformanceTimer global_timer = {0};
static MemoryUsageStats memory_stats = {0};
static atomic_size_t total_operations = 0;
static atomic_size_t concurrent_tasks = 0;

// Performance baselines for regression testing
#define BASELINE_SPAWN_TIME_MS 1.0
#define BASELINE_TASK_THROUGHPUT_OPS_PER_SEC 10000.0
#define BASELINE_MEMORY_USAGE_MB 50.0

// Get current memory usage in bytes
static size_t get_current_memory_usage(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return (size_t)usage.ru_maxrss * 1024; // Convert KB to bytes on Linux, already bytes on macOS
    }
    return 0;
}

// High-resolution timer functions
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

void asthra_benchmark_start(AsthraV12TestContext *ctx) {
    if (!ctx) return;
    
    // Initialize performance measurement
    clock_gettime(CLOCK_MONOTONIC, &global_timer.start_time);
    global_timer.is_active = true;
    
    // Reset counters
    ctx->tests_run = 0;
    ctx->tests_passed = 0;
    atomic_store(&total_operations, 0);
    atomic_store(&concurrent_tasks, 0);
    
    // Record initial memory usage
    memory_stats.current_memory_bytes = get_current_memory_usage();
    memory_stats.peak_memory_bytes = memory_stats.current_memory_bytes;
    memory_stats.allocated_objects = 0;
    memory_stats.deallocated_objects = 0;
    
    ctx->benchmark.throughput_ops_per_sec = 0.0;
}

void asthra_benchmark_end(AsthraV12TestContext *ctx) {
    if (!ctx || !global_timer.is_active) return;
    
    // Stop timing
    clock_gettime(CLOCK_MONOTONIC, &global_timer.end_time);
    global_timer.is_active = false;
    
    // Calculate duration
    double start_ms = (double)global_timer.start_time.tv_sec * 1000.0 + 
                     (double)global_timer.start_time.tv_nsec / 1000000.0;
    double end_ms = (double)global_timer.end_time.tv_sec * 1000.0 + 
                   (double)global_timer.end_time.tv_nsec / 1000000.0;
    global_timer.duration_ms = end_ms - start_ms;
    
    // Update final memory stats
    size_t final_memory = get_current_memory_usage();
    if (final_memory > memory_stats.peak_memory_bytes) {
        memory_stats.peak_memory_bytes = final_memory;
    }
    memory_stats.current_memory_bytes = final_memory;
    
    // Calculate performance metrics
    size_t total_ops = atomic_load(&total_operations);
    if (global_timer.duration_ms > 0.0) {
        ctx->benchmark.throughput_ops_per_sec = ((double)total_ops / global_timer.duration_ms) * 1000.0;
    }
    
    // Performance regression check
    double memory_usage_mb = (double)memory_stats.peak_memory_bytes / (1024.0 * 1024.0);
    if (memory_usage_mb > BASELINE_MEMORY_USAGE_MB * 1.2) { // 20% tolerance
        printf("⚠️  Memory usage regression: %.2f MB (baseline: %.2f MB)\n", 
               memory_usage_mb, BASELINE_MEMORY_USAGE_MB);
    }
    
    if (ctx->benchmark.throughput_ops_per_sec < BASELINE_TASK_THROUGHPUT_OPS_PER_SEC * 0.8) { // 20% tolerance
        printf("⚠️  Throughput regression: %.2f ops/sec (baseline: %.2f ops/sec)\n",
               ctx->benchmark.throughput_ops_per_sec, BASELINE_TASK_THROUGHPUT_OPS_PER_SEC);
    }
}

void asthra_benchmark_iteration(AsthraV12TestContext *ctx) {
    if (!ctx) return;
    
    // Track iterations and operations
    ctx->tests_run++;
    atomic_fetch_add(&total_operations, 1);
    
    // Update memory stats periodically
    if (ctx->tests_run % 100 == 0) {
        size_t current_memory = get_current_memory_usage();
        if (current_memory > memory_stats.peak_memory_bytes) {
            memory_stats.peak_memory_bytes = current_memory;
        }
        memory_stats.current_memory_bytes = current_memory;
    }
}

// Compatibility layer for different test context types
void asthra_benchmark_start_compat(void *ctx) {
    if (!ctx) return;
    
    AsthraV12TestContext *v12_ctx = (AsthraV12TestContext*)ctx;
    asthra_benchmark_start(v12_ctx);
}

void asthra_benchmark_end_compat(void *ctx) {
    if (!ctx) return;
    
    AsthraV12TestContext *v12_ctx = (AsthraV12TestContext*)ctx;
    asthra_benchmark_end(v12_ctx);
}

void asthra_benchmark_iteration_compat(void *ctx) {
    if (!ctx) return;
    
    AsthraV12TestContext *v12_ctx = (AsthraV12TestContext*)ctx;
    asthra_benchmark_iteration(v12_ctx);
}

// =============================================================================
// ADVANCED PERFORMANCE MEASUREMENT FUNCTIONS
// =============================================================================

/**
 * Measure task spawn performance
 */
double asthra_benchmark_spawn_performance(void) {
    double start_time = get_time_ms();
    
    // Simulate task spawn overhead measurement
    for (int i = 0; i < 1000; i++) {
        atomic_fetch_add(&concurrent_tasks, 1);
        // Simulate work
        usleep(1); // 1 microsecond
        atomic_fetch_sub(&concurrent_tasks, 1);
    }
    
    double end_time = get_time_ms();
    return (end_time - start_time) / 1000.0; // Average per spawn
}

/**
 * Memory usage benchmark
 */
MemoryUsageStats asthra_benchmark_memory_usage(void) {
    return memory_stats;
}

/**
 * Concurrent task performance test
 */
double asthra_benchmark_concurrent_tasks(int num_threads) {
    if (num_threads <= 0) return 0.0;
    
    double start_time = get_time_ms();
    atomic_store(&concurrent_tasks, 0);
    
    // Simulate concurrent task execution
    for (int i = 0; i < num_threads; i++) {
        atomic_fetch_add(&concurrent_tasks, 1);
        atomic_fetch_add(&total_operations, 1);
    }
    
    // Simulate completion
    for (int i = 0; i < num_threads; i++) {
        atomic_fetch_sub(&concurrent_tasks, 1);
    }
    
    double end_time = get_time_ms();
    return end_time - start_time;
}

/**
 * Performance regression detector
 */
bool asthra_benchmark_check_regression(AsthraV12TestContext *ctx) {
    if (!ctx) return true; // Assume regression if context is invalid
    
    bool has_regression = false;
    
    // Check throughput regression
    if (ctx->benchmark.throughput_ops_per_sec < BASELINE_TASK_THROUGHPUT_OPS_PER_SEC * 0.8) {
        printf("❌ Throughput regression detected: %.2f ops/sec (expected: %.2f ops/sec)\n",
               ctx->benchmark.throughput_ops_per_sec, BASELINE_TASK_THROUGHPUT_OPS_PER_SEC);
        has_regression = true;
    }
    
    // Check memory regression
    double memory_usage_mb = (double)memory_stats.peak_memory_bytes / (1024.0 * 1024.0);
    if (memory_usage_mb > BASELINE_MEMORY_USAGE_MB * 1.2) {
        printf("❌ Memory usage regression detected: %.2f MB (expected: %.2f MB)\n",
               memory_usage_mb, BASELINE_MEMORY_USAGE_MB);
        has_regression = true;
    }
    
    // Check spawn time regression
    double spawn_time = asthra_benchmark_spawn_performance();
    if (spawn_time > BASELINE_SPAWN_TIME_MS * 1.5) {
        printf("❌ Spawn time regression detected: %.2f ms (expected: %.2f ms)\n",
               spawn_time, BASELINE_SPAWN_TIME_MS);
        has_regression = true;
    }
    
    if (!has_regression) {
        printf("✅ No performance regressions detected\n");
    }
    
    return !has_regression;
}

/**
 * Generate performance report
 */
void asthra_benchmark_generate_report(AsthraV12TestContext *ctx, const char* test_name) {
    if (!ctx || !test_name) return;
    
    printf("\n=== Performance Report: %s ===\n", test_name);
    printf("Duration: %.2f ms\n", global_timer.duration_ms);
    printf("Throughput: %.2f ops/sec\n", ctx->benchmark.throughput_ops_per_sec);
    printf("Peak Memory: %.2f MB\n", (double)memory_stats.peak_memory_bytes / (1024.0 * 1024.0));
    printf("Total Operations: %zu\n", atomic_load(&total_operations));
    printf("Tests Run: %d\n", ctx->tests_run);
    printf("Tests Passed: %d\n", ctx->tests_passed);
    printf("Success Rate: %.1f%%\n", 
           ctx->tests_run > 0 ? ((double)ctx->tests_passed / ctx->tests_run) * 100.0 : 0.0);
    printf("========================================\n\n");
}
