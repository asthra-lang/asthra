/*
 * Coordination Performance Tests
 * Performance testing for coordination primitives (select, barrier, semaphore)
 * 
 * Phase 8: Testing and Validation
 * Focus: Coordination primitive performance and synchronization overhead
 */

#include "test_concurrency_benchmarks_common.h"

// ============================================================================
// Select Performance Tests
// ============================================================================

void test_select_performance(void) {
    printf("Testing select operation performance...\n");
    
    const int NUM_OPERATIONS = 10000;
    const int NUM_CHANNELS = 4;
    
    AsthraConcurrencyChannelHandle* channels[NUM_CHANNELS];
    for (int i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = asthra_channel_create(100);
        ASSERT_TRUE(channels[i] != NULL);
    }
    
    Timer timer;
    timer_start(&timer);
    
    for (int op = 0; op < NUM_OPERATIONS; op++) {
        AsthraConcurrencySelectBuilder* builder = asthra_select_builder_create();
        ASSERT_TRUE(builder != NULL);
        
        // Add recv cases for all channels
        for (int i = 0; i < NUM_CHANNELS; i++) {
            AsthraConcurrencyResult result = asthra_select_builder_add_recv_case(builder, channels[i], sizeof(int));
            ASSERT_TRUE(result == ASTHRA_CONCURRENCY_SUCCESS);
        }
        
        // Add timeout case
        asthra_select_builder_add_timeout_case(builder, 1); // 1ms timeout
        
        // Send data to random channel
        int random_channel = rand() % NUM_CHANNELS;
        asthra_channel_send(channels[random_channel], &op, sizeof(int));
        
        // Execute select
        int selected_case;
        int received_data;
        AsthraConcurrencyResult result = asthra_select_execute(builder, &selected_case, &received_data, sizeof(int));
        ASSERT_TRUE(result == ASTHRA_CONCURRENCY_SUCCESS);
        
        asthra_select_builder_destroy(builder);
    }
    
    double elapsed_ms = timer_stop(&timer);
    double ops_per_second = (NUM_OPERATIONS / elapsed_ms) * 1000.0;
    
    printf("  Select performance: %.0f operations/second (%.2f ms total)\n",
           ops_per_second, elapsed_ms);
    
    // Should achieve at least 1K select operations per second
    ASSERT_TRUE(ops_per_second >= 1000.0);
    
    for (int i = 0; i < NUM_CHANNELS; i++) {
        asthra_channel_destroy(channels[i]);
    }
}

// ============================================================================
// Barrier Performance Tests
// ============================================================================

void test_barrier_performance(void) {
    printf("Testing barrier synchronization performance...\n");
    
    const int NUM_THREADS = 8;
    const int NUM_ITERATIONS = 1000;
    
    AsthraConcurrencyBarrierHandle* barrier = asthra_barrier_create(NUM_THREADS);
    ASSERT_TRUE(barrier != NULL);
    
    pthread_t threads[NUM_THREADS];
    double thread_times[NUM_THREADS];
    
    Timer timer;
    timer_start(&timer);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        int* thread_id = malloc(sizeof(int));
        *thread_id = i;
        
        pthread_create(&threads[i], NULL, ^void* (void* arg) {
            int id = *(int*)arg;
            free(arg);
            
            Timer thread_timer;
            timer_start(&thread_timer);
            
            for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
                bool is_leader;
                AsthraConcurrencyResult result = asthra_barrier_wait(barrier, &is_leader);
                if (result != ASTHRA_CONCURRENCY_SUCCESS) {
                    return (void*)(intptr_t)result;
                }
                
                // Small amount of work
                usleep(100); // 0.1ms
            }
            
            double elapsed = timer_stop(&thread_timer);
            thread_times[id] = elapsed;
            
            return (void*)(intptr_t)ASTHRA_CONCURRENCY_SUCCESS;
        }, thread_id);
    }
    
    // Wait for all threads
    for (int i = 0; i < NUM_THREADS; i++) {
        void* result;
        pthread_join(threads[i], &result);
        ASSERT_TRUE((intptr_t)result == ASTHRA_CONCURRENCY_SUCCESS);
    }
    
    double total_elapsed = timer_stop(&timer);
    
    // Calculate statistics
    double total_thread_time = 0.0;
    for (int i = 0; i < NUM_THREADS; i++) {
        total_thread_time += thread_times[i];
    }
    double avg_thread_time = total_thread_time / NUM_THREADS;
    
    printf("  Barrier performance: %.2f ms total, %.2f ms average per thread\n",
           total_elapsed, avg_thread_time);
    printf("  Synchronization overhead: %.2f ms per barrier operation\n",
           (avg_thread_time - (NUM_ITERATIONS * 0.1)) / NUM_ITERATIONS);
    
    // Synchronization overhead should be minimal
    double overhead_per_op = (avg_thread_time - (NUM_ITERATIONS * 0.1)) / NUM_ITERATIONS;
    ASSERT_TRUE(overhead_per_op < 1.0); // Less than 1ms overhead per barrier
    
    asthra_barrier_destroy(barrier);
}

// ============================================================================
// Semaphore Performance Tests
// ============================================================================

void test_semaphore_performance(void) {
    printf("Testing semaphore performance...\n");
    
    const int NUM_THREADS = 8;
    const int NUM_OPERATIONS = 10000;
    const int MAX_CONCURRENT = 4;
    
    AsthraConcurrencySemaphoreHandle* semaphore = asthra_semaphore_create(MAX_CONCURRENT);
    ASSERT_TRUE(semaphore != NULL);
    
    pthread_t threads[NUM_THREADS];
    double thread_times[NUM_THREADS];
    
    Timer timer;
    timer_start(&timer);
    
    for (int i = 0; i < NUM_THREADS; i++) {
        int* thread_id = malloc(sizeof(int));
        *thread_id = i;
        
        pthread_create(&threads[i], NULL, ^void* (void* arg) {
            int id = *(int*)arg;
            free(arg);
            
            Timer thread_timer;
            timer_start(&thread_timer);
            
            for (int op = 0; op < NUM_OPERATIONS / NUM_THREADS; op++) {
                // Acquire semaphore
                AsthraConcurrencyResult result = asthra_semaphore_acquire(semaphore);
                if (result != ASTHRA_CONCURRENCY_SUCCESS) {
                    return (void*)(intptr_t)result;
                }
                
                // Minimal work while holding semaphore
                usleep(10); // 0.01ms
                
                // Release semaphore
                result = asthra_semaphore_release(semaphore);
                if (result != ASTHRA_CONCURRENCY_SUCCESS) {
                    return (void*)(intptr_t)result;
                }
            }
            
            double elapsed = timer_stop(&thread_timer);
            thread_times[id] = elapsed;
            
            return (void*)(intptr_t)ASTHRA_CONCURRENCY_SUCCESS;
        }, thread_id);
    }
    
    // Wait for all threads
    for (int i = 0; i < NUM_THREADS; i++) {
        void* result;
        pthread_join(threads[i], &result);
        ASSERT_TRUE((intptr_t)result == ASTHRA_CONCURRENCY_SUCCESS);
    }
    
    double total_elapsed = timer_stop(&timer);
    
    double ops_per_second = (NUM_OPERATIONS / total_elapsed) * 1000.0;
    printf("  Semaphore performance: %.0f acquire/release operations/second\n", ops_per_second);
    
    // Should achieve good throughput
    ASSERT_TRUE(ops_per_second >= 10000.0);
    
    asthra_semaphore_destroy(semaphore);
} 
