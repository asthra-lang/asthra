/**
 * Asthra Programming Language - Concurrency Stress Tests
 * 
 * Phase 4: Core Infrastructure Implementation
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Stress test implementations for concurrency testing
 */

#include "concurrency_integration_common.h"

// =============================================================================
// CONCURRENCY TESTING FUNCTIONS
// =============================================================================

/**
 * Producer thread function for queue stress test
 */
void* concurrency_producer_thread(void* arg) {
    ConcurrencyStressTestData* data = (ConcurrencyStressTestData*)arg;
    if (!data || !g_concurrency_context) {
        return NULL;
    }
    
    char thread_name[64];
    snprintf(thread_name, sizeof(thread_name), "producer_%d", data->thread_id);
    
    concurrency_register_test_thread(thread_name);
    
    bool success = true;
    for (int i = 0; i < data->iterations && success; i++) {
        for (int j = 0; j < data->operations_per_iteration; j++) {
            int test_data = data->thread_id * 1000 + i * 100 + j;
            if (!lockfree_queue_enqueue(g_concurrency_context->test_queue, test_data)) {
                success = false;
                atomic_fetch_add(&g_concurrency_context->failed_operations, 1);
                break;
            } else {
                atomic_fetch_add(&g_concurrency_context->successful_operations, 1);
            }
            atomic_fetch_add(&g_concurrency_context->total_operations, 1);
        }
        
        // Small delay to allow other threads to work
        sleep_ms(1);
    }
    
    data->test_result = success;
    concurrency_unregister_test_thread();
    return NULL;
}

/**
 * Consumer thread function for queue stress test
 */
void* concurrency_consumer_thread(void* arg) {
    ConcurrencyStressTestData* data = (ConcurrencyStressTestData*)arg;
    if (!data || !g_concurrency_context) {
        return NULL;
    }
    
    char thread_name[64];
    snprintf(thread_name, sizeof(thread_name), "consumer_%d", data->thread_id);
    
    concurrency_register_test_thread(thread_name);
    
    bool success = true;
    int consumed_items = 0;
    
    while (consumed_items < data->iterations * data->operations_per_iteration && success) {
        int consumed_data;
        if (lockfree_queue_dequeue(g_concurrency_context->test_queue, &consumed_data)) {
            consumed_items++;
            atomic_fetch_add(&g_concurrency_context->successful_operations, 1);
        } else {
            // Queue might be empty, small delay and retry
            sleep_ms(1);
        }
        atomic_fetch_add(&g_concurrency_context->total_operations, 1);
    }
    
    data->test_result = success;
    concurrency_unregister_test_thread();
    return NULL;
}

/**
 * Synchronization stress test thread
 */
void* concurrency_sync_stress_thread(void* arg) {
    ConcurrencyStressTestData* data = (ConcurrencyStressTestData*)arg;
    if (!data || !g_concurrency_context) {
        return NULL;
    }
    
    char thread_name[64];
    snprintf(thread_name, sizeof(thread_name), "sync_stress_%d", data->thread_id);
    
    concurrency_register_test_thread(thread_name);
    
    bool success = true;
    
    // Test mutex performance
    if (!sync_primitives_test_mutex(g_concurrency_context->sync_primitives, data->iterations)) {
        success = false;
        atomic_fetch_add(&g_concurrency_context->failed_operations, 1);
    }
    
    // Test read lock performance
    if (success && !sync_primitives_test_rwlock_read(g_concurrency_context->sync_primitives, data->iterations / 2)) {
        success = false;
        atomic_fetch_add(&g_concurrency_context->failed_operations, 1);
    }
    
    // Test barrier synchronization
    if (success && !sync_primitives_barrier_wait(g_concurrency_context->sync_primitives)) {
        success = false;
        atomic_fetch_add(&g_concurrency_context->failed_operations, 1);
    }
    
    if (success) {
        atomic_fetch_add(&g_concurrency_context->successful_operations, data->iterations);
    }
    
    atomic_fetch_add(&g_concurrency_context->total_operations, data->iterations);
    
    data->test_result = success;
    concurrency_unregister_test_thread();
    return NULL;
}

/**
 * Run comprehensive concurrency stress test
 */
