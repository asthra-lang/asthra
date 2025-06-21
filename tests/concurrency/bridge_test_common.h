/**
 * Asthra Enhanced Concurrency Bridge Test Suite - Common Definitions
 * Shared structures, macros, and function declarations for bridge tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef BRIDGE_TEST_COMMON_H
#define BRIDGE_TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "../../runtime/asthra_concurrency_bridge_modular.h"
#include "../../runtime/asthra_ffi_memory.h"

// =============================================================================
// TEST FRAMEWORK
// =============================================================================

typedef struct {
    const char *name;
    bool (*test_func)(void);
    bool passed;
    double duration_ms;
} TestCase;

extern int tests_run;
extern int tests_passed;

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("ASSERTION FAILED: %s at %s:%d\n", message, __FILE__, __LINE__); \
            return false; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        printf("Running %s... ", #test_func); \
        fflush(stdout); \
        struct timespec start, end; \
        clock_gettime(CLOCK_MONOTONIC, &start); \
        bool result = test_func(); \
        clock_gettime(CLOCK_MONOTONIC, &end); \
        double duration = (end.tv_sec - start.tv_sec) * 1000.0 + \
                         (end.tv_nsec - start.tv_nsec) / 1000000.0; \
        tests_run++; \
        if (result) { \
            tests_passed++; \
            printf("PASSED (%.2f ms)\n", duration); \
        } else { \
            printf("FAILED (%.2f ms)\n", duration); \
        } \
    } while(0)

// =============================================================================
// HELPER FUNCTION DECLARATIONS
// =============================================================================

// Task functions for testing
AsthraResult simple_task_function(void *args, size_t args_size);
AsthraResult failing_task_function(void *args, size_t args_size);
AsthraResult long_running_task_function(void *args, size_t args_size);
AsthraResult mutex_increment_task(void *args, size_t args_size);

// Callback functions for testing
AsthraResult callback_test_function(void *data, size_t data_size, void *context);

// Thread worker functions for testing
void* thread_registration_test_worker(void *arg);

// Test data structures
typedef struct {
    AsthraConcurrencyMutex *mutex;
    int shared_counter;
    int num_increments;
} MutexTestData;

// Test function declarations for each module
bool test_bridge_initialization(void);
bool test_bridge_cleanup(void);

bool test_task_spawn_and_completion(void);
bool test_task_failure_handling(void);
bool test_task_timeout(void);
bool test_task_spawn_options(void);

bool test_callback_enqueue_and_process(void);
bool test_callback_priority_ordering(void);
bool test_callback_queue_limits(void);

bool test_thread_registration(void);
bool test_multiple_thread_registration(void);

bool test_mutex_basic_operations(void);
bool test_recursive_mutex(void);

bool test_concurrent_task_synchronization(void);

bool test_statistics_tracking(void);
bool test_state_dump(void);

#endif // BRIDGE_TEST_COMMON_H 
