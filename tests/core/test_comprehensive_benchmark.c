/**
 * Asthra Programming Language v1.2 Comprehensive Test Suite - Benchmark Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Benchmark and performance measurement utilities for the comprehensive v1.2 test suite.
 */

#include "test_comprehensive_benchmark.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// =============================================================================
// BENCHMARK UTILITIES
// =============================================================================

void asthra_benchmark_start(AsthraV12TestContext *ctx) {
    if (!ctx)
        return;

    ctx->base.start_time_ns = asthra_test_get_time_ns();
    ctx->benchmark.iterations = 0;
}

void asthra_benchmark_end(AsthraV12TestContext *ctx) {
    if (!ctx)
        return;

    ctx->base.end_time_ns = asthra_test_get_time_ns();
    ctx->base.duration_ns = ctx->base.end_time_ns - ctx->base.start_time_ns;

    // Update benchmark statistics
    if (ctx->base.duration_ns < ctx->benchmark.min_duration_ns) {
        ctx->benchmark.min_duration_ns = ctx->base.duration_ns;
    }
    if (ctx->base.duration_ns > ctx->benchmark.max_duration_ns) {
        ctx->benchmark.max_duration_ns = ctx->base.duration_ns;
    }

    // Calculate average duration
    if (ctx->benchmark.iterations > 0) {
        ctx->benchmark.avg_duration_ns = ctx->base.duration_ns / ctx->benchmark.iterations;
    } else {
        ctx->benchmark.avg_duration_ns = ctx->base.duration_ns;
    }
}

void asthra_benchmark_iteration(AsthraV12TestContext *ctx) {
    if (!ctx)
        return;

    ctx->benchmark.iterations++;
    ctx->iteration_count++;
}

void asthra_benchmark_calculate_stats(AsthraV12TestContext *ctx, uint64_t *durations,
                                      size_t count) {
    if (!ctx || !durations || count == 0)
        return;

    // Calculate min, max, and average
    uint64_t min = durations[0];
    uint64_t max = durations[0];
    uint64_t sum = 0;

    for (size_t i = 0; i < count; i++) {
        if (durations[i] < min)
            min = durations[i];
        if (durations[i] > max)
            max = durations[i];
        sum += durations[i];
    }

    ctx->benchmark.min_duration_ns = min;
    ctx->benchmark.max_duration_ns = max;
    ctx->benchmark.avg_duration_ns = sum / count;

    // Calculate median
    // Note: This assumes durations array is already sorted
    if (count % 2 == 0) {
        ctx->benchmark.median_duration_ns = (durations[count / 2 - 1] + durations[count / 2]) / 2;
    } else {
        ctx->benchmark.median_duration_ns = durations[count / 2];
    }

    // Calculate standard deviation
    double mean = (double)ctx->benchmark.avg_duration_ns;
    double variance = 0.0;

    for (size_t i = 0; i < count; i++) {
        double diff = (double)durations[i] - mean;
        variance += diff * diff;
    }
    variance /= (double)count;

    ctx->benchmark.std_deviation_ns = (uint64_t)sqrt(variance);
    ctx->benchmark.iterations = count;
}

void asthra_benchmark_calculate_throughput(AsthraV12TestContext *ctx, size_t operations_count) {
    if (!ctx || ctx->benchmark.avg_duration_ns == 0)
        return;

    // Calculate operations per second
    double duration_seconds = (double)ctx->benchmark.avg_duration_ns / 1e9;
    ctx->benchmark.throughput_ops_per_sec = (double)operations_count / duration_seconds;
}

void asthra_benchmark_print_results(const AsthraV12TestContext *ctx) {
    if (!ctx)
        return;

    printf("\n=== Benchmark Results ===\n");
    printf("Iterations: %zu\n", ctx->benchmark.iterations);
    printf("Min Duration: %.3f ms\n", (double)ctx->benchmark.min_duration_ns / 1e6);
    printf("Max Duration: %.3f ms\n", (double)ctx->benchmark.max_duration_ns / 1e6);
    printf("Avg Duration: %.3f ms\n", (double)ctx->benchmark.avg_duration_ns / 1e6);
    printf("Median Duration: %.3f ms\n", (double)ctx->benchmark.median_duration_ns / 1e6);
    printf("Std Deviation: %.3f ms\n", (double)ctx->benchmark.std_deviation_ns / 1e6);
    printf("Throughput: %.0f ops/sec\n", ctx->benchmark.throughput_ops_per_sec);
    printf("Memory Peak: %zu bytes\n", ctx->benchmark.memory_peak_bytes);
    printf("Memory Avg: %zu bytes\n", ctx->benchmark.memory_avg_bytes);
}