bool run_concurrency_stress_test(int num_threads, int iterations_per_thread) {
    if (!concurrency_test_initialize((size_t)num_threads * 2)) {
        printf("❌ Failed to initialize concurrency test context\n");
        return false;
    }
    
    if (!concurrency_test_start()) {
        printf("❌ Failed to start concurrency test\n");
        concurrency_test_cleanup();
        return false;
    }
    
    printf("Running concurrency stress test with %d threads, %d iterations each...\n", 
           num_threads, iterations_per_thread);
    
    // Create thread data
    ConcurrencyStressTestData* producer_data = calloc((size_t)num_threads, sizeof(ConcurrencyStressTestData));
    ConcurrencyStressTestData* consumer_data = calloc((size_t)num_threads, sizeof(ConcurrencyStressTestData));
    pthread_t* producer_threads = calloc((size_t)num_threads, sizeof(pthread_t));
    pthread_t* consumer_threads = calloc((size_t)num_threads, sizeof(pthread_t));
    bool all_passed = true;
    
    if (!producer_data || !consumer_data || !producer_threads || !consumer_threads) {
        printf("❌ Failed to allocate thread data\n");
        all_passed = false;
        goto cleanup_and_exit;
    }
    
    // Initialize thread data
    for (int i = 0; i < num_threads; i++) {
        producer_data[i].thread_id = i;
        producer_data[i].iterations = iterations_per_thread;
        producer_data[i].operations_per_iteration = 10;
        producer_data[i].test_result = false;
        
        consumer_data[i].thread_id = i + num_threads;
        consumer_data[i].iterations = iterations_per_thread;
        consumer_data[i].operations_per_iteration = 10;
        consumer_data[i].test_result = false;
    }
    
    // Start producer threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&producer_threads[i], NULL, concurrency_producer_thread, &producer_data[i]) != 0) {
            printf("❌ Failed to create producer thread %d\n", i);
            all_passed = false;
            goto cleanup_and_exit;
        }
    }
    
    // Start consumer threads
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&consumer_threads[i], NULL, concurrency_consumer_thread, &consumer_data[i]) != 0) {
            printf("❌ Failed to create consumer thread %d\n", i);
            all_passed = false;
            goto cleanup_and_exit;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < num_threads; i++) {
        pthread_join(producer_threads[i], NULL);
        pthread_join(consumer_threads[i], NULL);
    }
    
    concurrency_test_stop();
    
    // Check results
    for (int i = 0; i < num_threads; i++) {
        if (!producer_data[i].test_result || !consumer_data[i].test_result) {
            all_passed = false;
            break;
        }
    }
    
    // Print statistics
    size_t total_ops, successful_ops, failed_ops, race_conditions, deadlocks;
    double duration_ms;
    concurrency_test_get_statistics(&total_ops, &successful_ops, &failed_ops, 
                                   &duration_ms, &race_conditions, &deadlocks);
    
    printf("Concurrency test completed in %.2f ms\n", duration_ms);
    printf("Total operations: %zu\n", total_ops);
    printf("Successful operations: %zu\n", successful_ops);
    printf("Failed operations: %zu\n", failed_ops);
    printf("Race conditions detected: %zu\n", race_conditions);
    printf("Deadlocks detected: %zu\n", deadlocks);
    
    if (all_passed && failed_ops == 0) {
        printf("✅ Concurrency stress test passed!\n");
    } else {
        printf("❌ Concurrency stress test failed!\n");
        all_passed = false;
    }
    
cleanup_and_exit:
    free(producer_data);
    free(consumer_data);
    free(producer_threads);
    free(consumer_threads);
    concurrency_test_cleanup();
    
    return all_passed;
}

/**
 * Test concurrency performance integration
 */
bool test_concurrency_performance_integration(void) {
    printf("Testing concurrency performance integration...\n");
    
    // Use performance validation infrastructure from Phase 3
    bool performance_acceptable = performance_validation_run_complete("concurrency_integration_performance");
    
    if (performance_acceptable) {
        printf("✅ Concurrency performance integration test passed\n");
    } else {
        printf("❌ Concurrency performance integration test failed\n");
    }
    
    return performance_acceptable;
}

/**
 * Run complete concurrency integration test suite
 */
bool run_concurrency_integration_tests(void) {
    printf("\n=== Concurrency Integration Test Suite ===\n");
    
    bool all_passed = true;
    
    // Test 1: Basic lock-free queue operations
    printf("\nTesting basic lock-free operations...\n");
    if (!concurrency_test_initialize(4)) {
        printf("❌ Failed to initialize basic test\n");
        return false;
    }
    
    LockFreeQueue* test_queue = lockfree_queue_create();
    bool basic_test = true;
    
    // Test enqueue/dequeue
    for (int i = 0; i < 100; i++) {
        if (!lockfree_queue_enqueue(test_queue, i)) {
            basic_test = false;
            break;
        }
    }
    
    for (int i = 0; i < 100; i++) {
        int data;
        if (!lockfree_queue_dequeue(test_queue, &data) || data != i) {
            basic_test = false;
            break;
        }
    }
    
    lockfree_queue_destroy(test_queue);
    concurrency_test_cleanup();
    
    if (basic_test) {
        printf("✅ Basic lock-free operations test passed\n");
    } else {
        printf("❌ Basic lock-free operations test failed\n");
        all_passed = false;
    }
    
    // Test 2: Multi-threaded stress test
    all_passed &= run_concurrency_stress_test(4, 100);
    
    // Test 3: Performance integration
    all_passed &= test_concurrency_performance_integration();
    
    printf("\n=== Test Results ===\n");
    if (all_passed) {
        printf("✅ All concurrency integration tests passed!\n");
    } else {
        printf("❌ Some concurrency integration tests failed!\n");
    }
    
    return all_passed;
}