/*
 * Asthra Fast Check Benchmark Reporting
 * Week 16: Performance Optimization & Testing
 *
 * Report generation and formatting functions for benchmark results.
 */

#include "benchmark_internal.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// Report Generation
// =============================================================================

void print_benchmark_header(void) {
    printf("🚀 Asthra Fast Check Performance Benchmark Suite\n");
    printf("================================================\n");
    printf("Week 16: Performance Optimization & Testing\n");
    printf("Phase 5: Fast Check Mode & Incremental Analysis\n\n");

    printf("Performance Targets:\n");
    printf("  • Single File: < 100ms\n");
    printf("  • Medium Project (20 files): < 500ms\n");
    printf("  • Large Project (50 files): < 2000ms\n");
    printf("  • Cache Performance: < 50ms (warm)\n\n");
}

void print_benchmark_report(const BenchmarkReport *report) {
    const char *status_emoji = strcmp(report->status, "PASS") == 0 ||
                                       strcmp(report->status, "EXCELLENT") == 0 ||
                                       strcmp(report->status, "GOOD") == 0
                                   ? "✅"
                               : strcmp(report->status, "SLOW") == 0 ? "⚠️"
                                                                     : "❌";

    printf("%s %s Benchmark\n", status_emoji, report->suite_name);
    printf("   Total Time: %.2f ms (Target: %.0f ms)\n", report->total_time_ms,
           report->target_time_ms);
    printf("   Average Time: %.2f ms\n", report->average_time_ms);
    printf("   Range: %.2f - %.2f ms\n", report->min_time_ms, report->max_time_ms);
    printf("   Files: %d\n", report->file_count);

    if (report->cache_hit_rate > 0) {
        printf("   Cache Performance: %.1f%% efficiency\n", report->cache_hit_rate);
    }

    if (report->peak_memory_mb > 0) {
        printf("   Peak Memory: %zu MB\n", report->peak_memory_mb);
    }

    printf("   Status: %s", report->status);
    if (strcmp(report->status, "EXCELLENT") == 0) {
        printf(" (Outstanding performance!)");
    } else if (strcmp(report->status, "GOOD") == 0) {
        printf(" (Good cache performance)");
    } else if (strcmp(report->status, "SLOW") == 0) {
        printf(" (Meets functionality but exceeds time target)");
    }
    printf("\n\n");
}

void print_benchmark_summary(const BenchmarkReport *reports, int count) {
    int passed = 0;
    int total_files = 0;
    double total_time = 0.0;

    for (int i = 0; i < count; i++) {
        if (strcmp(reports[i].status, "PASS") == 0 || strcmp(reports[i].status, "EXCELLENT") == 0 ||
            strcmp(reports[i].status, "GOOD") == 0) {
            passed++;
        }
        total_files += reports[i].file_count;
        total_time += reports[i].total_time_ms;
    }

    printf("📊 Benchmark Summary\n");
    printf("===================\n");
    printf("Suites Passed: %d/%d (%.1f%%)\n", passed, count, (passed * 100.0) / count);
    printf("Total Files Tested: %d\n", total_files);
    printf("Total Execution Time: %.2f ms\n", total_time);
    printf("Overall Performance: %s\n", passed == count          ? "✅ EXCELLENT"
                                        : passed >= count * 0.75 ? "✅ GOOD"
                                                                 : "⚠️ NEEDS IMPROVEMENT");

    if (passed == count) {
        printf("\n🎉 All performance targets achieved!\n");
        printf("Phase 5 Fast Check Mode implementation is production-ready.\n");
    } else {
        printf("\n💡 Some optimizations may be needed to meet all targets.\n");
    }
}