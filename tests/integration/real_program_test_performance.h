/**
 * Real Program Test Performance Header
 *
 * Contains functions for performance testing, benchmarking, and
 * performance threshold validation.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef REAL_PROGRAM_TEST_PERFORMANCE_H
#define REAL_PROGRAM_TEST_PERFORMANCE_H

#include "real_program_test_data.h"
#include "real_program_test_suite.h"
#include "real_program_test_utils.h"
#include <stdbool.h>
#include <stddef.h>

// =============================================================================
// PERFORMANCE STRUCTURES
// =============================================================================

typedef struct {
    size_t max_parse_time_ms;
    size_t max_memory_kb;
    bool must_complete_successfully;
    bool must_detect_errors;
    double min_parse_speed_loc_per_ms;
} PerformanceThreshold;

// =============================================================================
// PERFORMANCE TESTING FUNCTIONS
// =============================================================================

/**
 * Test parsing performance with large programs
 * @param line_count Number of lines to generate
 * @param config Test configuration
 * @return Performance metrics
 */
PerformanceMetrics test_large_program_parsing(size_t line_count, const TestSuiteConfig *config);

/**
 * Test parsing performance with deeply nested structures
 * @param depth Nesting depth
 * @param config Test configuration
 * @return Performance metrics
 */
PerformanceMetrics test_deeply_nested_structures(size_t depth, const TestSuiteConfig *config);

/**
 * Validate that a program meets performance thresholds
 * @param program The program source code
 * @param threshold Performance requirements
 * @return true if thresholds are met
 */
bool validate_performance_thresholds(const char *program, PerformanceThreshold threshold);

/**
 * Benchmark parsing speed over multiple iterations
 * @param source The source code to benchmark
 * @param iterations Number of iterations
 * @param config Test configuration
 * @return Average performance metrics
 */
PerformanceMetrics benchmark_parsing_speed(const char *source, size_t iterations,
                                           const TestSuiteConfig *config);

/**
 * Run a suite of performance benchmarks
 * @param benchmarks Array of benchmarks to run
 * @param benchmark_count Number of benchmarks
 * @param config Test configuration
 * @return true if all required benchmarks pass
 */
bool run_performance_benchmark_suite(const PerformanceBenchmark *benchmarks, size_t benchmark_count,
                                     const TestSuiteConfig *config);

#endif // REAL_PROGRAM_TEST_PERFORMANCE_H