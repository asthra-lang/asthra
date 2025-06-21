/**
 * Simple Concurrency Tests for Asthra Programming Language
 * Tests basic concurrency concepts without complex dependencies
 * Created: Current session - systematic concurrency category fix
 */

#include "../framework/test_assertions.h"
#include "../framework/test_context.h"
#include "../framework/test_statistics.h"
#include "../framework/test_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Test constants
#define THREAD_COUNT 4
#define ITERATIONS_PER_THREAD 1000
#define SIMPLE_CONCURRENCY_TEST_VALUE 42

// Shared test data structure
typedef struct {
    int counter;
    pthread_mutex_t mutex;
    int thread_results[THREAD_COUNT];
} ConcurrencyTestData;

// Test function declarations
static AsthraTestResult test_basic_thread_creation(AsthraTestContext *context);
static AsthraTestResult test_mutex_synchronization(AsthraTestContext *context);
static AsthraTestResult test_concurrent_counter(AsthraTestContext *context);

// Thread function for basic threading test
static void* basic_thread_function(void* arg) {
    int* result = (int*)arg;
    *result = SIMPLE_CONCURRENCY_TEST_VALUE;
    return NULL;
}

// Thread function for mutex test
static void* mutex_thread_function(void* arg) {
    ConcurrencyTestData* data = (ConcurrencyTestData*)arg;
    
    for (int i = 0; i < ITERATIONS_PER_THREAD; i++) {
        pthread_mutex_lock(&data->mutex);
        data->counter++;
        pthread_mutex_unlock(&data->mutex);
    }
    
    return NULL;
}

// Thread function for concurrent counter test
static void* counter_thread_function(void* arg) {
    int* counter = (int*)arg;
    
    for (int i = 0; i < ITERATIONS_PER_THREAD; i++) {
        __sync_fetch_and_add(counter, 1);  // Atomic increment
    }
    
    return NULL;
}

/**
 * Test basic thread creation and joining
 */
