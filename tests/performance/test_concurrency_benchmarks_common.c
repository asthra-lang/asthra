/*
 * Concurrency Performance Benchmarks - Common Utilities Implementation
 * Shared implementations for performance testing infrastructure
 *
 * Phase 8: Testing and Validation
 * Focus: Performance validation, scalability, and tier comparison
 */

#include "test_concurrency_benchmarks_common.h"

// ============================================================================
// Global Test Counters
// ============================================================================

size_t tests_run = 0;
size_t tests_passed = 0;

// ============================================================================
// Timing Utilities Implementation
// ============================================================================

void timer_start(Timer *timer) {
    gettimeofday(&timer->start, NULL);
}

double timer_stop(Timer *timer) {
    gettimeofday(&timer->end, NULL);
    return (timer->end.tv_sec - timer->start.tv_sec) * 1000.0 +
           (timer->end.tv_usec - timer->start.tv_usec) / 1000.0;
}

// ============================================================================
// Runtime Management
// ============================================================================

void initialize_benchmark_runtime(void) {
    printf("=== Asthra Concurrency Performance Benchmarks (Phase 8) ===\n");
    printf("Testing performance of three-tier concurrency system:\n");
    printf("- Channel throughput, latency, and scalability\n");
    printf("- Select, barrier, and semaphore performance\n");
    printf("- Memory efficiency and tier comparisons\n");
    printf("- Stress testing and bottleneck identification\n\n");

    // Initialize concurrency runtime
    AsthraConcurrencyResult init_result = asthra_concurrency_initialize();
    if (!asthra_concurrency_result_is_success(init_result)) {
        printf("❌ Failed to initialize concurrency runtime\n");
        exit(1);
    }
}

void cleanup_benchmark_runtime(void) {
    // Cleanup concurrency runtime
    asthra_concurrency_cleanup();
}

void print_benchmark_results(void) {
    // Results
    printf("\n=== Concurrency Performance Benchmark Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", (float)tests_passed / tests_run * 100.0);

    if (tests_passed == tests_run) {
        printf("✅ All performance benchmarks passed!\n");
        printf("Concurrency system meets performance requirements.\n");
    } else {
        printf("❌ Some performance benchmarks failed. Please review the output above.\n");
    }
}
