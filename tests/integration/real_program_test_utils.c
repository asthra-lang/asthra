/**
 * Real Program Test Utilities Implementation
 * 
 * Contains utility functions for timestamps, configuration, and common
 * helper functions for the real program test framework.
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_test_utils.h"
#include <sys/time.h>

// =============================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// =============================================================================

uint64_t get_timestamp_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

double calculate_execution_time(uint64_t start_time, uint64_t end_time) {
    return (double)(end_time - start_time);
}

TestSuiteConfig create_default_config(void) {
    TestSuiteConfig config = {
        .verbose_output = true,
        .stop_on_first_failure = false,
        .measure_performance = true,
        .detect_false_positives = true,
        .max_execution_time_ms = 30000,  // 30 seconds
        .max_memory_usage_kb = 102400,   // 100MB
        .output_format = "text",
        .output_stream = stdout,
        .enable_memory_debugging = false
    };
    return config;
}