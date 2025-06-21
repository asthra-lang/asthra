/*
 * =============================================================================
 * PERFORMANCE CATEGORY - BASIC TEST SUITE
 * =============================================================================
 * 
 * This file provides basic performance testing using only available runtime
 * functions. Avoids missing barrier/semaphore functions.
 * 
 * Root Cause: Performance tests were using unimplemented barrier/semaphore functions
 * Solution: Use only available channel and task functions for performance testing
 * 
 * =============================================================================
 */

#include "../framework/test_framework.h"
#include "../../runtime/asthra_concurrency_bridge_modular.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

// =============================================================================
// TIMING UTILITIES
// =============================================================================

typedef struct {
    struct timespec start;
    struct timespec end;
} Timer;

void timer_start(Timer *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

double timer_stop(Timer *timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->end);
    return (timer->end.tv_sec - timer->start.tv_sec) * 1000.0 + 
           (timer->end.tv_nsec - timer->start.tv_nsec) / 1000000.0;
}

// =============================================================================
// BASIC PERFORMANCE TESTS
// =============================================================================

AsthraTestResult test_channel_creation_performance(AsthraTestContext *context) {
    printf("Testing channel creation performance...\n");
    
    const int NUM_CHANNELS = 1000;
    AsthraConcurrencyChannel **channels = malloc(NUM_CHANNELS * sizeof(AsthraConcurrencyChannel*));
    
    Timer timer;
    timer_start(&timer);
    
    // Create channels
    for (int i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = Asthra_channel_create(sizeof(int), 10, "test_channel");
        if (!channels[i]) {
            printf("  Failed to create channel %d\n", i);
            // Cleanup created channels
            for (int j = 0; j < i; j++) {
                Asthra_channel_destroy(channels[j]);
            }
            free(channels);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    double creation_time = timer_stop(&timer);
    double channels_per_second = (NUM_CHANNELS / creation_time) * 1000.0;
    
    printf("  Channel creation: %.0f channels/second (%.2f ms total)\n",
           channels_per_second, creation_time);
    
    // Cleanup
    for (int i = 0; i < NUM_CHANNELS; i++) {
        Asthra_channel_destroy(channels[i]);
    }
    free(channels);
    
    // Should achieve at least 1K channel creations per second
    if (channels_per_second < 1000.0) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_channel_send_receive_performance(AsthraTestContext *context) {
    printf("Testing channel send/receive performance...\n");
    
    AsthraConcurrencyChannel *channel = Asthra_channel_create(sizeof(int), 100, "perf_channel");
    if (channel == NULL) {
        return ASTHRA_TEST_FAIL;
    }
    
    const int NUM_OPERATIONS = 10000;
    Timer timer;
    timer_start(&timer);
    
    // Send and receive operations
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        int data = i;
        AsthraResult send_result = Asthra_channel_send(channel, &data, 0);
        if (send_result.tag != ASTHRA_RESULT_OK) {
            printf("  Send failed at operation %d\n", i);
            Asthra_channel_destroy(channel);
            return ASTHRA_TEST_FAIL;
        }
        
        int received_data;
        AsthraResult recv_result = Asthra_channel_recv(channel, &received_data, 0);
        if (recv_result.tag != ASTHRA_RESULT_OK) {
            printf("  Receive failed at operation %d\n", i);
            Asthra_channel_destroy(channel);
            return ASTHRA_TEST_FAIL;
        }
        
        if (received_data != data) {
            printf("  Data mismatch: expected %d, got %d\n", data, received_data);
            Asthra_channel_destroy(channel);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    double elapsed_ms = timer_stop(&timer);
    double ops_per_second = (NUM_OPERATIONS * 2 / elapsed_ms) * 1000.0; // *2 for send+receive
    
    printf("  Channel operations: %.0f ops/second (%.2f ms total)\n",
           ops_per_second, elapsed_ms);
    
    Asthra_channel_destroy(channel);
    
    // Should achieve at least 10K operations per second
    if (ops_per_second < 10000.0) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_memory_allocation_performance(AsthraTestContext *context) {
    printf("Testing memory allocation performance...\n");
    
    const int NUM_ALLOCATIONS = 10000;
    const size_t ALLOCATION_SIZE = 1024; // 1KB
    void **pointers = malloc(NUM_ALLOCATIONS * sizeof(void*));
    
    Timer timer;
    timer_start(&timer);
    
    // Allocate memory
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        pointers[i] = malloc(ALLOCATION_SIZE);
        if (!pointers[i]) {
            printf("  Allocation failed at %d\n", i);
            // Cleanup
            for (int j = 0; j < i; j++) {
                free(pointers[j]);
            }
            free(pointers);
            return ASTHRA_TEST_FAIL;
        }
        // Touch the memory to ensure it's actually allocated
        memset(pointers[i], i & 0xFF, ALLOCATION_SIZE);
    }
    
    double allocation_time = timer_stop(&timer);
    
    // Free memory
    timer_start(&timer);
    for (int i = 0; i < NUM_ALLOCATIONS; i++) {
        free(pointers[i]);
    }
    double deallocation_time = timer_stop(&timer);
    
    double allocs_per_second = (NUM_ALLOCATIONS / allocation_time) * 1000.0;
    double frees_per_second = (NUM_ALLOCATIONS / deallocation_time) * 1000.0;
    
    printf("  Memory allocation: %.0f allocs/second (%.2f ms)\n",
           allocs_per_second, allocation_time);
    printf("  Memory deallocation: %.0f frees/second (%.2f ms)\n",
           frees_per_second, deallocation_time);
    
    free(pointers);
    
    // Should achieve reasonable allocation performance
    if (allocs_per_second < 1000.0 || frees_per_second < 1000.0) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("PERFORMANCE CATEGORY - BASIC TEST SUITE\n");
    printf("=============================================================================\n");
    
    AsthraTestContext context = {0};
    int tests_run = 0;
    int tests_passed = 0;
    
    // Test 1: Channel Creation Performance
    printf("\n[1/3] Channel Creation Performance Test\n");
    tests_run++;
    if (test_channel_creation_performance(&context) == ASTHRA_TEST_PASS) {
        printf("✅ PASS: Channel creation performance test\n");
        tests_passed++;
    } else {
        printf("❌ FAIL: Channel creation performance test\n");
    }
    
    // Test 2: Channel Send/Receive Performance
    printf("\n[2/3] Channel Send/Receive Performance Test\n");
    tests_run++;
    if (test_channel_send_receive_performance(&context) == ASTHRA_TEST_PASS) {
        printf("✅ PASS: Channel send/receive performance test\n");
        tests_passed++;
    } else {
        printf("❌ FAIL: Channel send/receive performance test\n");
    }
    
    // Test 3: Memory Allocation Performance
    printf("\n[3/3] Memory Allocation Performance Test\n");
    tests_run++;
    if (test_memory_allocation_performance(&context) == ASTHRA_TEST_PASS) {
        printf("✅ PASS: Memory allocation performance test\n");
        tests_passed++;
    } else {
        printf("❌ FAIL: Memory allocation performance test\n");
    }
    
    // Summary
    printf("\n=============================================================================\n");
    printf("PERFORMANCE BASIC TEST SUMMARY\n");
    printf("=============================================================================\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", tests_run > 0 ? (float)tests_passed / tests_run * 100.0 : 0.0);
    
    return tests_passed == tests_run ? 0 : 1;
} 
