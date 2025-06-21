#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// =============================================================================
// CONFIGURATION FUNCTIONS
// =============================================================================

AsthraBenchmarkConfig asthra_benchmark_config_default(const char *name) {
    AsthraBenchmarkConfig config = {
        .iterations = 1000,
        .warmup_iterations = ASTHRA_BENCHMARK_DEFAULT_WARMUP_ITERATIONS,
        .precision = ASTHRA_BENCHMARK_PRECISION_NANOSECONDS,
        .mode = ASTHRA_BENCHMARK_MODE_SINGLE_THREADED,
        .thread_count = 1,
        .track_memory = false,
        .enable_profiling = false,
        .timeout_ms = 30000 // 30 seconds
    };
    
    if (name) {
        strncpy(config.name, name, ASTHRA_BENCHMARK_MAX_NAME_LENGTH - 1);
        config.name[ASTHRA_BENCHMARK_MAX_NAME_LENGTH - 1] = '\0';
    } else {
        strcpy(config.name, "unnamed_benchmark");
    }
    
    return config;
}

AsthraBenchmarkConfig asthra_benchmark_config_create(
    const char *name,
    uint64_t iterations,
    AsthraBenchmarkMode mode) {
    
    AsthraBenchmarkConfig config = asthra_benchmark_config_default(name);
    config.iterations = iterations;
    config.mode = mode;
    
    // Set reasonable thread count for multi-threaded modes
    if (mode != ASTHRA_BENCHMARK_MODE_SINGLE_THREADED) {
        config.thread_count = (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
        if (config.thread_count == 0) {
            config.thread_count = 4; // Fallback
        }
    }
    
    return config;
}

bool asthra_benchmark_config_validate(const AsthraBenchmarkConfig *config) {
    if (!config) {
        return false;
    }
    
    if (config->iterations < ASTHRA_BENCHMARK_MIN_ITERATIONS ||
        config->iterations > ASTHRA_BENCHMARK_MAX_ITERATIONS) {
        return false;
    }
    
    if (config->mode < ASTHRA_BENCHMARK_MODE_SINGLE_THREADED ||
        config->mode > ASTHRA_BENCHMARK_MODE_PARALLEL) {
        return false;
    }
    
    if (config->thread_count == 0 || config->thread_count > 1024) {
        return false;
    }
    
    if (config->timeout_ms == 0) {
        return false;
    }
    
    return true;
}

bool asthra_benchmark_definition_validate(const AsthraBenchmarkDefinition *benchmark) {
    if (!benchmark || !benchmark->benchmark_func) {
        return false;
    }
    
    return asthra_benchmark_config_validate(&benchmark->config);
} 