static AsthraTestResult test_basic_thread_creation(AsthraTestContext *context) {
    asthra_test_context_start(context);
    
    pthread_t threads[THREAD_COUNT];
    int results[THREAD_COUNT] = {0};
    
    // Create threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        int create_result = pthread_create(&threads[i], NULL, basic_thread_function, &results[i]);
        if (create_result != 0) {
            printf("Error: Failed to create thread %d (error: %d)\n", i, create_result);
            asthra_test_context_end(context, ASTHRA_TEST_FAIL);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Join threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        int join_result = pthread_join(threads[i], NULL);
        if (join_result != 0) {
            printf("Error: Failed to join thread %d (error: %d)\n", i, join_result);
            asthra_test_context_end(context, ASTHRA_TEST_FAIL);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Verify results
    bool all_correct = true;
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (results[i] != SIMPLE_CONCURRENCY_TEST_VALUE) {
            printf("Error: Thread %d result incorrect: got %d, expected %d\n", 
                   i, results[i], SIMPLE_CONCURRENCY_TEST_VALUE);
            all_correct = false;
        }
    }
    
    printf("Basic thread creation: %d threads created and joined successfully\n", THREAD_COUNT);
    printf("Thread results validation: %s\n", all_correct ? "PASS" : "FAIL");
    
    if (!all_correct) {
        asthra_test_context_end(context, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

/**
 * Test mutex synchronization
 */
static AsthraTestResult test_mutex_synchronization(AsthraTestContext *context) {
    asthra_test_context_start(context);
    
    ConcurrencyTestData data = {0};
    pthread_t threads[THREAD_COUNT];
    
    // Initialize mutex
    int mutex_init_result = pthread_mutex_init(&data.mutex, NULL);
    if (mutex_init_result != 0) {
        printf("Error: Failed to initialize mutex (error: %d)\n", mutex_init_result);
        asthra_test_context_end(context, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    // Create threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        int create_result = pthread_create(&threads[i], NULL, mutex_thread_function, &data);
        if (create_result != 0) {
            printf("Error: Failed to create thread %d (error: %d)\n", i, create_result);
            pthread_mutex_destroy(&data.mutex);
            asthra_test_context_end(context, ASTHRA_TEST_FAIL);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Join threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        int join_result = pthread_join(threads[i], NULL);
        if (join_result != 0) {
            printf("Error: Failed to join thread %d (error: %d)\n", i, join_result);
        }
    }
    
    // Clean up mutex
    pthread_mutex_destroy(&data.mutex);
    
    // Verify counter value
    int expected_value = THREAD_COUNT * ITERATIONS_PER_THREAD;
    printf("Mutex synchronization: expected %d, got %d\n", expected_value, data.counter);
    
    if (data.counter != expected_value) {
        printf("Error: Mutex synchronization failed - counter value incorrect\n");
        asthra_test_context_end(context, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    printf("Mutex synchronization: SUCCESS\n");
    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

/**
 * Test concurrent counter with atomic operations
 */
static AsthraTestResult test_concurrent_counter(AsthraTestContext *context) {
    asthra_test_context_start(context);
    
    pthread_t threads[THREAD_COUNT];
    int shared_counter = 0;
    
    // Create threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        int create_result = pthread_create(&threads[i], NULL, counter_thread_function, &shared_counter);
        if (create_result != 0) {
            printf("Error: Failed to create thread %d (error: %d)\n", i, create_result);
            asthra_test_context_end(context, ASTHRA_TEST_FAIL);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Join threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        int join_result = pthread_join(threads[i], NULL);
        if (join_result != 0) {
            printf("Error: Failed to join thread %d (error: %d)\n", i, join_result);
        }
    }
    
    // Verify counter value
    int expected_value = THREAD_COUNT * ITERATIONS_PER_THREAD;
    printf("Concurrent counter: expected %d, got %d\n", expected_value, shared_counter);
    
    if (shared_counter != expected_value) {
        printf("Error: Concurrent counter failed - atomic operations not working correctly\n");
        asthra_test_context_end(context, ASTHRA_TEST_FAIL);
        return ASTHRA_TEST_FAIL;
    }
    
    printf("Concurrent counter with atomic operations: SUCCESS\n");
    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

int main(void) {
    printf("=== Asthra Concurrency Tests (Simple) ===\n\n");

    // Create test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Define test metadata
    AsthraTestMetadata metadata1 = {
        .name = "test_basic_thread_creation",
        .file = __FILE__,
        .line = __LINE__,
        .function = "test_basic_thread_creation",
        .severity = ASTHRA_TEST_SEVERITY_HIGH,
        .timeout_ns = 30000000000ULL,
        .skip = false,
        .skip_reason = NULL
    };

    AsthraTestMetadata metadata2 = {
        .name = "test_mutex_synchronization",
        .file = __FILE__,
        .line = __LINE__,
        .function = "test_mutex_synchronization",
        .severity = ASTHRA_TEST_SEVERITY_HIGH,
        .timeout_ns = 30000000000ULL,
        .skip = false,
        .skip_reason = NULL
    };

    AsthraTestMetadata metadata3 = {
        .name = "test_concurrent_counter",
        .file = __FILE__,
        .line = __LINE__,
        .function = "test_concurrent_counter",
        .severity = ASTHRA_TEST_SEVERITY_HIGH,
        .timeout_ns = 30000000000ULL,
        .skip = false,
        .skip_reason = NULL
    };

    // Run tests
    AsthraTestContext *context1 = asthra_test_context_create(&metadata1, stats);
    AsthraTestContext *context2 = asthra_test_context_create(&metadata2, stats);
    AsthraTestContext *context3 = asthra_test_context_create(&metadata3, stats);

    if (!context1 || !context2 || !context3) {
        fprintf(stderr, "Failed to create test contexts\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }

    // Run the tests
    AsthraTestResult result1 = test_basic_thread_creation(context1);
    AsthraTestResult result2 = test_mutex_synchronization(context2);
    AsthraTestResult result3 = test_concurrent_counter(context3);

    // Print results
    printf("\nBasic thread creation: %s\n",
           result1 == ASTHRA_TEST_PASS ? "PASS" : "FAIL");
    printf("Mutex synchronization: %s\n",
           result2 == ASTHRA_TEST_PASS ? "PASS" : "FAIL");
    printf("Concurrent counter: %s\n",
           result3 == ASTHRA_TEST_PASS ? "PASS" : "FAIL");

    // Print final statistics
    printf("\n");
    asthra_test_statistics_print(stats, false);

    // Cleanup
    asthra_test_context_destroy(context1);
    asthra_test_context_destroy(context2);
    asthra_test_context_destroy(context3);
    asthra_test_statistics_destroy(stats);

    // Return appropriate exit code
    if (result1 == ASTHRA_TEST_PASS && result2 == ASTHRA_TEST_PASS && result3 == ASTHRA_TEST_PASS) {
        printf("All concurrency tests passed!\n");
        return 0;
    } else {
        printf("Some concurrency tests failed!\n");
        return 1;
    }
} 
