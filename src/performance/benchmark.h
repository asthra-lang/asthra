#ifndef ASTHRA_BENCHMARK_H
#define ASTHRA_BENCHMARK_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PERFORMANCE BENCHMARKING FRAMEWORK
// =============================================================================

// C17 Feature Detection for Performance Optimization
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
#define ASTHRA_BENCHMARK_HAS_C17 1
#define ASTHRA_BENCHMARK_ALIGNAS(x) _Alignas(x)
#define ASTHRA_BENCHMARK_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#define ASTHRA_BENCHMARK_HAS_C17 0
#define ASTHRA_BENCHMARK_ALIGNAS(x)
#define ASTHRA_BENCHMARK_STATIC_ASSERT(cond, msg)
#endif

// Performance measurement precision
#define ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND 1000000000ULL
#define ASTHRA_BENCHMARK_MICROSECONDS_PER_SECOND 1000000ULL
#define ASTHRA_BENCHMARK_MILLISECONDS_PER_SECOND 1000ULL

// Benchmark configuration constants
#define ASTHRA_BENCHMARK_MAX_NAME_LENGTH 128
#define ASTHRA_BENCHMARK_MAX_ITERATIONS 1000000
#define ASTHRA_BENCHMARK_MIN_ITERATIONS 10
#define ASTHRA_BENCHMARK_DEFAULT_WARMUP_ITERATIONS 100

// Cache line size for alignment optimization
#define ASTHRA_BENCHMARK_CACHE_LINE_SIZE 64

// =============================================================================
// BENCHMARK TYPES AND STRUCTURES
// =============================================================================

// Benchmark timing precision
typedef enum {
    ASTHRA_BENCHMARK_PRECISION_NANOSECONDS,
    ASTHRA_BENCHMARK_PRECISION_MICROSECONDS,
    ASTHRA_BENCHMARK_PRECISION_MILLISECONDS,
    ASTHRA_BENCHMARK_PRECISION_SECONDS
} AsthraBenchmarkPrecision;

// Benchmark execution mode
typedef enum {
    ASTHRA_BENCHMARK_MODE_SINGLE_THREADED,
    ASTHRA_BENCHMARK_MODE_MULTI_THREADED,
    ASTHRA_BENCHMARK_MODE_CONCURRENT,
    ASTHRA_BENCHMARK_MODE_PARALLEL
} AsthraBenchmarkMode;

// Benchmark result status
typedef enum {
    ASTHRA_BENCHMARK_STATUS_SUCCESS,
    ASTHRA_BENCHMARK_STATUS_ERROR,
    ASTHRA_BENCHMARK_STATUS_TIMEOUT,
    ASTHRA_BENCHMARK_STATUS_MEMORY_ERROR,
    ASTHRA_BENCHMARK_STATUS_INVALID_CONFIG
} AsthraBenchmarkStatus;

// High-resolution timing structure
typedef struct {
    uint64_t start_ns;
    uint64_t end_ns;
    uint64_t duration_ns;
    bool is_valid;
} AsthraBenchmarkTimer;

// Statistical measurement data
typedef struct {
    ASTHRA_BENCHMARK_ALIGNAS(ASTHRA_BENCHMARK_CACHE_LINE_SIZE)
    uint64_t min_ns;
    uint64_t max_ns;
    uint64_t mean_ns;
    uint64_t median_ns;
    uint64_t std_dev_ns;
    uint64_t total_ns;
    uint64_t iterations;
    double throughput_ops_per_sec;
} AsthraBenchmarkStatistics;

// Memory usage tracking
typedef struct {
    size_t peak_memory_bytes;
    size_t current_memory_bytes;
    size_t total_allocations;
    size_t total_deallocations;
    size_t allocation_failures;
} AsthraBenchmarkMemoryStats;

// Benchmark configuration
typedef struct {
    char name[ASTHRA_BENCHMARK_MAX_NAME_LENGTH];
    uint64_t iterations;
    uint64_t warmup_iterations;
    AsthraBenchmarkPrecision precision;
    AsthraBenchmarkMode mode;
    uint32_t thread_count;
    bool track_memory;
    bool enable_profiling;
    uint64_t timeout_ms;
} AsthraBenchmarkConfig;

