/**
 * Asthra Programming Language
 * Test Framework - Statistics Management Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Thread-safe test statistics with atomic operations
 * Enhanced for Testing Framework Standardization Plan Phase 1
 */

#include "test_statistics.h"
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST STATISTICS MANAGEMENT
// =============================================================================

AsthraTestStatistics *asthra_test_statistics_create(void) {
    AsthraTestStatistics *stats = malloc(sizeof(AsthraTestStatistics));
    if (!stats) {
        return NULL;
    }

    // Initialize all fields to zero first
    memset(stats, 0, sizeof(AsthraTestStatistics));

    // Initialize atomic variables directly (ATOMIC_VAR_INIT is deprecated)
    atomic_init(&stats->tests_run, 0);
    atomic_init(&stats->tests_passed, 0);
    atomic_init(&stats->tests_failed, 0);
    atomic_init(&stats->tests_skipped, 0);
    atomic_init(&stats->tests_error, 0);
    atomic_init(&stats->tests_timeout, 0);
    atomic_init(&stats->total_duration_ns, 0);
    atomic_init(&stats->max_duration_ns, 0);
    atomic_init(&stats->min_duration_ns, UINT64_MAX);
    atomic_init(&stats->assertions_checked, 0);
    atomic_init(&stats->assertions_failed, 0);

    // Initialize compatibility fields
    stats->total_tests = 0;
    stats->passed_tests = 0;
    stats->failed_tests = 0;
    stats->skipped_tests = 0;
    stats->error_tests = 0;

    return stats;
}

void asthra_test_statistics_destroy(AsthraTestStatistics *stats) {
    if (stats) {
        free(stats);
    }
}

void asthra_test_statistics_reset(AsthraTestStatistics *stats) {
    if (!stats)
        return;

    asthra_test_set_stat(&stats->tests_run, 0);
    asthra_test_set_stat(&stats->tests_passed, 0);
    asthra_test_set_stat(&stats->tests_failed, 0);
    asthra_test_set_stat(&stats->tests_skipped, 0);
    asthra_test_set_stat(&stats->tests_error, 0);
    asthra_test_set_stat(&stats->tests_timeout, 0);
    asthra_test_set_stat(&stats->total_duration_ns, 0);
    asthra_test_set_stat(&stats->max_duration_ns, 0);
    asthra_test_set_stat(&stats->min_duration_ns, UINT64_MAX);
    asthra_test_set_stat(&stats->assertions_checked, 0);
    asthra_test_set_stat(&stats->assertions_failed, 0);

    // Reset compatibility fields
    stats->total_tests = 0;
    stats->passed_tests = 0;
    stats->failed_tests = 0;
    stats->skipped_tests = 0;
    stats->error_tests = 0;
}

// NEW: Sync compatibility fields with atomic counters (Phase 1)
void asthra_test_statistics_sync_compat_fields(AsthraTestStatistics *stats) {
    if (!stats)
        return;

    // Copy atomic values to compatibility fields for simple access
    stats->total_tests = (size_t)asthra_test_get_stat(&stats->tests_run);
    stats->passed_tests = (size_t)asthra_test_get_stat(&stats->tests_passed);
    stats->failed_tests = (size_t)asthra_test_get_stat(&stats->tests_failed);
    stats->skipped_tests = (size_t)asthra_test_get_stat(&stats->tests_skipped);
    stats->error_tests = (size_t)(asthra_test_get_stat(&stats->tests_error) +
                                  asthra_test_get_stat(&stats->tests_timeout));
}

void asthra_test_statistics_print(const AsthraTestStatistics *stats, bool json_format) {
    if (!stats)
        return;

    uint64_t tests_run = asthra_test_get_stat(&stats->tests_run);
    uint64_t tests_passed = asthra_test_get_stat(&stats->tests_passed);
    uint64_t tests_failed = asthra_test_get_stat(&stats->tests_failed);
    uint64_t tests_skipped = asthra_test_get_stat(&stats->tests_skipped);
    uint64_t tests_error = asthra_test_get_stat(&stats->tests_error);
    uint64_t tests_timeout = asthra_test_get_stat(&stats->tests_timeout);
    uint64_t total_duration_ns = asthra_test_get_stat(&stats->total_duration_ns);
    uint64_t max_duration_ns = asthra_test_get_stat(&stats->max_duration_ns);
    uint64_t min_duration_ns = asthra_test_get_stat(&stats->min_duration_ns);
    uint64_t assertions_checked = asthra_test_get_stat(&stats->assertions_checked);
    uint64_t assertions_failed = asthra_test_get_stat(&stats->assertions_failed);

    if (json_format) {
        printf("{\n");
        printf("  \"test_statistics\": {\n");
        printf("    \"tests_run\": %" PRIu64 ",\n", tests_run);
        printf("    \"tests_passed\": %" PRIu64 ",\n", tests_passed);
        printf("    \"tests_failed\": %" PRIu64 ",\n", tests_failed);
        printf("    \"tests_skipped\": %" PRIu64 ",\n", tests_skipped);
        printf("    \"tests_error\": %" PRIu64 ",\n", tests_error);
        printf("    \"tests_timeout\": %" PRIu64 ",\n", tests_timeout);
        printf("    \"total_duration_ms\": %.3f,\n", asthra_test_ns_to_ms(total_duration_ns));
        printf("    \"max_duration_ms\": %.3f,\n", asthra_test_ns_to_ms(max_duration_ns));
        printf("    \"min_duration_ms\": %.3f,\n",
               asthra_test_ns_to_ms(min_duration_ns == UINT64_MAX ? 0 : min_duration_ns));
        printf("    \"assertions_checked\": %" PRIu64 ",\n", assertions_checked);
        printf("    \"assertions_failed\": %" PRIu64 "\n", assertions_failed);
        printf("  }\n");
        printf("}\n");
    } else {
        printf("\n=== Test Statistics ===\n");
        printf("Tests run:       %" PRIu64 "\n", tests_run);
        printf("Tests passed:    %" PRIu64 "\n", tests_passed);
        printf("Tests failed:    %" PRIu64 "\n", tests_failed);
        printf("Tests skipped:   %" PRIu64 "\n", tests_skipped);
        printf("Tests error:     %" PRIu64 "\n", tests_error);
        printf("Tests timeout:   %" PRIu64 "\n", tests_timeout);
        printf("Total duration:  %.3f ms\n", asthra_test_ns_to_ms(total_duration_ns));
        printf("Max duration:    %.3f ms\n", asthra_test_ns_to_ms(max_duration_ns));
        printf("Min duration:    %.3f ms\n",
               asthra_test_ns_to_ms(min_duration_ns == UINT64_MAX ? 0 : min_duration_ns));
        printf("Assertions:      %" PRIu64 " checked, %" PRIu64 " failed\n", assertions_checked,
               assertions_failed);

        if (tests_run > 0) {
            double pass_rate = (double)tests_passed / (double)tests_run * 100.0;
            printf("Pass rate:       %.1f%%\n", pass_rate);
        }
        printf("========================\n");
    }
}
