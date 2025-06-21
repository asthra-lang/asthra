#ifndef BENCHMARK_TEST_FUNCTIONS_H
#define BENCHMARK_TEST_FUNCTIONS_H

#include "benchmark.h"
#include <stdint.h>

// =============================================================================
// BENCHMARK FUNCTION DECLARATIONS
// =============================================================================

/**
 * Simple CPU-bound benchmark function
 */
AsthraBenchmarkStatus benchmark_cpu_intensive(void *context, uint64_t iteration);

/**
 * Memory allocation benchmark function
 */
AsthraBenchmarkStatus benchmark_memory_allocation(void *context, uint64_t iteration);

/**
 * String operations benchmark using Asthra static analysis functions
 */
AsthraBenchmarkStatus benchmark_string_operations(void *context, uint64_t iteration);

/**
 * Buffer operations benchmark function
 */
AsthraBenchmarkStatus benchmark_buffer_operations(void *context, uint64_t iteration);

/**
 * Memory operations benchmark function
 */
AsthraBenchmarkStatus benchmark_memory_operations(void *context, uint64_t iteration);

/**
 * Atomic operations benchmark function (C17 feature)
 */
AsthraBenchmarkStatus benchmark_atomic_operations(void *context, uint64_t iteration);

/**
 * Large memory copy benchmark function (requires setup/teardown)
 */
AsthraBenchmarkStatus benchmark_large_memory_copy(void *context, uint64_t iteration);

#endif // BENCHMARK_TEST_FUNCTIONS_H
