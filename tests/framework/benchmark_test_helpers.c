#include "benchmark_test_helpers.h"
#include "static_analysis.h"
#include <stdlib.h>

// =============================================================================
// SETUP AND TEARDOWN FUNCTION IMPLEMENTATIONS
// =============================================================================

AsthraBenchmarkStatus setup_large_buffer(void **context) {
    BenchmarkContext *ctx = malloc(sizeof(BenchmarkContext));
    if (!ctx) {
        return ASTHRA_BENCHMARK_STATUS_MEMORY_ERROR;
    }

    ctx->buffer_size = 1024 * 1024; // 1MB
    ctx->large_buffer = malloc(ctx->buffer_size);
    if (!ctx->large_buffer) {
        free(ctx);
        return ASTHRA_BENCHMARK_STATUS_MEMORY_ERROR;
    }

    // Initialize buffer with test data
    for (size_t i = 0; i < ctx->buffer_size; i++) {
        ctx->large_buffer[i] = (char)(i & 0xFF);
    }

    ctx->setup_time = asthra_benchmark_get_time_ns();
    *context = ctx;

    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
}

void teardown_large_buffer(void *context) {
    BenchmarkContext *ctx = (BenchmarkContext *)context;
    if (ctx) {
        free(ctx->large_buffer);
        free(ctx);
    }
}
