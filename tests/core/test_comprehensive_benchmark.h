/**
 * Asthra Programming Language v1.2 Comprehensive Test Suite - Benchmark Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Benchmark and performance measurement function declarations.
 */

#ifndef ASTHRA_TEST_COMPREHENSIVE_BENCHMARK_H
#define ASTHRA_TEST_COMPREHENSIVE_BENCHMARK_H

#include "test_comprehensive_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BENCHMARK FUNCTION DECLARATIONS
// =============================================================================

// Basic benchmark operations
void asthra_benchmark_start(AsthraV12TestContext *ctx);
void asthra_benchmark_end(AsthraV12TestContext *ctx);
void asthra_benchmark_iteration(AsthraV12TestContext *ctx);

// Statistical analysis
void asthra_benchmark_calculate_stats(AsthraV12TestContext *ctx, uint64_t *durations, size_t count);
void asthra_benchmark_calculate_throughput(AsthraV12TestContext *ctx, size_t operations_count);

// Results reporting
void asthra_benchmark_print_results(const AsthraV12TestContext *ctx);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_COMPREHENSIVE_BENCHMARK_H
