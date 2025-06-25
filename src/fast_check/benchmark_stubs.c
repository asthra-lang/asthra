/*
 * Asthra Fast Check Benchmark Stubs
 * Week 16: Performance Optimization & Testing
 *
 * Stub implementations for FastCheckEngine used in benchmarking.
 */

#include "benchmark_internal.h"
#include <stdlib.h>
#include <sys/stat.h>

// =============================================================================
// FastCheckEngine Stub Implementations
// =============================================================================

FastCheckEngine *fast_check_engine_create(void) {
    FastCheckEngine *engine = malloc(sizeof(FastCheckEngine));
    if (engine) {
        engine->initialized = true;
        engine->dummy_field = 42;
    }
    return engine;
}

void fast_check_engine_destroy(FastCheckEngine *engine) {
    if (engine) {
        free(engine);
    }
}

FastCheckResult *fast_check_file(FastCheckEngine *engine, const char *filename) {
    (void)engine; // Suppress unused parameter warning

    FastCheckResult *result = malloc(sizeof(FastCheckResult));
    if (!result)
        return NULL;

    // Simulate processing time based on file size
    struct stat st;
    double simulated_time = 10.0; // Default 10ms

    if (stat(filename, &st) == 0) {
        // Simulate processing time: 1ms per KB + base 5ms
        simulated_time = 5.0 + (st.st_size / 1024.0);
    }

    // Add some random variation
    simulated_time += (rand() % 10) - 5;
    if (simulated_time < 1.0)
        simulated_time = 1.0;

    result->success = true;
    result->duration_ms = simulated_time;
    result->error_count = 0;
    result->warning_count = rand() % 3; // 0-2 warnings

    return result;
}

void fast_check_result_destroy(FastCheckResult *result) {
    if (result) {
        free(result);
    }
}