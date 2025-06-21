/**
 * Asthra Programming Language - Performance Validation Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Header for comprehensive performance measurement and regression testing
 */

#ifndef ASTHRA_PERFORMANCE_VALIDATION_H
#define ASTHRA_PERFORMANCE_VALIDATION_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PERFORMANCE MEASUREMENT STRUCTURES
// =============================================================================

typedef struct {
    double compilation_time_ms;
    double parse_time_ms;
    double semantic_analysis_time_ms;
    double codegen_time_ms;
    size_t memory_usage_bytes;
    size_t peak_memory_bytes;
    double reference_counting_time_ns;
    double keyword_lookup_time_ns;
    size_t files_processed;
    size_t lines_of_code;
} PerformanceMeasurement;

typedef struct TestPerformanceComparison {
    PerformanceMeasurement baseline;
    PerformanceMeasurement current;
    bool has_regression;
    double speedup_percentage;
    double memory_reduction_percentage;
    char regression_details[1024];
} TestPerformanceComparison;

// =============================================================================
// PERFORMANCE VALIDATION FUNCTIONS
// =============================================================================

/**
 * Initialize performance baseline measurements
 */
void performance_validation_init_baseline(void);

/**
 * Measure current performance metrics
 */
PerformanceMeasurement performance_validation_measure_current(void);

/**
 * Compare current performance against baseline
 */
TestPerformanceComparison performance_validation_compare(PerformanceMeasurement current);

/**
 * Validate performance against project targets
 */
bool performance_validation_check_targets(TestPerformanceComparison comparison);

/**
 * Generate comprehensive performance report
 */
void performance_validation_generate_report(TestPerformanceComparison comparison, const char* test_name);

/**
 * Run complete performance validation (convenience function)
 */
bool performance_validation_run_complete(const char* test_name);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PERFORMANCE_VALIDATION_H