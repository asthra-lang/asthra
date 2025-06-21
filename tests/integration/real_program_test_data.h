/**
 * Real Program Test Data Header
 * 
 * Contains predefined test programs and benchmarks for comprehensive
 * real program testing to prevent false positives.
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef REAL_PROGRAM_TEST_DATA_H
#define REAL_PROGRAM_TEST_DATA_H

#include <stddef.h>
#include <stdbool.h>

// =============================================================================
// PREDEFINED TEST PROGRAMS
// =============================================================================

extern const char* BASIC_TYPES_PROGRAMS[];
extern const size_t BASIC_TYPES_PROGRAM_COUNT;

extern const char* FUNCTION_TEST_PROGRAMS[];
extern const size_t FUNCTION_TEST_PROGRAM_COUNT;

extern const char* CONTROL_FLOW_PROGRAMS[];
extern const size_t CONTROL_FLOW_PROGRAM_COUNT;

extern const char* ERROR_DETECTION_PROGRAMS[];
extern const size_t ERROR_DETECTION_PROGRAM_COUNT;

// =============================================================================
// PERFORMANCE BENCHMARKS
// =============================================================================

typedef struct {
    const char* benchmark_name;
    const char* test_program;
    size_t max_parse_time_ms;
    size_t max_memory_kb;
    bool required_to_pass;
    const char* description;
} PerformanceBenchmark;

extern const PerformanceBenchmark REQUIRED_BENCHMARKS[];
extern const size_t REQUIRED_BENCHMARK_COUNT;

#endif // REAL_PROGRAM_TEST_DATA_H