/**
 * Asthra Programming Language v1.2 Comprehensive Test Suite - Core Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core utilities for the comprehensive v1.2 test suite including
 * test context management and basic utility functions.
 */

#include "test_comprehensive_core.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// =============================================================================
// GLOBAL TEST REGISTRY AND STATISTICS
// =============================================================================

// Global test statistics
static AsthraTestStatistics *g_global_stats = NULL;

// =============================================================================
// TEST CONTEXT MANAGEMENT
// =============================================================================

AsthraExtendedTestContext* asthra_test_context_create_extended(const AsthraExtendedTestMetadata *metadata) {
    AsthraV12TestContext *ctx = calloc(1, sizeof(AsthraV12TestContext));
    if (!ctx) {
        return NULL;
    }

    // Initialize base context
    ctx->base.metadata = metadata->base;
    ctx->base.result = ASTHRA_TEST_PASS;
    ctx->base.start_time_ns = 0;
    ctx->base.end_time_ns = 0;
    ctx->base.duration_ns = 0;
    ctx->base.error_message = NULL;
    ctx->base.assertions_in_test = 0;
    ctx->base.global_stats = g_global_stats;

    // Initialize v1.2 specific context
    ctx->extended_metadata = *metadata;

    // Initialize benchmark results
    ctx->benchmark.min_duration_ns = UINT64_MAX;
    ctx->benchmark.max_duration_ns = 0;
    ctx->benchmark.avg_duration_ns = 0;
    ctx->benchmark.median_duration_ns = 0;
    ctx->benchmark.std_deviation_ns = 0;
    ctx->benchmark.iterations = 0;
    ctx->benchmark.memory_peak_bytes = 0;
    ctx->benchmark.memory_avg_bytes = 0;
    ctx->benchmark.throughput_ops_per_sec = 0.0;

    // Initialize security results
    ctx->security.constant_time_verified = false;
    ctx->security.side_channel_resistant = false;
    ctx->security.memory_secure_zeroed = false;
    ctx->security.entropy_sufficient = false;
    ctx->security.timing_variance_ns = 0;
    ctx->security.timing_samples = 0;
    ctx->security.security_notes = NULL;

    // Initialize test data
    ctx->test_data = NULL;
    ctx->test_data_size = 0;
    atomic_store(&ctx->should_abort, false);
    ctx->iteration_count = 0;

    return ctx;
}

void asthra_test_context_destroy_extended(AsthraExtendedTestContext *ctx) {
    if (!ctx) return;

    if (ctx->test_data) {
        free(ctx->test_data);
    }

    free(ctx);
}

void asthra_test_context_reset_extended(AsthraExtendedTestContext *ctx) {
    if (!ctx) return;

    ctx->base.result = ASTHRA_TEST_PASS;
    ctx->base.start_time_ns = 0;
    ctx->base.end_time_ns = 0;
    ctx->base.duration_ns = 0;
    ctx->base.error_message = NULL;
    ctx->base.assertions_in_test = 0;

    // Reset benchmark results
    ctx->benchmark.min_duration_ns = UINT64_MAX;
    ctx->benchmark.max_duration_ns = 0;
    ctx->benchmark.avg_duration_ns = 0;
    ctx->benchmark.median_duration_ns = 0;
    ctx->benchmark.std_deviation_ns = 0;
    ctx->benchmark.iterations = 0;
    ctx->benchmark.memory_peak_bytes = 0;
    ctx->benchmark.memory_avg_bytes = 0;
    ctx->benchmark.throughput_ops_per_sec = 0.0;

    // Reset security results
    ctx->security.constant_time_verified = false;
    ctx->security.side_channel_resistant = false;
    ctx->security.memory_secure_zeroed = false;
    ctx->security.entropy_sufficient = false;
    ctx->security.timing_variance_ns = 0;
    ctx->security.timing_samples = 0;
    ctx->security.security_notes = NULL;

    atomic_store(&ctx->should_abort, false);
    ctx->iteration_count = 0;
}

// =============================================================================
// GLOBAL STATISTICS MANAGEMENT
// =============================================================================

void asthra_set_global_stats(AsthraTestStatistics *stats) {
    g_global_stats = stats;
}

AsthraTestStatistics* asthra_get_global_stats(void) {
    return g_global_stats;
}

 
