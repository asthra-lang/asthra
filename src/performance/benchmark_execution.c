#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// =============================================================================
// CORE BENCHMARK EXECUTION
// =============================================================================

static AsthraBenchmarkStatus execute_single_threaded_benchmark(
    const AsthraBenchmarkDefinition *benchmark,
    AsthraBenchmarkResult *result) {
    
    const AsthraBenchmarkConfig *config = &benchmark->config;
    
    // Allocate timing data
    uint64_t *durations = malloc(config->iterations * sizeof(uint64_t));
    if (!durations) {
        return ASTHRA_BENCHMARK_STATUS_MEMORY_ERROR;
    }
    
    // Setup benchmark context
    void *context = benchmark->user_data;
    if (benchmark->setup_func) {
        AsthraBenchmarkStatus setup_status = benchmark->setup_func(&context);
        if (setup_status != ASTHRA_BENCHMARK_STATUS_SUCCESS) {
            free(durations);
            return setup_status;
        }
    }
    
    // Start memory tracking if enabled
    if (config->track_memory) {
        asthra_benchmark_memory_tracking_start(&result->memory_stats);
    }
    
    result->execution_start_time = asthra_benchmark_get_time_ns();
    
    // Execute warmup iterations
    for (uint64_t i = 0; i < config->warmup_iterations; i++) {
        AsthraBenchmarkStatus status = benchmark->benchmark_func(context, i);
        if (status != ASTHRA_BENCHMARK_STATUS_SUCCESS) {
            free(durations);
            return status;
        }
    }
    
    // Execute measured iterations
    for (uint64_t i = 0; i < config->iterations; i++) {
        AsthraBenchmarkTimer timer = asthra_benchmark_timer_start();
        
        AsthraBenchmarkStatus status = benchmark->benchmark_func(context, i);
        
        asthra_benchmark_timer_end(&timer);
        durations[i] = timer.duration_ns;
        
        if (status != ASTHRA_BENCHMARK_STATUS_SUCCESS) {
            free(durations);
            return status;
        }
        
        // Check for timeout
        uint64_t elapsed_ms = (asthra_benchmark_get_time_ns() - result->execution_start_time) / 1000000;
        if (elapsed_ms > config->timeout_ms) {
            free(durations);
            return ASTHRA_BENCHMARK_STATUS_TIMEOUT;
        }
    }
    
    result->execution_end_time = asthra_benchmark_get_time_ns();
    
    // Stop memory tracking
    if (config->track_memory) {
        asthra_benchmark_memory_tracking_stop(&result->memory_stats);
    }
    
    // Calculate statistics
    asthra_benchmark_calculate_statistics(durations, config->iterations, &result->stats);
    
    // Cleanup
    if (benchmark->teardown_func) {
        benchmark->teardown_func(context);
    }
    
    free(durations);
    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
}

AsthraBenchmarkStatus asthra_benchmark_execute(
    const AsthraBenchmarkDefinition *benchmark,
    AsthraBenchmarkResult *result) {
    
    if (!benchmark || !result) {
        return ASTHRA_BENCHMARK_STATUS_INVALID_CONFIG;
    }
    
    if (!asthra_benchmark_definition_validate(benchmark)) {
        return ASTHRA_BENCHMARK_STATUS_INVALID_CONFIG;
    }
    
    // Initialize result
    memset(result, 0, sizeof(AsthraBenchmarkResult));
    result->config = benchmark->config;
    result->status = ASTHRA_BENCHMARK_STATUS_SUCCESS;
    
    // Execute based on mode
    switch (benchmark->config.mode) {
        case ASTHRA_BENCHMARK_MODE_SINGLE_THREADED:
            result->status = execute_single_threaded_benchmark(benchmark, result);
            break;
            
        case ASTHRA_BENCHMARK_MODE_MULTI_THREADED:
        case ASTHRA_BENCHMARK_MODE_CONCURRENT:
        case ASTHRA_BENCHMARK_MODE_PARALLEL:
            // TODO: Implement multi-threaded execution
            result->status = ASTHRA_BENCHMARK_STATUS_ERROR;
            strcpy(result->error_message, "Multi-threaded benchmarking not yet implemented");
            break;
            
        default:
            result->status = ASTHRA_BENCHMARK_STATUS_INVALID_CONFIG;
            strcpy(result->error_message, "Invalid benchmark mode");
            break;
    }
    
    return result->status;
}

AsthraBenchmarkStatus asthra_benchmark_time_function(
    AsthraBenchmarkFunction func,
    void *context,
    uint64_t *duration_ns) {
    
    if (!func || !duration_ns) {
        return ASTHRA_BENCHMARK_STATUS_INVALID_CONFIG;
    }
    
    AsthraBenchmarkTimer timer = asthra_benchmark_timer_start();
    AsthraBenchmarkStatus status = func(context, 0);
    asthra_benchmark_timer_end(&timer);
    
    *duration_ns = timer.duration_ns;
    return status;
} 
