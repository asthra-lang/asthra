#include "benchmark_test_functions.h"
#include "benchmark_test_helpers.h"
#include "static_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

// =============================================================================
// BENCHMARK FUNCTION IMPLEMENTATIONS
// =============================================================================

// Simple CPU-bound benchmark
AsthraBenchmarkStatus benchmark_cpu_intensive(void *context, uint64_t iteration) {
    (void)context;

    // Simulate CPU-intensive work
    volatile uint64_t sum = 0;
    for (uint64_t i = 0; i < iteration * 1000 + 10000; i++) {
        sum += i * i;
    }

    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
}

// Memory allocation benchmark
AsthraBenchmarkStatus benchmark_memory_allocation(void *context, uint64_t iteration) {
    (void)context;

    size_t size = 1024 + (iteration % 4096);
    void *ptr = malloc(size);
    if (!ptr) {
        return ASTHRA_BENCHMARK_STATUS_MEMORY_ERROR;
    }

    // Touch the memory to ensure it's actually allocated
    memset(ptr, (int)(iteration & 0xFF), size);

    free(ptr);
    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
}

// String operations benchmark using Asthra static analysis functions
AsthraBenchmarkStatus benchmark_string_operations(void *context, uint64_t iteration) {
    (void)context;

    char buffer[256];
    const char *test_string = "Hello, Asthra Benchmark Framework!";

    // Test string copy
    asthra_string_copy(buffer, test_string, sizeof(buffer));

    // Test string length
    size_t len = asthra_string_length(buffer);

    // Test string comparison
    int cmp = asthra_string_compare(buffer, test_string);

    // Use results to prevent optimization
    if (len == 0 || cmp != 0) {
        return ASTHRA_BENCHMARK_STATUS_ERROR;
    }

    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
}

// Buffer operations benchmark
AsthraBenchmarkStatus benchmark_buffer_operations(void *context, uint64_t iteration) {
    (void)context;

    char src[128];
    char dest[256];

    // Create test data
    snprintf(src, sizeof(src), "Test data for iteration %llu", iteration);
    size_t src_size = strlen(src);

    // Test buffer copy
    size_t copied = asthra_buffer_copy(dest, sizeof(dest), src, src_size);
    if (copied != src_size) {
        return ASTHRA_BENCHMARK_STATUS_ERROR;
    }

    // Test buffer equality
    bool equal = asthra_buffer_equal(src, src_size, dest, copied);
    if (!equal) {
        return ASTHRA_BENCHMARK_STATUS_ERROR;
    }

    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
}

// Memory operations benchmark
AsthraBenchmarkStatus benchmark_memory_operations(void *context, uint64_t iteration) {
    (void)context;

    size_t size = 1024;
    char *buffer1 = malloc(size);
    char *buffer2 = malloc(size);

    if (!buffer1 || !buffer2) {
        free(buffer1);
        free(buffer2);
        return ASTHRA_BENCHMARK_STATUS_MEMORY_ERROR;
    }

    // Fill with test data
    memset(buffer1, (int)(iteration & 0xFF), size);

    // Test memory copy
    asthra_memory_copy(buffer2, buffer1, size);

    // Test memory comparison
    int cmp = asthra_memory_compare(buffer1, buffer2, size);

    free(buffer1);
    free(buffer2);

    if (cmp != 0) {
        return ASTHRA_BENCHMARK_STATUS_ERROR;
    }

    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
}

// Atomic operations benchmark (C17 feature)
static _Atomic(uint64_t) g_atomic_counter = 0;

AsthraBenchmarkStatus benchmark_atomic_operations(void *context, uint64_t iteration) {
    (void)context;

    // Test various atomic operations
    atomic_fetch_add(&g_atomic_counter, 1);
    atomic_fetch_sub(&g_atomic_counter, 1);

    uint64_t expected = 0;
    atomic_compare_exchange_strong(&g_atomic_counter, &expected, iteration);

    atomic_store(&g_atomic_counter, 0);

    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
}

AsthraBenchmarkStatus benchmark_large_memory_copy(void *context, uint64_t iteration) {
    BenchmarkContext *ctx = (BenchmarkContext*)context;
    if (!ctx || !ctx->large_buffer) {
        return ASTHRA_BENCHMARK_STATUS_ERROR;
    }

    // Allocate destination buffer
    char *dest = malloc(ctx->buffer_size);
    if (!dest) {
        return ASTHRA_BENCHMARK_STATUS_MEMORY_ERROR;
    }

    // Copy data
    asthra_memory_copy(dest, ctx->large_buffer, ctx->buffer_size);

    // Verify a few bytes to ensure copy worked
    if (dest[0] != ctx->large_buffer[0] ||
        dest[ctx->buffer_size - 1] != ctx->large_buffer[ctx->buffer_size - 1]) {
        free(dest);
        return ASTHRA_BENCHMARK_STATUS_ERROR;
    }

    free(dest);
    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
}
