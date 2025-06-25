/*
 * Stdlib Concurrent Coordination Tests - Common Definitions
 * Shared types, utilities, and framework for coordination primitive tests
 *
 * Phase 8: Testing and Validation
 * Focus: Select operations, barriers, semaphores, and coordination primitives
 */

#ifndef TEST_CONCURRENT_COORDINATION_COMMON_H
#define TEST_CONCURRENT_COORDINATION_COMMON_H

#include "../../runtime/stdlib_concurrency_support.h"
#include "../framework/test_framework.h"
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// ============================================================================
// Test Framework Macros
// ============================================================================

extern size_t tests_run;
extern size_t tests_passed;

#define ASSERT_TRUE(condition)                                                                     \
    do {                                                                                           \
        tests_run++;                                                                               \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("  ✅ PASS: %s\n", #condition);                                                 \
        } else {                                                                                   \
            printf("  ❌ FAIL: %s:%d - %s\n", __FILE__, __LINE__, #condition);                     \
        }                                                                                          \
    } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_EQUAL(a, b) ASSERT_TRUE((a) == (b))

// ============================================================================
// Barrier Test Data Structures
// ============================================================================

typedef struct {
    AsthraConcurrencyBarrierHandle *barrier;
    int thread_id;
    int *execution_order;
    int *next_position;
    pthread_mutex_t *order_mutex;
} BarrierTestData;

// ============================================================================
// Semaphore Test Data Structures
// ============================================================================

typedef struct {
    AsthraConcurrencySemaphoreHandle *semaphore;
    int thread_id;
    int work_duration_ms;
    int *completion_order;
    int *next_position;
    pthread_mutex_t *order_mutex;
} SemaphoreTestData;

// ============================================================================
// Thread Functions
// ============================================================================

/**
 * Thread function for barrier synchronization tests
 * @param arg Pointer to BarrierTestData structure
 * @return Thread result (leader status or error code)
 */
void *barrier_thread(void *arg);

/**
 * Thread function for semaphore worker tests
 * @param arg Pointer to SemaphoreTestData structure
 * @return Thread result (success or error code)
 */
void *semaphore_worker_thread(void *arg);

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Initialize test framework counters
 */
void init_test_framework(void);

/**
 * Print test results summary
 * @param category Test category name
 */
void print_test_results(const char *category);

/**
 * Get the current number of tests run
 * @return Current tests run count
 */
size_t get_tests_run(void);

/**
 * Get the current number of tests passed
 * @return Current tests passed count
 */
size_t get_tests_passed(void);

/**
 * Print detailed test summary
 * @param test_name Name of the test
 * @param local_tests_run Number of local tests run
 * @param local_tests_passed Number of local tests passed
 */
void print_test_summary(const char *test_name, size_t local_tests_run, size_t local_tests_passed);

/**
 * Validate timing with tolerance
 * @param elapsed_ms Actual elapsed time in milliseconds
 * @param expected_ms Expected time in milliseconds
 * @param tolerance_factor Tolerance factor (e.g., 0.8 for 80% tolerance)
 * @return true if timing is within tolerance
 */
bool validate_timing(double elapsed_ms, uint64_t expected_ms, double tolerance_factor);

/**
 * Create and initialize barrier test data array
 * @param num_threads Number of threads
 * @param barrier Barrier handle
 * @param execution_order Array to store execution order
 * @param next_position Pointer to next position counter
 * @param order_mutex Mutex for order tracking
 * @return Allocated and initialized test data array
 */
BarrierTestData *create_barrier_test_data(int num_threads, AsthraConcurrencyBarrierHandle *barrier,
                                          int *execution_order, int *next_position,
                                          pthread_mutex_t *order_mutex);

/**
 * Clean up barrier test data array
 * @param thread_data Array of BarrierTestData to clean up
 */
void cleanup_barrier_test_data(BarrierTestData *thread_data);

/**
 * Create and initialize semaphore test data array
 * @param num_threads Number of threads
 * @param semaphore Semaphore handle
 * @param work_duration_ms Work duration in milliseconds
 * @param completion_order Array to store completion order
 * @param next_position Pointer to next position counter
 * @param order_mutex Mutex for order tracking
 * @return Allocated and initialized test data array
 */
SemaphoreTestData *create_semaphore_test_data(int num_threads,
                                              AsthraConcurrencySemaphoreHandle *semaphore,
                                              int work_duration_ms, int *completion_order,
                                              int *next_position, pthread_mutex_t *order_mutex);

/**
 * Clean up semaphore test data array
 * @param thread_data Array of SemaphoreTestData to clean up
 */
void cleanup_semaphore_test_data(SemaphoreTestData *thread_data);

/**
 * Start multiple threads with barrier test data
 * @param threads Array of pthread_t handles
 * @param thread_data Array of BarrierTestData
 * @param num_threads Number of threads to start
 * @return 0 on success, error code on failure
 */
int start_barrier_threads(pthread_t *threads, BarrierTestData *thread_data, int num_threads);

/**
 * Start multiple threads with semaphore test data
 * @param threads Array of pthread_t handles
 * @param thread_data Array of SemaphoreTestData
 * @param num_threads Number of threads to start
 * @return 0 on success, error code on failure
 */
int start_semaphore_threads(pthread_t *threads, SemaphoreTestData *thread_data, int num_threads);

/**
 * Wait for all threads to complete and validate results
 * @param threads Array of pthread_t handles
 * @param num_threads Number of threads
 * @param expect_success Whether to expect success results
 * @return Number of successful threads
 */
int wait_for_threads(pthread_t *threads, int num_threads, bool expect_success);

/**
 * Count leader threads from barrier test results
 * @param threads Array of pthread_t handles
 * @param num_threads Number of threads
 * @return Number of threads that became leaders
 */
int count_leader_threads(pthread_t *threads, int num_threads);

// ============================================================================
// Test Categories
// ============================================================================

// Select Operations Tests
void test_select_builder_creation(void);
void test_select_recv_cases(void);
void test_select_send_cases(void);
void test_select_timeout_case(void);
void test_select_default_case(void);

// Barrier Tests
void test_barrier_creation_and_cleanup(void);
void test_barrier_invalid_creation(void);
void test_barrier_synchronization(void);
void test_barrier_reuse(void);

// Semaphore Tests
void test_semaphore_creation_and_cleanup(void);
void test_semaphore_acquire_release(void);
void test_semaphore_resource_limiting(void);
void test_semaphore_timeout_operations(void);

// Integration Tests
void test_barrier_semaphore_integration(void);
void test_select_coordination_integration(void);

// Error Handling Tests
void test_coordination_null_pointer_handling(void);

#endif // TEST_CONCURRENT_COORDINATION_COMMON_H
