/*
 * Asthra Fast Check Performance Profiler
 * Week 16: Performance Optimization & Testing
 *
 * Provides comprehensive performance monitoring, profiling, and optimization
 * tools for the fast check system to achieve target performance metrics.
 */

#ifndef ASTHRA_FAST_CHECK_PERFORMANCE_PROFILER_H
#define ASTHRA_FAST_CHECK_PERFORMANCE_PROFILER_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Performance Measurement Data Structures
// =============================================================================

// High-resolution timing
typedef struct {
    struct timespec start_time;
    struct timespec end_time;
    double duration_ms;
    bool is_active;
} PerformanceTimer;

// Memory usage tracking
typedef struct {
    size_t peak_memory_bytes;
    size_t current_memory_bytes;
    size_t allocated_objects;
    size_t deallocated_objects;
    size_t memory_leaks;
} MemoryUsageStats;

// Cache performance metrics
typedef struct {
    uint64_t total_requests;
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint64_t cache_evictions;
    double hit_rate_percentage;
    double average_lookup_time_ms;
} CachePerformanceStats;

// File processing metrics
typedef struct {
    uint32_t files_processed;
    uint32_t lines_analyzed;
    uint32_t symbols_resolved;
    double average_file_time_ms;
    double total_processing_time_ms;
    char fastest_file[256];
    char slowest_file[256];
    double fastest_time_ms;
    double slowest_time_ms;
} FileProcessingStats;

// Parallel processing metrics
typedef struct {
    uint32_t active_threads;
    uint32_t thread_pool_size;
    uint64_t tasks_completed;
    uint64_t tasks_queued;
    double thread_utilization_percentage;
    double parallel_efficiency;
} ParallelProcessingStats;

// Comprehensive performance profile
typedef struct {
    PerformanceTimer overall_timer;
    MemoryUsageStats memory_stats;
    CachePerformanceStats cache_stats;
    FileProcessingStats file_stats;
    ParallelProcessingStats parallel_stats;

    // Target performance metrics
    double target_single_file_ms;    // < 100ms target
    double target_medium_project_ms; // < 500ms target
    double target_large_project_ms;  // < 2s target

    // Performance flags
    bool meets_single_file_target;
    bool meets_medium_project_target;
    bool meets_large_project_target;
    bool overall_performance_acceptable;

    pthread_mutex_t stats_mutex;

    // Historical data for regression detection (last N runs)
    double historical_total_times[10];
    double historical_avg_file_times[10];
    double historical_cache_hit_rates[10];
    size_t historical_peak_memory[10]; // in bytes
    int history_count;
    int history_index; // Circular buffer index
} PerformanceProfile;

// Bottleneck analysis
typedef enum {
    BOTTLENECK_NONE = 0,
    BOTTLENECK_MEMORY_ALLOCATION,
    BOTTLENECK_DISK_IO,
    BOTTLENECK_CACHE_MISSES,
    BOTTLENECK_PARSER_OVERHEAD,
    BOTTLENECK_SEMANTIC_ANALYSIS,
    BOTTLENECK_THREAD_CONTENTION,
    BOTTLENECK_DEPENDENCY_RESOLUTION,
    BOTTLENECK_PERFORMANCE_REGRESSION // New bottleneck type
} BottleneckType;

typedef struct {
    BottleneckType type;
    double impact_percentage;
    char description[512];
    char suggested_optimization[512];
} BottleneckAnalysis;

// =============================================================================
// Performance Profiler Interface
// =============================================================================

// Profiler lifecycle
PerformanceProfile *performance_profiler_create(void);
void performance_profiler_destroy(PerformanceProfile *profile);
void performance_profiler_reset(PerformanceProfile *profile);
void performance_profiler_record_history(PerformanceProfile *profile); // New function

// Timer management
void performance_timer_start(PerformanceTimer *timer);
void performance_timer_stop(PerformanceTimer *timer);
double performance_timer_get_duration_ms(const PerformanceTimer *timer);

// Memory tracking
void performance_track_memory_allocation(PerformanceProfile *profile, size_t bytes);
void performance_track_memory_deallocation(PerformanceProfile *profile, size_t bytes);
void performance_update_peak_memory(PerformanceProfile *profile);

// Cache metrics
void performance_record_cache_hit(PerformanceProfile *profile, double lookup_time_ms);
void performance_record_cache_miss(PerformanceProfile *profile, double lookup_time_ms);
void performance_record_cache_eviction(PerformanceProfile *profile);

// File processing metrics
void performance_record_file_start(PerformanceProfile *profile, const char *filename);
void performance_record_file_complete(PerformanceProfile *profile, const char *filename,
                                      uint32_t lines, uint32_t symbols, double duration_ms);

// Parallel processing metrics
void performance_record_thread_start(PerformanceProfile *profile);
void performance_record_thread_complete(PerformanceProfile *profile);
void performance_record_task_queued(PerformanceProfile *profile);
void performance_record_task_completed(PerformanceProfile *profile);

// =============================================================================
// Performance Analysis & Optimization
// =============================================================================