// Benchmark function signature
typedef AsthraBenchmarkStatus (*AsthraBenchmarkFunction)(void *context, uint64_t iteration);

// Benchmark setup/teardown functions
typedef AsthraBenchmarkStatus (*AsthraBenchmarkSetupFunction)(void **context);
typedef void (*AsthraBenchmarkTeardownFunction)(void *context);

// Complete benchmark definition
typedef struct {
    AsthraBenchmarkConfig config;
    AsthraBenchmarkFunction benchmark_func;
    AsthraBenchmarkSetupFunction setup_func;
    AsthraBenchmarkTeardownFunction teardown_func;
    void *user_data;
} AsthraBenchmarkDefinition;

// Benchmark execution result
typedef struct {
    ASTHRA_BENCHMARK_ALIGNAS(ASTHRA_BENCHMARK_CACHE_LINE_SIZE)
    AsthraBenchmarkConfig config;
    AsthraBenchmarkStatistics stats;
    AsthraBenchmarkMemoryStats memory_stats;
    AsthraBenchmarkStatus status;
    char error_message[256];
    uint64_t execution_start_time;
    uint64_t execution_end_time;
} AsthraBenchmarkResult;

// Benchmark suite for multiple benchmarks
typedef struct {
    AsthraBenchmarkDefinition *benchmarks;
    size_t benchmark_count;
    size_t capacity;
    char suite_name[ASTHRA_BENCHMARK_MAX_NAME_LENGTH];
    bool parallel_execution;
} AsthraBenchmarkSuite;

// Thread-safe benchmark context for concurrent execution
typedef struct {
    ASTHRA_BENCHMARK_ALIGNAS(ASTHRA_BENCHMARK_CACHE_LINE_SIZE)
    _Atomic(uint64_t) completed_iterations;
    _Atomic(uint64_t) total_duration_ns;
    _Atomic(uint64_t) min_duration_ns;
    _Atomic(uint64_t) max_duration_ns;
    _Atomic(size_t) memory_usage;
    _Atomic(bool) should_stop;
    uint32_t thread_id;
} AsthraBenchmarkThreadContext;

// =============================================================================
// BENCHMARK EXECUTION FUNCTIONS
// =============================================================================

// Core benchmark execution
AsthraBenchmarkStatus asthra_benchmark_execute(const AsthraBenchmarkDefinition *benchmark,
                                               AsthraBenchmarkResult *result);

// Execute benchmark suite
AsthraBenchmarkStatus asthra_benchmark_execute_suite(const AsthraBenchmarkSuite *suite,
                                                     AsthraBenchmarkResult *results,
                                                     size_t *result_count);

// Single iteration timing
AsthraBenchmarkStatus asthra_benchmark_time_function(AsthraBenchmarkFunction func, void *context,
                                                     uint64_t *duration_ns);

// =============================================================================
// TIMING UTILITIES
// =============================================================================

// High-resolution timer functions
AsthraBenchmarkTimer asthra_benchmark_timer_start(void);
void asthra_benchmark_timer_end(AsthraBenchmarkTimer *timer);
uint64_t asthra_benchmark_timer_duration_ns(const AsthraBenchmarkTimer *timer);

// Time conversion utilities
double asthra_benchmark_ns_to_seconds(uint64_t nanoseconds);
double asthra_benchmark_ns_to_milliseconds(uint64_t nanoseconds);
double asthra_benchmark_ns_to_microseconds(uint64_t nanoseconds);

// System time functions
uint64_t asthra_benchmark_get_time_ns(void);
uint64_t asthra_benchmark_get_cpu_time_ns(void);

// =============================================================================
// STATISTICAL ANALYSIS
// =============================================================================

// Calculate statistics from timing data
void asthra_benchmark_calculate_statistics(const uint64_t *durations, size_t count,
                                           AsthraBenchmarkStatistics *stats);

// Outlier detection and filtering
size_t asthra_benchmark_filter_outliers(uint64_t *durations, size_t count,
                                        double threshold_std_devs);

// Confidence interval calculation
void asthra_benchmark_confidence_interval(const AsthraBenchmarkStatistics *stats,
                                          double confidence_level, uint64_t *lower_bound,
                                          uint64_t *upper_bound);

