/**
 * Asthra Programming Language
 * Sanitizer Integration Test Suite - Common Definitions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef TEST_SANITIZER_COMMON_H
#define TEST_SANITIZER_COMMON_H

#include "../framework/test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <limits.h>

// Test configuration
#define BUFFER_SIZE 64
#define THREAD_COUNT 4
#define ITERATIONS 1000

// Global variables for thread safety tests
extern int global_counter;
extern pthread_mutex_t counter_mutex;

// AddressSanitizer test functions
AsthraTestResult test_asan_buffer_overflow_detection(AsthraTestContext *context);
AsthraTestResult test_asan_use_after_free_detection(AsthraTestContext *context);
AsthraTestResult test_asan_memory_leak_detection(AsthraTestContext *context);
AsthraTestResult test_asan_double_free_detection(AsthraTestContext *context);

// UndefinedBehaviorSanitizer test functions
AsthraTestResult test_ubsan_integer_overflow_detection(AsthraTestContext *context);
AsthraTestResult test_ubsan_null_pointer_dereference_detection(AsthraTestContext *context);
AsthraTestResult test_ubsan_array_bounds_detection(AsthraTestContext *context);
AsthraTestResult test_ubsan_division_by_zero_detection(AsthraTestContext *context);

// ThreadSanitizer test functions
AsthraTestResult test_tsan_race_condition_detection(AsthraTestContext *context);
AsthraTestResult test_tsan_data_race_infrastructure(AsthraTestContext *context);

// MemorySanitizer test functions
AsthraTestResult test_msan_uninitialized_memory_detection(AsthraTestContext *context);
AsthraTestResult test_msan_uninitialized_variable_detection(AsthraTestContext *context);

// Integration test functions
AsthraTestResult test_sanitizer_environment_setup(AsthraTestContext *context);
AsthraTestResult test_sanitizer_symbol_availability(AsthraTestContext *context);

// Thread helper functions
void* thread_increment_function(void *arg);
void* thread_unsafe_increment_function(void *arg);

#endif // TEST_SANITIZER_COMMON_H 
