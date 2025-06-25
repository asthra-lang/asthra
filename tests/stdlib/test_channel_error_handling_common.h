/*
 * Common Header for Channel Error Handling Tests
 * Shared definitions, types, and function declarations for error handling tests
 *
 * Part of test_channel_error_handling.c split (716 lines -> 7 focused modules)
 * Provides comprehensive error handling test infrastructure
 */

#ifndef TEST_CHANNEL_ERROR_HANDLING_COMMON_H
#define TEST_CHANNEL_ERROR_HANDLING_COMMON_H

#include "test_concurrent_channels_common.h"

// ============================================================================
// ERROR HANDLING TEST CONSTANTS
// ============================================================================

// Test timeouts and limits
#define SHORT_TIMEOUT_MS 50
#define MEDIUM_TIMEOUT_MS 200
#define LONG_TIMEOUT_MS 1000

// Resource limits for testing
#define MAX_TEST_CHANNELS 1000
#define LARGE_CHANNEL_CAPACITY 10000
#define LARGE_DATA_SIZE 8192
#define VERY_LARGE_DATA_SIZE 16384

// Thread counts for concurrent testing
#define ERROR_TEST_THREAD_COUNT 6
#define DESTRUCTION_TEST_THREAD_COUNT 4

// Performance thresholds
#define MIN_SUCCESS_RATE_PERCENT 60.0
#define MAX_RETRY_COUNT 5

// ============================================================================
// ERROR HANDLING TEST DATA STRUCTURES
// ============================================================================

/**
 * Configuration for error handling tests
 */
typedef struct {
    size_t channel_capacity;
    int num_threads;
    int operations_per_thread;
    uint64_t timeout_ms;
    bool inject_errors;
    double error_rate;
} ErrorTestConfig;

/**
 * Results from error handling tests
 */
typedef struct {
    int total_operations;
    int successful_operations;
    int failed_operations;
    int timeout_operations;
    int error_operations;
    double success_rate;
    uint64_t elapsed_time_ms;
} ErrorTestResults;

/**
 * Test structure for data size boundary testing
 */
typedef struct {
    int a;
    double b;
    char c[16];
} TestStruct;

// ============================================================================
// CORE ERROR HANDLING UTILITY FUNCTIONS
// ============================================================================

/**
 * Create error test configuration with defaults
 */
ErrorTestConfig create_error_test_config(size_t capacity, int threads, int ops);

/**
 * Initialize error test results structure
 */
ErrorTestResults init_error_test_results(void);

/**
 * Update error test results with operation outcome
 */
void update_error_test_results(ErrorTestResults *results, AsthraConcurrencyResult operation_result);

/**
 * Calculate final error test statistics
 */
void finalize_error_test_results(ErrorTestResults *results, uint64_t start_time);

/**
 * Print error test results summary
 */
void print_error_test_summary(const char *test_name, const ErrorTestResults *results);

/**
 * Print error handling report
 */
void print_error_handling_report(const char *category, size_t tests_run, size_t tests_passed);

// ============================================================================
// ERROR INJECTION AND SIMULATION FUNCTIONS
// ============================================================================

/**
 * Simulate memory exhaustion by creating many channels
 */
int simulate_memory_exhaustion(AsthraConcurrencyChannelHandle **channels, int max_channels);

/**
 * Test channel overflow pattern by filling to capacity
 */
bool test_channel_overflow_pattern(AsthraConcurrencyChannelHandle *channel, int capacity);

/**
 * Inject random errors into channel operations
 */
AsthraConcurrencyResult inject_operation_error(double error_rate);

/**
 * Test retry pattern with recovery logic
 */
bool test_retry_pattern(AsthraConcurrencyChannelHandle *channel, int max_retries);

// ============================================================================
// BOUNDARY CONDITION TEST FUNCTIONS
// ============================================================================

/**
 * Test zero capacity channel edge cases
 */
bool test_zero_capacity_edge_cases(AsthraConcurrencyChannelHandle *channel);

/**
 * Test maximum capacity scenarios
 */
bool test_maximum_capacity_scenarios(AsthraConcurrencyChannelHandle *channel, size_t capacity);

/**
 * Test data size boundaries pattern
 */
bool test_data_size_boundaries_pattern(AsthraConcurrencyChannelHandle *channel);

/**
 * Validate large data transfer
 */
bool validate_large_data_transfer(AsthraConcurrencyChannelHandle *channel, size_t data_size);

// ============================================================================
// CONCURRENT ERROR TEST FUNCTIONS
// ============================================================================

/**
 * Run concurrent error tests with multiple threads
 */
int run_concurrent_error_tests(AsthraConcurrencyChannelHandle *channel, int num_threads,
                               ErrorTestResults *results);

/**
 * Test destruction with active threads
 */
bool test_destruction_with_active_threads(AsthraConcurrencyChannelHandle *channel, int num_threads);

/**
 * Monitor thread completion and collect results
 */
int monitor_thread_completion(pthread_t *threads, int num_threads, ThreadTestData **thread_data);

// ============================================================================
// RECOVERY AND RESILIENCE TEST FUNCTIONS
// ============================================================================

/**
 * Test error recovery patterns
 */
bool test_error_recovery_patterns(AsthraConcurrencyChannelHandle *channel);

/**
 * Test graceful degradation pattern
 */
bool test_graceful_degradation_pattern(AsthraConcurrencyChannelHandle **channels, int num_channels);

/**
 * Validate success rate meets minimum threshold
 */
bool validate_success_rate(int successful, int total, double min_rate);

// ============================================================================
// SPECIALIZED THREAD FUNCTIONS FOR ERROR TESTING
// ============================================================================

/**
 * Producer thread that intentionally generates errors
 */
void *error_producer_thread(void *arg);

/**
 * Consumer thread that handles errors gracefully
 */
void *error_consumer_thread(void *arg);

/**
 * Thread that tests timeout scenarios
 */
void *timeout_test_thread(void *arg);

/**
 * Thread that tests channel destruction scenarios
 */
void *destruction_test_thread(void *arg);

// ============================================================================
// ERROR VALIDATION AND REPORTING FUNCTIONS
// ============================================================================

/**
 * Validate null pointer error handling
 */
bool validate_null_pointer_errors(void);

/**
 * Validate invalid data error handling
 */
bool validate_invalid_data_errors(AsthraConcurrencyChannelHandle *channel);

/**
 * Validate timeout error handling
 */
bool validate_timeout_errors(AsthraConcurrencyChannelHandle *channel);

/**
 * Validate invalid handle error handling
 */
bool validate_invalid_handle_errors(void);

#endif // TEST_CHANNEL_ERROR_HANDLING_COMMON_H
