/**
 * Asthra Programming Language
 * Profile-Guided Optimization (PGO) Performance Testing Common Utilities
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Common utilities, data structures, and constants for PGO performance testing
 */

#ifndef ASTHRA_TEST_PGO_PERFORMANCE_COMMON_H
#define ASTHRA_TEST_PGO_PERFORMANCE_COMMON_H

#include "test_optimization_common.h"
#include <sys/time.h>
#include <time.h>
#include <math.h>

// PGO Performance test configuration
#define PGO_BENCHMARK_ITERATIONS 10
#define PGO_PERFORMANCE_THRESHOLD_PERCENT 2.0
#define PGO_MAX_COMMAND_LENGTH 1024
#define PGO_MAX_OUTPUT_LENGTH 4096

// Performance measurement structure
typedef struct {
    double min_time_ms;
    double max_time_ms;
    double avg_time_ms;
    double std_dev_ms;
    double median_time_ms;
    size_t sample_count;
} PGOPerformanceStats;

// Benchmark result comparison
typedef struct {
    PGOPerformanceStats baseline;
    PGOPerformanceStats optimized;
    double improvement_percent;
    double confidence_level;
    bool statistically_significant;
    bool meets_threshold;
} PGOBenchmarkComparison;

// Benchmark types
typedef enum {
    PGO_BENCHMARK_FIBONACCI,
    PGO_BENCHMARK_MATRIX,
    PGO_BENCHMARK_SORTING,
    PGO_BENCHMARK_SIMPLE
} PGOBenchmarkType;

// Function declarations

// Timing utilities
double pgo_get_time_ms(void);

// Statistical analysis
PGOPerformanceStats pgo_calculate_stats(double *samples, size_t count);
PGOBenchmarkComparison pgo_compare_performance(PGOPerformanceStats baseline, PGOPerformanceStats optimized);

// Benchmark execution
bool pgo_run_binary_benchmark(const char *binary_path, const char *args, PGOPerformanceStats *stats);

// Test program generation
bool pgo_create_test_program(const char *filename, PGOBenchmarkType program_type);

// Binary validation
bool pgo_validate_binaries(const char *baseline_binary, const char *optimized_binary);

// Performance reporting
void pgo_print_performance_stats(const char *label, const PGOPerformanceStats *stats);
void pgo_print_benchmark_comparison(const char *benchmark_name, const PGOBenchmarkComparison *comparison);

#endif // ASTHRA_TEST_PGO_PERFORMANCE_COMMON_H 
