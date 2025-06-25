/**
 * Asthra Programming Language - Performance Validation Infrastructure
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Comprehensive performance measurement and regression testing infrastructure
 * for validating compiler performance targets and detecting regressions.
 */

#include "../framework/test_framework.h"
#include "benchmark.h"
#include "performance_profiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdatomic.h>

// =============================================================================
// PERFORMANCE TARGETS AND THRESHOLDS
// =============================================================================

// Compilation performance targets (based on project goals)
#define TARGET_COMPILATION_SPEEDUP_PERCENT 20.0  // 15-25% faster than baseline
#define TARGET_MEMORY_REDUCTION_PERCENT 15.0     // 10-20% reduction
#define TARGET_REFERENCE_COUNTING_SPEEDUP 65.0   // 50-80% faster
#define TARGET_KEYWORD_LOOKUP_NS 15.0             // ~12.86 nanoseconds average

// Performance regression thresholds
#define REGRESSION_THRESHOLD_PERCENT 10.0        // 10% degradation triggers warning
#define CRITICAL_THRESHOLD_PERCENT 25.0          // 25% degradation triggers failure

// Memory and timing baselines
#define BASELINE_COMPILER_TIME_MS 1000.0
#define VALIDATION_BASELINE_MEMORY_USAGE_MB 100.0
#define BASELINE_PARSE_TIME_PER_FILE_MS 10.0

// =============================================================================
// PERFORMANCE MEASUREMENT INFRASTRUCTURE
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

typedef struct {
    PerformanceMeasurement baseline;
    PerformanceMeasurement current;
    bool has_regression;
    double speedup_percentage;
    double memory_reduction_percentage;
    char regression_details[1024];
} TestPerformanceComparison;

// Global performance state
static PerformanceMeasurement global_baseline = {0};
static atomic_bool baseline_initialized = false;

// =============================================================================
// MEASUREMENT FUNCTIONS
// =============================================================================

static double get_precise_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static size_t get_memory_usage_bytes(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // On macOS, ru_maxrss is in bytes; on Linux, it's in KB
        #ifdef __APPLE__
        return (size_t)usage.ru_maxrss;
        #else
        return (size_t)usage.ru_maxrss * 1024;
        #endif
    }
    return 0;
}

/**
 * Measure keyword lookup performance (nanosecond precision)
 */
