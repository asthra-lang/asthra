#include "benchmark.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// STATISTICAL ANALYSIS
// =============================================================================

static int compare_uint64(const void *a, const void *b) {
    uint64_t ua = *(const uint64_t *)a;
    uint64_t ub = *(const uint64_t *)b;
    return (ua > ub) - (ua < ub);
}

void asthra_benchmark_calculate_statistics(const uint64_t *durations, size_t count,
                                           AsthraBenchmarkStatistics *stats) {
    if (!durations || count == 0 || !stats) {
        return;
    }

    // Create a sorted copy for median calculation
    uint64_t *sorted = malloc(count * sizeof(uint64_t));
    if (!sorted) {
        return;
    }

    memcpy(sorted, durations, count * sizeof(uint64_t));
    qsort(sorted, count, sizeof(uint64_t), compare_uint64);

    // Basic statistics
    stats->min_ns = sorted[0];
    stats->max_ns = sorted[count - 1];
    stats->iterations = count;

    // Calculate median
    if (count % 2 == 0) {
        stats->median_ns = (sorted[count / 2 - 1] + sorted[count / 2]) / 2;
    } else {
        stats->median_ns = sorted[count / 2];
    }

    // Calculate mean and total
    uint64_t sum = 0;
    for (size_t i = 0; i < count; i++) {
        sum += durations[i];
    }
    stats->total_ns = sum;
    stats->mean_ns = sum / count;

    // Calculate standard deviation
    double variance_sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        double diff = (double)durations[i] - (double)stats->mean_ns;
        variance_sum += diff * diff;
    }
    stats->std_dev_ns = (uint64_t)sqrt(variance_sum / (double)count);

    // Calculate throughput (operations per second)
    if (stats->mean_ns > 0) {
        stats->throughput_ops_per_sec =
            (double)ASTHRA_BENCHMARK_NANOSECONDS_PER_SECOND / (double)stats->mean_ns;
    } else {
        stats->throughput_ops_per_sec = 0.0;
    }

    free(sorted);
}

size_t asthra_benchmark_filter_outliers(uint64_t *durations, size_t count,
                                        double threshold_std_devs) {
    if (!durations || count < 3 || threshold_std_devs <= 0.0) {
        return count;
    }

    // Calculate initial statistics
    AsthraBenchmarkStatistics stats;
    asthra_benchmark_calculate_statistics(durations, count, &stats);

    // Filter outliers
    size_t filtered_count = 0;
    double threshold = (double)stats.std_dev_ns * threshold_std_devs;

    for (size_t i = 0; i < count; i++) {
        double diff = fabs((double)durations[i] - (double)stats.mean_ns);
        if (diff <= threshold) {
            durations[filtered_count++] = durations[i];
        }
    }

    return filtered_count;
}

void asthra_benchmark_confidence_interval(const AsthraBenchmarkStatistics *stats,
                                          double confidence_level, uint64_t *lower_bound,
                                          uint64_t *upper_bound) {
    if (!stats || !lower_bound || !upper_bound || stats->iterations == 0) {
        return;
    }

    // Simple confidence interval using normal approximation
    // For more accurate results, would need t-distribution
    double z_score = 1.96; // 95% confidence level
    if (confidence_level < 0.95) {
        z_score = 1.645; // 90% confidence level
    } else if (confidence_level > 0.99) {
        z_score = 2.576; // 99% confidence level
    }

    double margin_of_error = z_score * (double)stats->std_dev_ns / sqrt((double)stats->iterations);

    *lower_bound = (uint64_t)fmax(0.0, (double)stats->mean_ns - margin_of_error);
    *upper_bound = (uint64_t)((double)stats->mean_ns + margin_of_error);
}
