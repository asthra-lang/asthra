#ifndef BENCHMARK_TEST_HELPERS_H
#define BENCHMARK_TEST_HELPERS_H

#include "benchmark.h"
#include <stdint.h>

// =============================================================================
// HELPER STRUCTURES
// =============================================================================

typedef struct {
    char *large_buffer;
    size_t buffer_size;
    uint64_t setup_time;
} BenchmarkContext;

// =============================================================================
// SETUP AND TEARDOWN FUNCTION DECLARATIONS
// =============================================================================

/**
 * Setup function for large buffer allocation benchmark
 */
AsthraBenchmarkStatus setup_large_buffer(void **context);

/**
 * Teardown function for large buffer allocation benchmark
 */
void teardown_large_buffer(void *context);

#endif // BENCHMARK_TEST_HELPERS_H
