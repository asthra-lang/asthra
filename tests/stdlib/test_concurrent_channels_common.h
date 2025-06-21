/*
 * Common Header for Stdlib Concurrent Channels Tests
 * Shared definitions, types, and function declarations
 * 
 * Phase 8: Testing and Validation
 * Part of test_concurrent_channels.c split (746 lines -> 7 focused modules)
 */

#ifndef TEST_CONCURRENT_CHANNELS_COMMON_H
#define TEST_CONCURRENT_CHANNELS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "../framework/test_framework.h"
#include "../../runtime/stdlib_concurrency_support.h"

// ============================================================================
// GLOBAL TEST FRAMEWORK
// ============================================================================

// Global test tracking (defined in test_concurrent_channels_common.c)
extern atomic_int g_tests_run;
extern atomic_int g_tests_passed;

// Test macros with atomic updates
#define ASSERT_TRUE(condition) \
    do { \
        atomic_fetch_add(&g_tests_run, 1); \
        if (condition) { \
            atomic_fetch_add(&g_tests_passed, 1); \
            printf("  ✅ PASS: %s\n", #condition); \
        } else { \
            printf("  ❌ FAIL: %s:%d - %s\n", __FILE__, __LINE__, #condition); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_EQUAL(a, b) ASSERT_TRUE((a) == (b))

// ============================================================================
// SHARED TYPES AND STRUCTURES
// ============================================================================

/**
 * Thread test data structure for multi-threaded tests
 */
typedef struct {
    AsthraConcurrencyChannelHandle* channel;
    int* values;
    int count;
    int thread_id;
    void* extra_data;  // For additional test-specific data
} ThreadTestData;

/**
 * Channel test configuration
 */
typedef struct {
    size_t buffer_capacity;
    int num_producers;
    int num_consumers;
    int items_per_producer;
    int items_per_consumer;
    uint64_t timeout_ms;
    bool use_timeout;
} ChannelTestConfig;

/**
 * Channel test results
 */
typedef struct {
    int total_sent;
    int total_received;
    int successful_operations;
    int failed_operations;
    uint64_t elapsed_time_ms;
    double throughput_ops_per_sec;
} ChannelTestResults;

// ============================================================================
// SHARED UTILITY FUNCTIONS
// ============================================================================

/**
 * Initialize global test tracking
 */
void init_test_tracking(void);

/**
 * Get current test statistics
 */
void get_test_statistics(int* total_run, int* total_passed);

/**
 * Reset test statistics
 */
void reset_test_statistics(void);

/**
 * Create thread test data
 */
ThreadTestData* create_thread_test_data(AsthraConcurrencyChannelHandle* channel, 
                                       int count, int thread_id);

/**
 * Free thread test data
 */
void free_thread_test_data(ThreadTestData* data);

/**
 * Create default channel test configuration
 */
ChannelTestConfig create_default_channel_config(void);

/**
 * Create test channel with specified capacity
 */
AsthraConcurrencyChannelHandle* create_test_channel(size_t capacity);

/**
 * Clean up test channel
 */
void cleanup_test_channel(AsthraConcurrencyChannelHandle* channel);

/**
 * Create multiple test channels
 */
AsthraConcurrencyChannelHandle** create_test_channels(int count, size_t capacity);

/**
 * Clean up multiple test channels
 */
void cleanup_test_channels(AsthraConcurrencyChannelHandle** channels, int count);

/**
 * Validate channel properties
 */
bool validate_channel_properties(AsthraConcurrencyChannelHandle* channel, 
                                size_t expected_capacity, 
                                size_t expected_length,
                                bool expected_closed);

/**
 * Get current timestamp in milliseconds
 */
uint64_t get_current_timestamp_ms(void);

/**
 * Calculate elapsed time
 */
uint64_t calculate_elapsed_time(uint64_t start_time, uint64_t end_time);

// ============================================================================
// SHARED THREAD FUNCTIONS
// ============================================================================

/**
 * Generic producer thread function
 */
void* producer_thread(void* arg);

/**
 * Generic consumer thread function
 */
void* consumer_thread(void* arg);

/**
 * Producer thread with timeout
 */
void* producer_thread_timeout(void* arg);

/**
 * Consumer thread with timeout
 */
void* consumer_thread_timeout(void* arg);

/**
 * Producer thread with error injection
 */
void* producer_thread_with_errors(void* arg);

/**
 * Consumer thread with error injection
 */
void* consumer_thread_with_errors(void* arg);

// ============================================================================
// SHARED VALIDATION FUNCTIONS
// ============================================================================

/**
 * Validate send operation result
 */
bool validate_send_result(AsthraConcurrencyResult result, bool should_succeed);

/**
 * Validate receive operation result
 */
bool validate_receive_result(AsthraConcurrencyResult result, bool should_succeed);

/**
 * Validate channel statistics
 */
bool validate_channel_statistics(AsthraConcurrencyStats* stats,
                                uint64_t min_channels,
                                uint64_t min_send_ops,
                                uint64_t min_recv_ops);

/**
 * Validate timeout behavior
 */
bool validate_timeout_behavior(AsthraConcurrencyResult result, 
                              uint64_t start_time, 
                              uint64_t timeout_ms,
                              double tolerance_factor);

/**
 * Validate memory usage
 */
bool validate_memory_usage(uint64_t initial_memory, 
                          uint64_t current_memory,
                          uint64_t expected_increase_bytes);

// ============================================================================
// SHARED ERROR HANDLING
// ============================================================================

/**
 * Convert result code to string
 */
const char* result_code_to_string(AsthraConcurrencyResult result);

/**
 * Print error message with context
 */
void print_error_with_context(const char* test_name, 
                             const char* operation,
                             AsthraConcurrencyResult result);

/**
 * Handle expected error gracefully
 */
bool handle_expected_error(AsthraConcurrencyResult result, 
                          AsthraConcurrencyResult expected_result);

// ============================================================================
// SHARED CONSTANTS
// ============================================================================

// Test timeouts
#define SHORT_TIMEOUT_MS    100
#define MEDIUM_TIMEOUT_MS   1000
#define LONG_TIMEOUT_MS     5000

// Test sizes
#define SMALL_BUFFER_SIZE   5
#define MEDIUM_BUFFER_SIZE  50
#define LARGE_BUFFER_SIZE   1000

// Thread counts
#define SMALL_THREAD_COUNT  2
#define MEDIUM_THREAD_COUNT 4
#define LARGE_THREAD_COUNT  8

// Operation counts
#define SMALL_OP_COUNT      10
#define MEDIUM_OP_COUNT     100
#define LARGE_OP_COUNT      1000

// Memory thresholds
#define MIN_MEMORY_INCREASE 1024    // 1KB
#define MAX_MEMORY_INCREASE 1048576 // 1MB

// Performance thresholds
#define MIN_THROUGHPUT_OPS_PER_SEC 1000.0
#define MIN_SUCCESS_RATE_PERCENT   95.0

#endif // TEST_CONCURRENT_CHANNELS_COMMON_H 