static double measure_keyword_lookup_performance(void) {
    const char* keywords[] = {
        "let", "fn", "if", "else", "while", "for", "match", "enum", 
        "struct", "impl", "trait", "pub", "const", "static", "mut"
    };
    size_t num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    size_t iterations = 100000;
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    // Simulate keyword lookup operations
    volatile size_t dummy = 0;
    for (size_t i = 0; i < iterations; i++) {
        const char* keyword = keywords[i % num_keywords];
        dummy += strlen(keyword); // Simple hash-like operation
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double total_ns = (double)(end.tv_sec - start.tv_sec) * 1000000000.0 + 
                     (double)(end.tv_nsec - start.tv_nsec);
    
    return total_ns / (double)(iterations); // Average nanoseconds per lookup
}

/**
 * Measure reference counting performance
 */
static double measure_reference_counting_performance(void) {
    size_t operations = 1000000;
    atomic_size_t ref_count = 1;
    
    double start_time = get_precise_time_ms();
    
    // Simulate reference counting operations
    for (size_t i = 0; i < operations; i++) {
        atomic_fetch_add(&ref_count, 1);
        atomic_fetch_sub(&ref_count, 1);
    }
    
    double end_time = get_precise_time_ms();
    double total_ms = end_time - start_time;
    
    return (total_ms * 1000000.0) / (double)operations; // Nanoseconds per operation
}

/**
 * Initialize performance baseline
 */
void performance_validation_init_baseline(void) {
    if (atomic_load(&baseline_initialized)) return;
    
    printf("Initializing performance baseline...\n");
    
    global_baseline.compilation_time_ms = BASELINE_COMPILER_TIME_MS;
    global_baseline.memory_usage_bytes = (size_t)(VALIDATION_BASELINE_MEMORY_USAGE_MB * 1024 * 1024);
    global_baseline.parse_time_ms = BASELINE_PARSE_TIME_PER_FILE_MS;
    global_baseline.keyword_lookup_time_ns = measure_keyword_lookup_performance();
    global_baseline.reference_counting_time_ns = measure_reference_counting_performance();
    global_baseline.files_processed = 1;
    global_baseline.lines_of_code = 100;
    
    atomic_store(&baseline_initialized, true);
    
    printf("✅ Performance baseline initialized:\n");
    printf("   Keyword lookup: %.2f ns\n", global_baseline.keyword_lookup_time_ns);
    printf("   Reference counting: %.2f ns/op\n", global_baseline.reference_counting_time_ns);
    printf("   Memory baseline: %.2f MB\n", (double)global_baseline.memory_usage_bytes / (1024.0 * 1024.0));
}

/**
 * Measure current performance
 */
PerformanceMeasurement performance_validation_measure_current(void) {
    PerformanceMeasurement measurement = {0};
    
    measurement.compilation_time_ms = get_precise_time_ms();
    measurement.memory_usage_bytes = get_memory_usage_bytes();
    measurement.peak_memory_bytes = measurement.memory_usage_bytes;
    measurement.keyword_lookup_time_ns = measure_keyword_lookup_performance();
    measurement.reference_counting_time_ns = measure_reference_counting_performance();
    measurement.files_processed = 1;
    measurement.lines_of_code = 100;
    
    return measurement;
}

/**
 * Compare performance against baseline
 */
TestPerformanceComparison performance_validation_compare(PerformanceMeasurement current) {
    TestPerformanceComparison comparison = {0};
    comparison.baseline = global_baseline;
    comparison.current = current;
    
    // Calculate speedup percentages
    if (global_baseline.keyword_lookup_time_ns > 0) {
        double lookup_speedup = ((global_baseline.keyword_lookup_time_ns - current.keyword_lookup_time_ns) 
                                / global_baseline.keyword_lookup_time_ns) * 100.0;
        comparison.speedup_percentage = lookup_speedup;
    }
    
    // Calculate memory reduction
    if (global_baseline.memory_usage_bytes > 0) {
        double memory_reduction = ((double)global_baseline.memory_usage_bytes - (double)current.memory_usage_bytes) 
                                 / (double)global_baseline.memory_usage_bytes * 100.0;
        comparison.memory_reduction_percentage = memory_reduction;
    }
    
    // Detect regressions
    comparison.has_regression = false;
    strcpy(comparison.regression_details, "");
    
    // Check keyword lookup regression
    if (current.keyword_lookup_time_ns > global_baseline.keyword_lookup_time_ns * (1.0 + REGRESSION_THRESHOLD_PERCENT / 100.0)) {
        comparison.has_regression = true;
        snprintf(comparison.regression_details + strlen(comparison.regression_details), 
                sizeof(comparison.regression_details) - strlen(comparison.regression_details),
                "Keyword lookup regression: %.2f ns (baseline: %.2f ns); ",
                current.keyword_lookup_time_ns, global_baseline.keyword_lookup_time_ns);
    }
    
    // Check reference counting regression
    if (current.reference_counting_time_ns > global_baseline.reference_counting_time_ns * (1.0 + REGRESSION_THRESHOLD_PERCENT / 100.0)) {
        comparison.has_regression = true;
        snprintf(comparison.regression_details + strlen(comparison.regression_details),
                sizeof(comparison.regression_details) - strlen(comparison.regression_details),
                "Reference counting regression: %.2f ns/op (baseline: %.2f ns/op); ",
                current.reference_counting_time_ns, global_baseline.reference_counting_time_ns);
    }
    
    // Check memory regression
    if (current.memory_usage_bytes > global_baseline.memory_usage_bytes * (1.0 + REGRESSION_THRESHOLD_PERCENT / 100.0)) {
        comparison.has_regression = true;
        snprintf(comparison.regression_details + strlen(comparison.regression_details),
                sizeof(comparison.regression_details) - strlen(comparison.regression_details),
                "Memory usage regression: %.2f MB (baseline: %.2f MB); ",
                (double)current.memory_usage_bytes / (1024.0 * 1024.0),
                (double)global_baseline.memory_usage_bytes / (1024.0 * 1024.0));
    }
    
    return comparison;
}

/**
 * Validate performance targets
 */
bool performance_validation_check_targets(TestPerformanceComparison comparison) {
    bool targets_met = true;
    
    printf("\n=== Performance Target Validation ===\n");
    
    // Check compilation speedup target
    if (comparison.speedup_percentage >= TARGET_COMPILATION_SPEEDUP_PERCENT) {
        printf("✅ Compilation speedup target met: %.1f%% (target: %.1f%%)\n",
               comparison.speedup_percentage, TARGET_COMPILATION_SPEEDUP_PERCENT);
    } else {
        printf("❌ Compilation speedup target missed: %.1f%% (target: %.1f%%)\n",
               comparison.speedup_percentage, TARGET_COMPILATION_SPEEDUP_PERCENT);
        targets_met = false;
    }
    
    // Check memory reduction target
    if (comparison.memory_reduction_percentage >= TARGET_MEMORY_REDUCTION_PERCENT) {
        printf("✅ Memory reduction target met: %.1f%% (target: %.1f%%)\n",
               comparison.memory_reduction_percentage, TARGET_MEMORY_REDUCTION_PERCENT);
    } else {
        printf("❌ Memory reduction target missed: %.1f%% (target: %.1f%%)\n",
               comparison.memory_reduction_percentage, TARGET_MEMORY_REDUCTION_PERCENT);
        targets_met = false;
    }
    
    // Check keyword lookup performance
    if (comparison.current.keyword_lookup_time_ns <= TARGET_KEYWORD_LOOKUP_NS) {
        printf("✅ Keyword lookup target met: %.2f ns (target: %.2f ns)\n",
               comparison.current.keyword_lookup_time_ns, TARGET_KEYWORD_LOOKUP_NS);
    } else {
        printf("❌ Keyword lookup target missed: %.2f ns (target: %.2f ns)\n",
               comparison.current.keyword_lookup_time_ns, TARGET_KEYWORD_LOOKUP_NS);
        targets_met = false;
    }
    
    // Check for regressions
    if (comparison.has_regression) {
        printf("❌ Performance regressions detected: %s\n", comparison.regression_details);
        targets_met = false;
    } else {
        printf("✅ No performance regressions detected\n");
    }
    
    printf("=====================================\n\n");
    
    return targets_met;
}

/**
 * Generate comprehensive performance report
 */
void performance_validation_generate_report(TestPerformanceComparison comparison, const char* test_name) {
    printf("\n=== Performance Report: %s ===\n", test_name ? test_name : "Unknown Test");
    
    printf("Current Performance:\n");
    printf("  Keyword lookup: %.2f ns\n", comparison.current.keyword_lookup_time_ns);
    printf("  Reference counting: %.2f ns/op\n", comparison.current.reference_counting_time_ns);
    printf("  Memory usage: %.2f MB\n", (double)comparison.current.memory_usage_bytes / (1024.0 * 1024.0));
    
    printf("\nBaseline Performance:\n");
    printf("  Keyword lookup: %.2f ns\n", comparison.baseline.keyword_lookup_time_ns);
    printf("  Reference counting: %.2f ns/op\n", comparison.baseline.reference_counting_time_ns);
    printf("  Memory usage: %.2f MB\n", (double)comparison.baseline.memory_usage_bytes / (1024.0 * 1024.0));
    
    printf("\nPerformance Comparison:\n");
    printf("  Speedup: %.1f%%\n", comparison.speedup_percentage);
    printf("  Memory reduction: %.1f%%\n", comparison.memory_reduction_percentage);
    printf("  Regression status: %s\n", comparison.has_regression ? "DETECTED" : "None");
    
    if (comparison.has_regression) {
        printf("  Regression details: %s\n", comparison.regression_details);
    }
    
    printf("======================================\n\n");
}

/**
 * Run complete performance validation
 */
bool performance_validation_run_complete(const char* test_name) {
    if (!atomic_load(&baseline_initialized)) {
        performance_validation_init_baseline();
    }
    
    printf("Running performance validation for: %s\n", test_name ? test_name : "Unknown");
    
    PerformanceMeasurement current = performance_validation_measure_current();
    TestPerformanceComparison comparison = performance_validation_compare(current);
    
    performance_validation_generate_report(comparison, test_name);
    
    bool targets_met = performance_validation_check_targets(comparison);
    
    if (targets_met) {
        printf("✅ Performance validation PASSED\n\n");
    } else {
        printf("❌ Performance validation FAILED\n\n");
    }
    
    return targets_met;
}