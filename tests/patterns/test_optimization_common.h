/*
 * =============================================================================
 * PATTERN OPTIMIZATION TEST COMMON UTILITIES
 * =============================================================================
 * 
 * This header contains shared types, macros, and utility functions used
 * across all pattern optimization test files.
 * 
 * Part of Phase 3.3: Advanced Pattern Matching
 * 
 * =============================================================================
 */

#ifndef TEST_OPTIMIZATION_COMMON_H
#define TEST_OPTIMIZATION_COMMON_H

#include "test_pattern_common.h"
#include <time.h>
#include <sys/resource.h>

// =============================================================================
// OPTIMIZATION-SPECIFIC TYPES (using base types from test_pattern_common.h)
// =============================================================================

// Optimization levels
typedef enum {
    OPTIMIZATION_NONE = 0,
    OPTIMIZATION_BASIC = 1,
    OPTIMIZATION_FULL = 2
} OptimizationLevel;

// Performance measurement structure
typedef struct {
    clock_t start_time;
    clock_t end_time;
    struct rusage start_usage;
    struct rusage end_usage;
} PerformanceMeasurement;

// Pattern performance profile
typedef struct {
    double average_execution_time_ns;
    double cache_hit_rate;
    double branch_misprediction_rate;
    int pattern_hit_counts[20];
    int total_executions;
} PatternPerformanceProfile;

// =============================================================================
// OPTIMIZATION-SPECIFIC UTILITY FUNCTIONS
// =============================================================================

// Optimization test environment
static void setup_optimization_test_environment(void);
static void cleanup_optimization_test_environment(void);
static void init_pattern_optimizer(void);
static void cleanup_pattern_optimizer(void);
static void init_performance_profiler(void);
static void cleanup_performance_profiler(void);

// Performance measurement
static PerformanceMeasurement start_performance_measurement(void);
static void end_performance_measurement(PerformanceMeasurement* measurement);
static double get_cpu_time_ms(const PerformanceMeasurement* measurement);
static long get_memory_usage_kb(const PerformanceMeasurement* measurement);

// Pattern compilation with optimization
static PatternCompilationResult* compile_patterns(ASTNode* ast, OptimizationLevel level);

// Performance profiling
static void enable_pattern_profiling(PatternCompilationResult* result);
static PatternPerformanceProfile* get_pattern_performance_profile(PatternCompilationResult* result);
static void cleanup_pattern_performance_profile(PatternPerformanceProfile* profile);

// Cleanup functions (using base implementation from test_pattern_common.h)

#endif // TEST_OPTIMIZATION_COMMON_H 
