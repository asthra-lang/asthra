/**
 * Asthra Programming Language
 * Optimization Tests Common Utilities Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_optimization_common.h"

CommandResult execute_command(const char *command, int timeout_seconds) {
    CommandResult result = {0};
    char temp_stdout[] = "/tmp/asthra_test_stdout_XXXXXX";
    char temp_stderr[] = "/tmp/asthra_test_stderr_XXXXXX";
    
    int stdout_fd = mkstemp(temp_stdout);
    int stderr_fd = mkstemp(temp_stderr);
    
    if (stdout_fd == -1 || stderr_fd == -1) {
        result.exit_code = -1;
        snprintf(result.error, sizeof(result.error), "Failed to create temp files");
        return result;
    }
    
    close(stdout_fd);
    close(stderr_fd);
    
    // Build command with output redirection - use cross-platform timeout approach
    char full_command[MAX_COMMAND_LENGTH];
    
#ifdef __APPLE__
    // macOS doesn't have timeout command by default, use gtimeout if available or skip timeout
    int which_result = system("which gtimeout > /dev/null 2>&1");
    if (which_result != -1 && WIFEXITED(which_result) && WEXITSTATUS(which_result) == 0) {
        snprintf(full_command, sizeof(full_command), 
                 "gtimeout %d %s > %s 2> %s", 
                 timeout_seconds, command, temp_stdout, temp_stderr);
    } else {
        // Fallback: run without timeout on macOS
        snprintf(full_command, sizeof(full_command), 
                 "%s > %s 2> %s", 
                 command, temp_stdout, temp_stderr);
    }
#else
    // Linux and other Unix systems - use standard timeout command
    snprintf(full_command, sizeof(full_command), 
             "timeout %d %s > %s 2> %s", 
             timeout_seconds, command, temp_stdout, temp_stderr);
#endif
    
    clock_t start = clock();
    int sys_result = system(full_command);
    if (sys_result == -1) {
        result.exit_code = -1;
    } else {
        result.exit_code = WIFEXITED(sys_result) ? WEXITSTATUS(sys_result) : -1;
    }
    clock_t end = clock();
    
    result.execution_time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
    
    // Read stdout
    FILE *stdout_file = fopen(temp_stdout, "r");
    if (stdout_file) {
        size_t bytes_read = fread(result.output, 1, sizeof(result.output) - 1, stdout_file);
        result.output[bytes_read] = '\0';
        fclose(stdout_file);
    }
    
    // Read stderr
    FILE *stderr_file = fopen(temp_stderr, "r");
    if (stderr_file) {
        size_t bytes_read = fread(result.error, 1, sizeof(result.error) - 1, stderr_file);
        result.error[bytes_read] = '\0';
        fclose(stderr_file);
    }
    
    // Cleanup temp files
    unlink(temp_stdout);
    unlink(temp_stderr);
    
    return result;
}

bool file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

bool directory_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

PerformanceBenchmark benchmark_performance(const char *baseline_binary, const char *optimized_binary, const char *test_input) {
    PerformanceBenchmark benchmark = {0};
    
    // Test baseline binary
    char baseline_command[MAX_COMMAND_LENGTH];
    snprintf(baseline_command, sizeof(baseline_command), "time %s %s", baseline_binary, test_input);
    
    CommandResult baseline_result = execute_command(baseline_command, 60);
    benchmark.baseline_time_ms = baseline_result.execution_time_ms;
    
    // Test optimized binary
    char optimized_command[MAX_COMMAND_LENGTH];
    snprintf(optimized_command, sizeof(optimized_command), "time %s %s", optimized_binary, test_input);
    
    CommandResult optimized_result = execute_command(optimized_command, 60);
    benchmark.optimized_time_ms = optimized_result.execution_time_ms;
    
    // Calculate improvement
    if (benchmark.baseline_time_ms > 0) {
        benchmark.improvement_percent = 
            ((benchmark.baseline_time_ms - benchmark.optimized_time_ms) / benchmark.baseline_time_ms) * 100.0;
        benchmark.meets_threshold = benchmark.improvement_percent >= PERFORMANCE_THRESHOLD_PERCENT;
    }
    
    return benchmark;
}

bool validate_binary_optimization(const char *binary_path) {
    if (!file_exists(binary_path)) {
        return false;
    }
    
    // Check if binary has optimization symbols/metadata
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command), "file %s", binary_path);
    
    CommandResult result = execute_command(command, 10);
    
    // Look for optimization indicators in file output
    return (result.exit_code == 0 && 
            (strstr(result.output, "not stripped") != NULL ||
             strstr(result.output, "executable") != NULL));
}

bool check_sanitizer_reports(const char *report_dir) {
    if (!directory_exists(report_dir)) {
        return false;
    }
    
    // Check for sanitizer report files
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command), "find %s -name '*.log' -type f | wc -l", report_dir);
    
    CommandResult result = execute_command(command, 10);
    
    if (result.exit_code == 0) {
        int report_count = atoi(result.output);
        return report_count > 0;
    }
    
    return false;
} 
