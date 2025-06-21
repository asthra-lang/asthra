/**
 * Real Program Test Performance Implementation
 * 
 * Contains functions for performance testing, benchmarking, and
 * performance threshold validation.
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_test_performance.h"
#include "real_program_test_generators.h"
#include "parser_string_interface.h"
#include <stdio.h>
#include <stdlib.h>

// =============================================================================
// PERFORMANCE TESTING FUNCTIONS
// =============================================================================

PerformanceMetrics test_large_program_parsing(size_t line_count, const TestSuiteConfig* config) {
    PerformanceMetrics metrics = {0};
    
    // Generate a large program
    char* program = generate_large_program(line_count);
    if (!program) {
        return metrics;
    }
    
    // Test parsing performance
    RealProgramTestResult result = validate_complete_program(program, "large_program", config);
    metrics = result.metrics;
    
    cleanup_test_result(&result);
    free(program);
    
    return metrics;
}

PerformanceMetrics test_deeply_nested_structures(size_t depth, const TestSuiteConfig* config) {
    PerformanceMetrics metrics = {0};
    
    // Generate a deeply nested program
    char* program = generate_deep_nesting_program(depth);
    if (!program) {
        return metrics;
    }
    
    // Test parsing performance
    RealProgramTestResult result = validate_complete_program(program, "nested_program", config);
    metrics = result.metrics;
    
    cleanup_test_result(&result);
    free(program);
    
    return metrics;
}

bool validate_performance_thresholds(const char* program, PerformanceThreshold threshold) {
    if (!program) {
        return false;
    }
    
    TestSuiteConfig config = create_default_config();
    config.verbose_output = false;
    
    RealProgramTestResult result = validate_complete_program(program, "threshold_test", &config);
    
    bool meets_requirements = true;
    
    // Check parsing time
    if (result.execution_time_ms > threshold.max_parse_time_ms) {
        meets_requirements = false;
    }
    
    // Check memory usage (if available)
    if (result.metrics.memory_used_kb > threshold.max_memory_kb) {
        meets_requirements = false;
    }
    
    // Check success requirement
    if (threshold.must_complete_successfully && !result.success) {
        meets_requirements = false;
    }
    
    // Check error detection requirement
    if (threshold.must_detect_errors && result.success) {
        meets_requirements = false;
    }
    
    // Check parsing speed
    if (threshold.min_parse_speed_loc_per_ms > 0.0 && 
        result.metrics.parse_speed_loc_per_ms < threshold.min_parse_speed_loc_per_ms) {
        meets_requirements = false;
    }
    
    cleanup_test_result(&result);
    return meets_requirements;
}

PerformanceMetrics benchmark_parsing_speed(const char* source, size_t iterations, const TestSuiteConfig* config) {
    PerformanceMetrics metrics = {0};
    
    if (!source || iterations == 0) {
        return metrics;
    }
    
    uint64_t total_time = 0;
    size_t successful_iterations = 0;
    
    for (size_t i = 0; i < iterations; i++) {
        uint64_t start = get_timestamp_ms();
        
        ParseResult result = parse_string(source);
        
        uint64_t end = get_timestamp_ms();
        
        if (result.success) {
            total_time += (end - start);
            successful_iterations++;
        }
        
        cleanup_parse_result(&result);
    }
    
    if (successful_iterations > 0) {
        metrics.parse_time_ms = (size_t)(total_time / successful_iterations);
        
        // Count lines
        const char* ptr = source;
        while (*ptr) {
            if (*ptr == '\n') {
                metrics.lines_of_code++;
            }
            ptr++;
        }
        metrics.lines_of_code++;
        
        // Calculate speed
        if (metrics.parse_time_ms > 0) {
            metrics.parse_speed_loc_per_ms = (double)metrics.lines_of_code / metrics.parse_time_ms;
        }
    }
    
    return metrics;
}

bool run_performance_benchmark_suite(const PerformanceBenchmark* benchmarks, 
                                   size_t benchmark_count, 
                                   const TestSuiteConfig* config) {
    if (!benchmarks || benchmark_count == 0 || !config) {
        return false;
    }
    
    if (config->verbose_output) {
        fprintf(config->output_stream, "=== Running Performance Benchmark Suite ===\n");
        fprintf(config->output_stream, "Total benchmarks: %zu\n\n", benchmark_count);
    }
    
    size_t passed = 0;
    size_t required_passed = 0;
    size_t required_total = 0;
    
    for (size_t i = 0; i < benchmark_count; i++) {
        const PerformanceBenchmark* benchmark = &benchmarks[i];
        
        if (config->verbose_output) {
            fprintf(config->output_stream, "Running benchmark %zu/%zu: %s\n", 
                    i + 1, benchmark_count, benchmark->benchmark_name);
        }
        
        PerformanceThreshold threshold = {
            .max_parse_time_ms = benchmark->max_parse_time_ms,
            .max_memory_kb = benchmark->max_memory_kb,
            .must_complete_successfully = true,
            .must_detect_errors = false,
            .min_parse_speed_loc_per_ms = 0.0
        };
        
        bool benchmark_passed = validate_performance_thresholds(benchmark->test_program, threshold);
        
        if (benchmark_passed) {
            passed++;
            if (benchmark->required_to_pass) {
                required_passed++;
            }
            if (config->verbose_output) {
                fprintf(config->output_stream, "  ✅ PASS\n");
            }
        } else {
            if (config->verbose_output) {
                fprintf(config->output_stream, "  ❌ FAIL\n");
            }
        }
        
        if (benchmark->required_to_pass) {
            required_total++;
        }
    }
    
    if (config->verbose_output) {
        fprintf(config->output_stream, "\n=== Benchmark Results ===\n");
        fprintf(config->output_stream, "Total passed: %zu/%zu\n", passed, benchmark_count);
        fprintf(config->output_stream, "Required passed: %zu/%zu\n", required_passed, required_total);
        fprintf(config->output_stream, "Success rate: %.1f%%\n", 
                (double)passed / benchmark_count * 100.0);
        fprintf(config->output_stream, "========================\n\n");
    }
    
    // All required benchmarks must pass
    return required_passed == required_total;
}