// Performance target validation
bool performance_meets_single_file_target(const PerformanceProfile *profile, double actual_time_ms);
bool performance_meets_medium_project_target(const PerformanceProfile *profile,
                                             double actual_time_ms);
bool performance_meets_large_project_target(const PerformanceProfile *profile,
                                            double actual_time_ms);

// Bottleneck detection
BottleneckAnalysis *performance_analyze_bottlenecks(const PerformanceProfile *profile,
                                                    size_t *bottleneck_count);
void performance_bottleneck_analysis_destroy(BottleneckAnalysis *analysis, size_t count);

// Optimization recommendations
typedef struct {
    char optimization_type[128];
    char description[512];
    double expected_improvement_percentage;
    bool requires_code_changes;
    char implementation_steps[1024];
} OptimizationRecommendation;

OptimizationRecommendation *
performance_get_optimization_recommendations(const PerformanceProfile *profile,
                                             size_t *recommendation_count);
void performance_optimization_recommendations_destroy(OptimizationRecommendation *recommendations,
                                                      size_t count);

// =============================================================================
// Internal Utility Functions (used across split files)
// =============================================================================

double timespec_to_ms(const struct timespec *ts);
double timespec_diff_ms(const struct timespec *start, const struct timespec *end);
size_t get_memory_usage_kb(void);
double calculate_historical_average_double(const double history_array[], int count);
size_t calculate_historical_average_size_t(const size_t history_array[], int count);
bool performance_detect_regression(const PerformanceProfile *profile,
                                   BottleneckAnalysis *bottleneck);
bool performance_meets_single_file_target(const PerformanceProfile *profile, double actual_time_ms);
bool performance_meets_medium_project_target(const PerformanceProfile *profile,
                                             double actual_time_ms);
bool performance_meets_large_project_target(const PerformanceProfile *profile,
                                            double actual_time_ms);

// =============================================================================
// Benchmarking & Testing
// =============================================================================

// Benchmark test configurations
typedef struct {
    const char *test_name;
    uint32_t file_count;
    uint32_t average_file_size_lines;
    bool enable_caching;
    bool enable_parallel_processing;
    uint32_t thread_count;
} BenchmarkConfig;

// Benchmark results
typedef struct {
    char test_name[128];
    double total_time_ms;
    double average_file_time_ms;
    double cache_hit_rate;
    size_t peak_memory_mb;
    bool meets_performance_targets;
    char performance_summary[512];
} BenchmarkResult;

// Benchmark execution
BenchmarkResult *performance_run_benchmark(const BenchmarkConfig *config, const char **test_files);
void performance_benchmark_result_destroy(BenchmarkResult *result);

// Standard benchmark suites
BenchmarkResult *performance_run_single_file_benchmark(void);
BenchmarkResult *performance_run_medium_project_benchmark(void);
BenchmarkResult *performance_run_large_project_benchmark(void);
BenchmarkResult *performance_run_comprehensive_benchmark_suite(size_t *result_count);

// =============================================================================
// Reporting & Visualization
// =============================================================================

// Report generation
void performance_generate_text_report(const PerformanceProfile *profile, const char *output_file);
void performance_generate_json_report(const PerformanceProfile *profile, const char *output_file);
void performance_generate_html_report(const PerformanceProfile *profile, const char *output_file);

// Console output
void performance_print_summary(const PerformanceProfile *profile);
void performance_print_detailed_stats(const PerformanceProfile *profile);
void performance_print_bottleneck_analysis(const PerformanceProfile *profile);
void performance_print_optimization_recommendations(const PerformanceProfile *profile);

// Performance comparison
typedef struct {
    PerformanceProfile *baseline;
    PerformanceProfile *current;
    double improvement_percentage;
    bool performance_regressed;
    char comparison_summary[512];
} PerformanceComparison;

PerformanceComparison *performance_compare_profiles(PerformanceProfile *baseline,
                                                    PerformanceProfile *current);
void performance_comparison_destroy(PerformanceComparison *comparison);
void performance_print_comparison(const PerformanceComparison *comparison);

// =============================================================================
// Real-Time Monitoring
// =============================================================================

// Real-time monitoring
typedef void (*PerformanceAlertCallback)(const char *alert_message, double severity);

void performance_enable_real_time_monitoring(PerformanceProfile *profile,
                                             PerformanceAlertCallback callback);
void performance_disable_real_time_monitoring(PerformanceProfile *profile);
void performance_set_alert_thresholds(PerformanceProfile *profile, double memory_threshold_mb,
                                      double response_time_threshold_ms);

// =============================================================================
// Integration with Fast Check Engine
// =============================================================================

// Engine performance wrapper
// FastCheckEngine forward declaration (defined in fast_check_engine.h)

// Enhanced engine operations with profiling
struct FastCheckEngine *performance_fast_check_engine_create_with_profiling(void);
PerformanceProfile *performance_get_engine_profile(struct FastCheckEngine *engine);
void performance_fast_check_file_with_profiling(struct FastCheckEngine *engine,
                                                const char *file_path, bool *success,
                                                double *duration_ms);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FAST_CHECK_PERFORMANCE_PROFILER_H
