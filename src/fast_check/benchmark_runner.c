/*
 * Asthra Fast Check Benchmark Runner
 * Week 16: Performance Optimization & Testing
 *
 * Main benchmark runner and coordination logic.
 */

#include "benchmark_internal.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// Main Benchmark Function
// =============================================================================

int run_performance_benchmarks(void) {
    print_benchmark_header();

    const BenchmarkSuite *suites = get_benchmark_suites();
    int suite_count = get_benchmark_suite_count();

    BenchmarkReport reports[suite_count + 1]; // +1 for cache benchmark
    int report_count = 0;

    // Run standard benchmark suites
    for (int i = 0; i < suite_count - 1; i++) { // -1 to skip cache_performance (handled separately)
        printf("Running %s benchmark...\n", suites[i].name);
        reports[report_count] = run_single_benchmark_suite(&suites[i]);
        print_benchmark_report(&reports[report_count]);
        report_count++;
    }

    // Run cache performance benchmark separately
    printf("Running cache performance benchmark...\n");
    reports[report_count] = run_cache_performance_benchmark();
    print_benchmark_report(&reports[report_count]);
    report_count++;

    // Print summary
    print_benchmark_summary(reports, report_count);

    // Return success if all benchmarks passed
    int passed = 0;
    for (int i = 0; i < report_count; i++) {
        if (strcmp(reports[i].status, "PASS") == 0 || strcmp(reports[i].status, "EXCELLENT") == 0 ||
            strcmp(reports[i].status, "GOOD") == 0) {
            passed++;
        }
    }

    return passed == report_count ? 0 : 1;
}

// Main function removed - this is now a library component
// To run benchmarks, use the test or create a separate executable