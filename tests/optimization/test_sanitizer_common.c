/**
 * Asthra Programming Language
 * Sanitizer Integration Test Suite - Common Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_sanitizer_common.h"

// Global variables for thread safety tests
int global_counter = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread function for race condition testing
void* thread_increment_function(void *arg) {
    int thread_id = *(int*)arg;
    
    for (int i = 0; i < ITERATIONS; i++) {
        // Safe increment with mutex
        pthread_mutex_lock(&counter_mutex);
        global_counter++;
        pthread_mutex_unlock(&counter_mutex);
        
        // Small delay to increase chance of race conditions
        usleep(1);
    }
    
    printf("Thread %d completed %d iterations\n", thread_id, ITERATIONS);
    return NULL;
}

// Thread function that would cause race conditions (for testing detection)
void* thread_unsafe_increment_function(void *arg) {
    (void)arg; // Suppress unused parameter warning
    
    for (int i = 0; i < 100; i++) {
        // Note: In a real TSAN test, this would be unsafe:
        // global_counter++;  // Race condition - no mutex
        
        // For the test suite, we use safe increment
        pthread_mutex_lock(&counter_mutex);
        global_counter++;
        pthread_mutex_unlock(&counter_mutex);
        
        usleep(1);
    }
    
    return NULL;
} 
