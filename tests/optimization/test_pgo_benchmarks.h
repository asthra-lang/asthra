/**
 * Asthra Programming Language
 * Profile-Guided Optimization (PGO) Individual Benchmark Tests Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Declarations for individual benchmark test functions
 */

#ifndef ASTHRA_TEST_PGO_BENCHMARKS_H
#define ASTHRA_TEST_PGO_BENCHMARKS_H

#include "test_pgo_performance_common.h"

// Individual benchmark test function declarations
AsthraTestResult test_pgo_fibonacci_benchmark(AsthraTestContext *context);
AsthraTestResult test_pgo_matrix_benchmark(AsthraTestContext *context);
AsthraTestResult test_pgo_sorting_benchmark(AsthraTestContext *context);
AsthraTestResult test_pgo_compilation_performance(AsthraTestContext *context);
AsthraTestResult test_pgo_memory_usage(AsthraTestContext *context);
AsthraTestResult test_pgo_profile_data_quality(AsthraTestContext *context);

#endif // ASTHRA_TEST_PGO_BENCHMARKS_H
