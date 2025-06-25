/**
 * Asthra Programming Language
 * Gperf Integration Test Common Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_TEST_GPERF_COMMON_H
#define ASTHRA_TEST_GPERF_COMMON_H

#include "../framework/test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// Test configuration constants
#define MAX_KEYWORDS 256
#define MAX_KEYWORD_LENGTH 64
#define HASH_PERFORMANCE_ITERATIONS 100000
#define MAX_COMMAND_LENGTH 1024

// Keyword test structure
typedef struct {
    char keyword[MAX_KEYWORD_LENGTH];
    int expected_token_type;
    bool is_reserved;
} KeywordTest;

// Hash function performance metrics
typedef struct {
    double avg_lookup_time_ns;
    double total_time_ms;
    size_t collisions;
    size_t successful_lookups;
    double efficiency_score;
} HashPerformanceMetrics;

// Sample Asthra keywords for testing
extern const KeywordTest test_keywords[];
extern const size_t test_keywords_count;

// Common utility functions
bool gperf_file_exists(const char *filepath);
bool check_file_substantial(const char *filepath, size_t min_size);
int run_command_with_timeout(const char *command, double timeout_seconds);

#endif // ASTHRA_TEST_GPERF_COMMON_H
