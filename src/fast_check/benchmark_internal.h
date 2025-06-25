/*
 * Asthra Fast Check Benchmark Internal Header
 * Week 16: Performance Optimization & Testing
 *
 * Internal types and functions shared between benchmark modules.
 */

#ifndef ASTHRA_BENCHMARK_INTERNAL_H
#define ASTHRA_BENCHMARK_INTERNAL_H

#include "performance_profiler.h"
#include <stdbool.h>
#include <stddef.h>

// =============================================================================
// Stub Types (from benchmark_stubs.c)
// =============================================================================

typedef struct {
    bool initialized;
    int dummy_field;
} FastCheckEngine;

typedef struct {
    bool success;
    double duration_ms;
    int error_count;
    int warning_count;
} FastCheckResult;

// =============================================================================
// Benchmark Configuration Types
// =============================================================================

typedef struct {
    const char *name;
    const char *description;
    double target_time_ms;
    int file_count;
    const char *complexity;
} BenchmarkSuite;

// =============================================================================
// Benchmark Report Types
// =============================================================================

typedef struct {
    char suite_name[128];
    double total_time_ms;
    double average_time_ms;
    double min_time_ms;
    double max_time_ms;
    int file_count;
    bool meets_target;
    double target_time_ms;
    double cache_hit_rate;
    size_t peak_memory_mb;
    char status[64];
} BenchmarkReport;

// =============================================================================
// Stub Functions (implemented in benchmark_stubs.c)
// =============================================================================

FastCheckEngine *fast_check_engine_create(void);
void fast_check_engine_destroy(FastCheckEngine *engine);
FastCheckResult *fast_check_file(FastCheckEngine *engine, const char *filename);
void fast_check_result_destroy(FastCheckResult *result);

// =============================================================================
// Configuration Functions (implemented in benchmark_config.c)
// =============================================================================

extern const BenchmarkSuite *get_benchmark_suites(void);
extern int get_benchmark_suite_count(void);
const char *get_code_template(const char *complexity);

// =============================================================================
// Utility Functions (implemented in benchmark_execution.c)
// =============================================================================

double get_current_time_ms(void);
bool create_benchmark_file(const char *filename, const char *template, int index);
void cleanup_benchmark_file(const char *filename);

// =============================================================================
// Benchmark Execution Functions (implemented in benchmark_execution.c)
// =============================================================================

BenchmarkReport run_single_benchmark_suite(const BenchmarkSuite *suite);
BenchmarkReport run_cache_performance_benchmark(void);

// =============================================================================
// Report Functions (implemented in benchmark_report.c)
// =============================================================================

void print_benchmark_header(void);
void print_benchmark_report(const BenchmarkReport *report);
void print_benchmark_summary(const BenchmarkReport *reports, int count);

// =============================================================================
// Main Benchmark Function (implemented in benchmark_runner.c)
// =============================================================================

int run_performance_benchmarks(void);

#endif // ASTHRA_BENCHMARK_INTERNAL_H