// =============================================================================
// MEMORY TRACKING
// =============================================================================

// Memory tracking functions
void asthra_benchmark_memory_tracking_start(AsthraBenchmarkMemoryStats *stats);
void asthra_benchmark_memory_tracking_stop(AsthraBenchmarkMemoryStats *stats);
void asthra_benchmark_memory_record_allocation(size_t size);
void asthra_benchmark_memory_record_deallocation(size_t size);

// =============================================================================
// CONFIGURATION AND UTILITIES
// =============================================================================

// Configuration helpers
AsthraBenchmarkConfig asthra_benchmark_config_default(const char *name);
AsthraBenchmarkConfig asthra_benchmark_config_create(const char *name, uint64_t iterations,
                                                     AsthraBenchmarkMode mode);

// Validation functions
bool asthra_benchmark_config_validate(const AsthraBenchmarkConfig *config);
bool asthra_benchmark_definition_validate(const AsthraBenchmarkDefinition *benchmark);

// =============================================================================
// REPORTING AND OUTPUT
// =============================================================================

// Result formatting and output
void asthra_benchmark_print_result(const AsthraBenchmarkResult *result);
void asthra_benchmark_print_suite_results(const AsthraBenchmarkResult *results, size_t count);

// JSON output for automated analysis
AsthraBenchmarkStatus asthra_benchmark_export_json(const AsthraBenchmarkResult *results,
                                                   size_t count, const char *filename);

// CSV output for spreadsheet analysis
AsthraBenchmarkStatus asthra_benchmark_export_csv(const AsthraBenchmarkResult *results,
                                                  size_t count, const char *filename);

// =============================================================================
// SUITE MANAGEMENT
// =============================================================================

// Suite creation and management
AsthraBenchmarkSuite *asthra_benchmark_suite_create(const char *name);
void asthra_benchmark_suite_destroy(AsthraBenchmarkSuite *suite);

// Add benchmarks to suite
AsthraBenchmarkStatus asthra_benchmark_suite_add(AsthraBenchmarkSuite *suite,
                                                 const AsthraBenchmarkDefinition *benchmark);

// Remove benchmarks from suite
AsthraBenchmarkStatus asthra_benchmark_suite_remove(AsthraBenchmarkSuite *suite,
                                                    const char *benchmark_name);

// =============================================================================
// PERFORMANCE MACROS
// =============================================================================

// Convenient macros for common benchmarking patterns
#define ASTHRA_BENCHMARK_SIMPLE(name, func, iterations)                                            \
    do {                                                                                           \
        AsthraBenchmarkConfig config = asthra_benchmark_config_create(                             \
            name, iterations, ASTHRA_BENCHMARK_MODE_SINGLE_THREADED);                              \
        AsthraBenchmarkDefinition def = {.config = config,                                         \
                                         .benchmark_func = func,                                   \
                                         .setup_func = NULL,                                       \
                                         .teardown_func = NULL,                                    \
                                         .user_data = NULL};                                       \
        AsthraBenchmarkResult result;                                                              \
        asthra_benchmark_execute(&def, &result);                                                   \
        asthra_benchmark_print_result(&result);                                                    \
    } while (0)

#define ASTHRA_BENCHMARK_TIME_BLOCK(name)                                                          \
    for (AsthraBenchmarkTimer _timer = asthra_benchmark_timer_start(); _timer.start_ns != 0;       \
         asthra_benchmark_timer_end(&_timer), printf("%s: %lu ns\n", name, _timer.duration_ns),    \
                              _timer.start_ns = 0)

// Compile-time assertions for structure sizes and alignment
ASTHRA_BENCHMARK_STATIC_ASSERT(sizeof(AsthraBenchmarkStatistics) >=
                                   ASTHRA_BENCHMARK_CACHE_LINE_SIZE,
                               "AsthraBenchmarkStatistics should be cache-line aligned");

ASTHRA_BENCHMARK_STATIC_ASSERT(sizeof(AsthraBenchmarkResult) >= ASTHRA_BENCHMARK_CACHE_LINE_SIZE,
                               "AsthraBenchmarkResult should be cache-line aligned");

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_BENCHMARK_H
