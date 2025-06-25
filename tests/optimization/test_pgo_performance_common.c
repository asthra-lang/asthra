/**
 * Asthra Programming Language
 * Profile-Guided Optimization (PGO) Performance Testing Common Utilities Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_pgo_performance_common.h"

// =============================================================================
// TIMING UTILITIES
// =============================================================================

double pgo_get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

// =============================================================================
// STATISTICAL ANALYSIS
// =============================================================================

PGOPerformanceStats pgo_calculate_stats(double *samples, size_t count) {
    PGOPerformanceStats stats = {0};

    if (count == 0) {
        return stats;
    }

    stats.sample_count = count;

    // Find min, max, and calculate sum
    stats.min_time_ms = samples[0];
    stats.max_time_ms = samples[0];
    double sum = 0.0;

    for (size_t i = 0; i < count; i++) {
        if (samples[i] < stats.min_time_ms) {
            stats.min_time_ms = samples[i];
        }
        if (samples[i] > stats.max_time_ms) {
            stats.max_time_ms = samples[i];
        }
        sum += samples[i];
    }

    // Calculate average
    stats.avg_time_ms = sum / count;

    // Calculate standard deviation
    double variance_sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        double diff = samples[i] - stats.avg_time_ms;
        variance_sum += diff * diff;
    }
    stats.std_dev_ms = sqrt(variance_sum / count);

    // Calculate median (simple approach - sort and take middle)
    double *sorted = malloc(count * sizeof(double));
    if (sorted) {
        memcpy(sorted, samples, count * sizeof(double));

        // Simple bubble sort for small arrays
        for (size_t i = 0; i < count - 1; i++) {
            for (size_t j = 0; j < count - i - 1; j++) {
                if (sorted[j] > sorted[j + 1]) {
                    double temp = sorted[j];
                    sorted[j] = sorted[j + 1];
                    sorted[j + 1] = temp;
                }
            }
        }

        if (count % 2 == 0) {
            stats.median_time_ms = (sorted[count / 2 - 1] + sorted[count / 2]) / 2.0;
        } else {
            stats.median_time_ms = sorted[count / 2];
        }

        free(sorted);
    } else {
        stats.median_time_ms = stats.avg_time_ms;
    }

    return stats;
}

PGOBenchmarkComparison pgo_compare_performance(PGOPerformanceStats baseline,
                                               PGOPerformanceStats optimized) {
    PGOBenchmarkComparison comparison = {0};
    comparison.baseline = baseline;
    comparison.optimized = optimized;

    if (baseline.avg_time_ms > 0) {
        comparison.improvement_percent =
            ((baseline.avg_time_ms - optimized.avg_time_ms) / baseline.avg_time_ms) * 100.0;

        // Simple statistical significance test (t-test approximation)
        double pooled_std = sqrt((baseline.std_dev_ms * baseline.std_dev_ms +
                                  optimized.std_dev_ms * optimized.std_dev_ms) /
                                 2.0);

        if (pooled_std > 0) {
            double t_stat = fabs(baseline.avg_time_ms - optimized.avg_time_ms) /
                            (pooled_std * sqrt(2.0 / baseline.sample_count));

            // Rough confidence level based on t-statistic
            comparison.confidence_level = (t_stat > 2.0) ? 0.95 : (t_stat > 1.5) ? 0.80 : 0.50;
            comparison.statistically_significant = (t_stat > 1.5);
        }

        comparison.meets_threshold =
            comparison.improvement_percent >= PGO_PERFORMANCE_THRESHOLD_PERCENT;
    }

    return comparison;
}

// =============================================================================
// BENCHMARK EXECUTION
// =============================================================================

bool pgo_run_binary_benchmark(const char *binary_path, const char *args,
                              PGOPerformanceStats *stats) {
    if (!binary_path || !stats) {
        return false;
    }

    // Check if binary exists
    if (!file_exists(binary_path)) {
        printf("Binary not found: %s\n", binary_path);
        return false;
    }

    double samples[PGO_BENCHMARK_ITERATIONS];
    size_t successful_runs = 0;

    printf("Benchmarking %s...\n", binary_path);

    for (int i = 0; i < PGO_BENCHMARK_ITERATIONS && successful_runs < PGO_BENCHMARK_ITERATIONS;
         i++) {
        char command[PGO_MAX_COMMAND_LENGTH];
        snprintf(command, sizeof(command), "%s %s > /dev/null 2>&1", binary_path, args ? args : "");

        double start_time = pgo_get_time_ms();
        int exit_code = system(command);
        double end_time = pgo_get_time_ms();

        if (exit_code == 0) {
            samples[successful_runs] = end_time - start_time;
            successful_runs++;
            printf("  Run %zu: %.2f ms\n", successful_runs, samples[successful_runs - 1]);
        } else {
            printf("  Run %d failed (exit code: %d)\n", i + 1, exit_code);
        }

        // Small delay between runs
        usleep(10000); // 10ms
    }

    if (successful_runs < 3) {
        printf("Insufficient successful runs (%zu) for reliable statistics\n", successful_runs);
        return false;
    }

    *stats = pgo_calculate_stats(samples, successful_runs);
    return true;
}

// =============================================================================
// TEST PROGRAM GENERATION
// =============================================================================

bool pgo_create_test_program(const char *filename, PGOBenchmarkType program_type) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        return false;
    }

    switch (program_type) {
    case PGO_BENCHMARK_FIBONACCI:
        fprintf(file, "// Fibonacci benchmark program\n");
        fprintf(file, "fn fibonacci(n: i32) -> i32 {\n");
        fprintf(file, "    if n <= 1 {\n");
        fprintf(file, "        return n;\n");
        fprintf(file, "    }\n");
        fprintf(file, "    return fibonacci(n - 1) + fibonacci(n - 2);\n");
        fprintf(file, "}\n\n");
        fprintf(file, "fn main(void) {\n");
        fprintf(file, "    let result = fibonacci(35);\n");
        fprintf(file, "    print(\"Fibonacci(35) = \", result);\n");
        fprintf(file, "}\n");
        break;

    case PGO_BENCHMARK_MATRIX:
        fprintf(file, "// Matrix multiplication benchmark\n");
        fprintf(file, "fn matrix_multiply(size: i32) -> i32 {\n");
        fprintf(file, "    let mut sum = 0;\n");
        fprintf(file, "    for i in 0..size {\n");
        fprintf(file, "        for j in 0..size {\n");
        fprintf(file, "            for k in 0..size {\n");
        fprintf(file, "                sum = sum + i * j * k;\n");
        fprintf(file, "            }\n");
        fprintf(file, "        }\n");
        fprintf(file, "    }\n");
        fprintf(file, "    return sum;\n");
        fprintf(file, "}\n\n");
        fprintf(file, "fn main(void) {\n");
        fprintf(file, "    let result = matrix_multiply(100);\n");
        fprintf(file, "    print(\"Matrix result: \", result);\n");
        fprintf(file, "}\n");
        break;

    case PGO_BENCHMARK_SORTING:
        fprintf(file, "// Sorting benchmark program\n");
        fprintf(file, "fn bubble_sort(arr: [i32; 1000]) -> [i32; 1000] {\n");
        fprintf(file, "    let mut result = arr;\n");
        fprintf(file, "    for i in 0..1000 {\n");
        fprintf(file, "        for j in 0..(1000 - i - 1) {\n");
        fprintf(file, "            if result[j] > result[j + 1] {\n");
        fprintf(file, "                let temp = result[j];\n");
        fprintf(file, "                result[j] = result[j + 1];\n");
        fprintf(file, "                result[j + 1] = temp;\n");
        fprintf(file, "            }\n");
        fprintf(file, "        }\n");
        fprintf(file, "    }\n");
        fprintf(file, "    return result;\n");
        fprintf(file, "}\n\n");
        fprintf(file, "fn main(void) {\n");
        fprintf(file, "    let mut arr: [i32; 1000];\n");
        fprintf(file, "    for i in 0..1000 {\n");
        fprintf(file, "        arr[i] = 1000 - i;\n");
        fprintf(file, "    }\n");
        fprintf(file, "    let sorted = bubble_sort(arr);\n");
        fprintf(file, "    print(\"Sorted first element: \", sorted[0]);\n");
        fprintf(file, "}\n");
        break;

    case PGO_BENCHMARK_SIMPLE:
    default:
        fprintf(file, "// Simple benchmark program\n");
        fprintf(file, "fn compute(n: i32) -> i32 {\n");
        fprintf(file, "    let mut result = 0;\n");
        fprintf(file, "    for i in 0..n {\n");
        fprintf(file, "        result = result + i * i;\n");
        fprintf(file, "    }\n");
        fprintf(file, "    return result;\n");
        fprintf(file, "}\n\n");
        fprintf(file, "fn main(void) {\n");
        fprintf(file, "    let result = compute(10000);\n");
        fprintf(file, "    print(\"Result: \", result);\n");
        fprintf(file, "}\n");
        break;
    }

    fclose(file);
    return true;
}

// =============================================================================
// BINARY VALIDATION
// =============================================================================

bool pgo_validate_binaries(const char *baseline_binary, const char *optimized_binary) {
    if (!file_exists(baseline_binary)) {
        printf("Baseline binary not found: %s\n", baseline_binary);
        return false;
    }

    if (!file_exists(optimized_binary)) {
        printf("Optimized binary not found: %s\n", optimized_binary);
        return false;
    }

    return true;
}

// =============================================================================
// PERFORMANCE REPORTING
// =============================================================================

void pgo_print_performance_stats(const char *label, const PGOPerformanceStats *stats) {
    printf("%s Performance:\n", label);
    printf("  Average: %.2f ms (±%.2f ms)\n", stats->avg_time_ms, stats->std_dev_ms);
    printf("  Range: %.2f - %.2f ms\n", stats->min_time_ms, stats->max_time_ms);
    printf("  Median: %.2f ms\n", stats->median_time_ms);
    printf("  Samples: %zu\n", stats->sample_count);
}

void pgo_print_benchmark_comparison(const char *benchmark_name,
                                    const PGOBenchmarkComparison *comparison) {
    printf("\n%s Benchmark Results:\n", benchmark_name);

    pgo_print_performance_stats("Baseline", &comparison->baseline);
    pgo_print_performance_stats("PGO Optimized", &comparison->optimized);

    printf("Performance Comparison:\n");
    printf("  Improvement: %.2f%%\n", comparison->improvement_percent);
    printf("  Confidence: %.0f%%\n", comparison->confidence_level * 100);
    printf("  Statistically Significant: %s\n",
           comparison->statistically_significant ? "Yes" : "No");
    printf("  Meets Threshold (%.1f%%): %s\n", PGO_PERFORMANCE_THRESHOLD_PERCENT,
           comparison->meets_threshold ? "Yes" : "No");

    if (comparison->meets_threshold) {
        printf("✓ PGO provides significant performance improvement\n");
    } else if (comparison->improvement_percent > 0) {
        printf("⚠ PGO provides modest improvement (%.2f%%) - acceptable\n",
               comparison->improvement_percent);
    } else {
        printf("⚠ No measurable PGO improvement - may indicate optimization limitations\n");
    }
}
