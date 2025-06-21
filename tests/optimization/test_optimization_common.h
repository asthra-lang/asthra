/**
 * Asthra Programming Language
 * Optimization Tests Common Utilities
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Common utilities, constants, and data structures for optimization test suite
 */

#ifndef ASTHRA_TEST_OPTIMIZATION_COMMON_H
#define ASTHRA_TEST_OPTIMIZATION_COMMON_H

#include "../framework/test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

// Test configuration
#define MAX_COMMAND_LENGTH 2048
#define MAX_OUTPUT_LENGTH 8192
#define TEST_TIMEOUT_SECONDS 300
#define PERFORMANCE_THRESHOLD_PERCENT 5.0

// Test result structure for command execution
typedef struct {
    int exit_code;
    char output[MAX_OUTPUT_LENGTH];
    char error[MAX_OUTPUT_LENGTH];
    double execution_time_ms;
} CommandResult;

// Performance benchmark result
typedef struct {
    double baseline_time_ms;
    double optimized_time_ms;
    double improvement_percent;
    bool meets_threshold;
} PerformanceBenchmark;

// Function declarations
CommandResult execute_command(const char *command, int timeout_seconds);
bool file_exists(const char *path);
bool directory_exists(const char *path);
PerformanceBenchmark benchmark_performance(const char *baseline_binary, const char *optimized_binary, const char *test_input);
bool validate_binary_optimization(const char *binary_path);
bool check_sanitizer_reports(const char *report_dir);

#endif // ASTHRA_TEST_OPTIMIZATION_COMMON_H 
