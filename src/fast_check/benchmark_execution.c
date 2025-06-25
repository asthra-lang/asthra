/*
 * Asthra Fast Check Benchmark Execution
 * Week 16: Performance Optimization & Testing
 *
 * Core benchmark execution logic and utility functions.
 */

#include "benchmark_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// =============================================================================
// Utility Functions
// =============================================================================

double get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

bool create_benchmark_file(const char *filename, const char *template, int index) {
    FILE *file = fopen(filename, "w");
    if (!file)
        return false;

    fprintf(file, template, index);
    fclose(file);
    return true;
}

void cleanup_benchmark_file(const char *filename) {
    unlink(filename);
}

// =============================================================================
// Benchmark Execution
// =============================================================================

BenchmarkReport run_single_benchmark_suite(const BenchmarkSuite *suite) {
    BenchmarkReport report = {0};
    strncpy(report.suite_name, suite->name, sizeof(report.suite_name) - 1);
    report.suite_name[sizeof(report.suite_name) - 1] = '\0';
    report.target_time_ms = suite->target_time_ms;
    report.file_count = suite->file_count;
    report.min_time_ms = 999999.0;
    report.max_time_ms = 0.0;

    FastCheckEngine *engine = fast_check_engine_create();
    if (!engine) {
        strcpy(report.status, "ENGINE_CREATION_FAILED");
        return report;
    }

    PerformanceProfile *profile = performance_profiler_create();
    if (!profile) {
        fast_check_engine_destroy(engine);
        strcpy(report.status, "PROFILER_CREATION_FAILED");
        return report;
    }

    // Create benchmark files
    char filenames[suite->file_count][64];
    const char *template = get_code_template(suite->complexity);

    for (int i = 0; i < suite->file_count; i++) {
        snprintf(filenames[i], sizeof(filenames[i]), "benchmark_%s_%d.asthra", suite->name, i);
        if (!create_benchmark_file(filenames[i], template, i)) {
            strcpy(report.status, "FILE_CREATION_FAILED");
            goto cleanup;
        }
    }

    // Start overall timing and profiling
    performance_timer_start(&profile->overall_timer);
    double benchmark_start = get_current_time_ms();

    // Execute benchmark
    bool all_success = true;
    double total_file_time = 0.0;

    for (int i = 0; i < suite->file_count; i++) {
        performance_record_file_start(profile, filenames[i]);

        double file_start = get_current_time_ms();
        FastCheckResult *result = fast_check_file(engine, filenames[i]);
        double file_time = get_current_time_ms() - file_start;

        performance_record_file_complete(profile, filenames[i], 100, 30, file_time);

        if (!result || !result->success) {
            all_success = false;
        }

        // Track min/max times
        if (file_time < report.min_time_ms)
            report.min_time_ms = file_time;
        if (file_time > report.max_time_ms)
            report.max_time_ms = file_time;

        total_file_time += file_time;

        if (result) {
            fast_check_result_destroy(result);
        }
    }

    double benchmark_end = get_current_time_ms();
    performance_timer_stop(&profile->overall_timer);

    // Calculate results
    report.total_time_ms = benchmark_end - benchmark_start;
    report.average_time_ms = total_file_time / suite->file_count;
    report.meets_target = report.total_time_ms <= suite->target_time_ms;

    // Get cache performance
    if (profile->cache_stats.total_requests > 0) {
        report.cache_hit_rate = profile->cache_stats.hit_rate_percentage;
    }

    // Get memory usage
    report.peak_memory_mb = profile->memory_stats.peak_memory_bytes / (1024 * 1024);

    // Set status
    if (all_success && report.meets_target) {
        strcpy(report.status, "PASS");
    } else if (all_success && !report.meets_target) {
        strcpy(report.status, "SLOW");
    } else {
        strcpy(report.status, "FAIL");
    }

cleanup:
    // Cleanup files
    for (int i = 0; i < suite->file_count; i++) {
        cleanup_benchmark_file(filenames[i]);
    }

    performance_profiler_destroy(profile);
    fast_check_engine_destroy(engine);

    return report;
}

// =============================================================================
// Cache Performance Benchmark
// =============================================================================

BenchmarkReport run_cache_performance_benchmark(void) {
    BenchmarkReport report = {0};
    strncpy(report.suite_name, "cache_performance", sizeof(report.suite_name) - 1);
    report.suite_name[sizeof(report.suite_name) - 1] = '\0';
    report.target_time_ms = 50.0; // Cached operations should be very fast
    report.file_count = 1;
    report.min_time_ms = 999999.0;
    report.max_time_ms = 0.0;

    FastCheckEngine *engine = fast_check_engine_create();
    if (!engine) {
        strcpy(report.status, "ENGINE_CREATION_FAILED");
        return report;
    }

    // Create test file
    const char *test_file = "benchmark_cache_test.asthra";
    const char *template = get_code_template("medium");
    if (!create_benchmark_file(test_file, template, 1)) {
        strcpy(report.status, "FILE_CREATION_FAILED");
        fast_check_engine_destroy(engine);
        return report;
    }

    // First run (cold cache)
    double cold_start = get_current_time_ms();
    FastCheckResult *cold_result = fast_check_file(engine, test_file);
    double cold_time = get_current_time_ms() - cold_start;

    // Multiple warm runs
    double warm_times[5];
    double total_warm_time = 0.0;

    for (int i = 0; i < 5; i++) {
        double warm_start = get_current_time_ms();
        FastCheckResult *warm_result = fast_check_file(engine, test_file);
        warm_times[i] = get_current_time_ms() - warm_start;
        total_warm_time += warm_times[i];

        if (warm_result) {
            fast_check_result_destroy(warm_result);
        }
    }

    double average_warm_time = total_warm_time / 5;
    double cache_speedup = cold_time / average_warm_time;

    // Calculate results
    report.total_time_ms = cold_time + total_warm_time;
    report.average_time_ms = average_warm_time;
    report.min_time_ms = average_warm_time;
    report.max_time_ms = cold_time;
    report.meets_target = average_warm_time <= report.target_time_ms;
    report.cache_hit_rate = ((cache_speedup - 1.0) / cache_speedup) * 100.0;

    // Set status based on cache performance
    if (cache_speedup >= 5.0 && report.meets_target) {
        strcpy(report.status, "EXCELLENT");
    } else if (cache_speedup >= 2.0 && report.meets_target) {
        strcpy(report.status, "GOOD");
    } else if (report.meets_target) {
        strcpy(report.status, "PASS");
    } else {
        strcpy(report.status, "SLOW");
    }

    // Cleanup
    if (cold_result) {
        fast_check_result_destroy(cold_result);
    }
    cleanup_benchmark_file(test_file);
    fast_check_engine_destroy(engine);

    return report;
}