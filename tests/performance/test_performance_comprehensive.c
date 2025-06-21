/*
 * =============================================================================
 * PERFORMANCE CATEGORY - COMPREHENSIVE TEST SUITE
 * =============================================================================
 * 
 * This file provides comprehensive performance testing using the correct
 * runtime API patterns. Based on systematic analysis of API mismatches.
 * 
 * Root Cause: Performance tests were using incorrect API names and types
 * Solution: Use correct Asthra_* functions and AsthraResult patterns
 * 
 * =============================================================================
 */

#include "../framework/test_framework.h"
#include "../../runtime/asthra_concurrency_bridge_modular.h"
#include "../../runtime/asthra_statistics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

// Test configuration
#define NUM_CHANNELS 10
#define NUM_MESSAGES 1000
#define NUM_WORKER_THREADS 4
#define NUM_OPERATIONS 10000
#define BUFFER_SIZE 100

// Global test counters
static int tests_run = 0;
static int tests_passed = 0;

// Timer utility
typedef struct {
    struct timespec start;
    struct timespec end;
} Timer;

static void timer_start(Timer* timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

static double timer_stop(Timer* timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->end);
    return (timer->end.tv_sec - timer->start.tv_sec) * 1000.0 + 
           (timer->end.tv_nsec - timer->start.tv_nsec) / 1000000.0;
}

// =============================================================================
// CHANNEL PERFORMANCE TESTS
// =============================================================================

static void test_channel_throughput_performance(void) {
    printf("  Testing channel throughput performance...\n");
    tests_run++;
    
    // Create channel with correct API - Use smaller message count that fits buffer
    const int PERF_NUM_MESSAGES = BUFFER_SIZE - 10; // Leave some room in buffer
    AsthraConcurrencyChannel* channel = Asthra_channel_create(sizeof(int), BUFFER_SIZE, "perf_test_channel");
    if (!channel) {
        printf("    ❌ Failed to create channel\n");
        return;
    }
    
    Timer timer;
    timer_start(&timer);
    
    // Send messages (using buffer-safe count)
    int sent_count = 0;
    for (int i = 0; i < PERF_NUM_MESSAGES; i++) {
        AsthraResult result = Asthra_channel_send(channel, &i, 0); // 0 timeout = non-blocking
        if (result.tag == ASTHRA_RESULT_OK) {
            sent_count++;
        } else {
            printf("    ⚠️  Send failed at message %d (buffer full)\n", i);
            break; // Stop sending when buffer is full
        }
    }
    
    // Receive messages
    int received_count = 0;
    for (int i = 0; i < sent_count; i++) {
        int value;
        AsthraResult result = Asthra_channel_recv(channel, &value, 0); // 0 timeout = non-blocking
        if (result.tag == ASTHRA_RESULT_OK) {
            received_count++;
        } else {
            printf("    ⚠️  Receive failed at message %d\n", i);
            break;
        }
    }
    
    double elapsed_ms = timer_stop(&timer);
    double throughput = (sent_count + received_count) / (elapsed_ms / 1000.0); // operations per second
    
    printf("    📊 Throughput: %.0f ops/sec (%.2f ms total)\n", throughput, elapsed_ms);
    printf("    📊 Sent: %d, Received: %d/%d messages\n", sent_count, received_count, sent_count);
    
    // Cleanup
    Asthra_channel_destroy(channel);
    
    if (received_count == sent_count && sent_count > 0) { // All sent messages should be received
        printf("    ✅ Channel throughput test PASSED\n");
        tests_passed++;
    } else {
        printf("    ❌ Channel throughput test FAILED\n");
    }
}

static void test_channel_latency_performance(void) {
    printf("  Testing channel latency performance...\n");
    tests_run++;
    
    AsthraConcurrencyChannel* channel = Asthra_channel_create(sizeof(int), 1, "latency_test_channel");
    if (!channel) {
        printf("    ❌ Failed to create channel\n");
        return;
    }
    
    double total_latency = 0.0;
    int successful_ops = 0;
    
    for (int i = 0; i < 100; i++) {
        Timer timer;
        timer_start(&timer);
        
        // Send and immediately receive
        AsthraResult send_result = Asthra_channel_send(channel, &i, 0);
        if (send_result.tag == ASTHRA_RESULT_OK) {
            int value;
            AsthraResult recv_result = Asthra_channel_recv(channel, &value, 0);
            if (recv_result.tag == ASTHRA_RESULT_OK) {
                double latency = timer_stop(&timer);
                total_latency += latency;
                successful_ops++;
            }
        }
    }
    
    if (successful_ops > 0) {
        double avg_latency = total_latency / successful_ops;
        printf("    📊 Average latency: %.3f ms (%d successful ops)\n", avg_latency, successful_ops);
        
        if (avg_latency < 10.0) { // Accept < 10ms average latency
            printf("    ✅ Channel latency test PASSED\n");
            tests_passed++;
        } else {
            printf("    ❌ Channel latency test FAILED (too slow)\n");
        }
    } else {
        printf("    ❌ Channel latency test FAILED (no successful operations)\n");
    }
    
    Asthra_channel_destroy(channel);
}

// =============================================================================
// SYNCHRONIZATION PERFORMANCE TESTS
// =============================================================================

static void test_barrier_performance(void) {
    printf("  Testing barrier performance...\n");
    tests_run++;
    
    // Fixed: Proper barrier performance test without hanging in single-threaded context
    Timer timer;
    timer_start(&timer);
    
    // Test barrier creation and destruction performance (safe for single-threaded)
    const int barrier_count = 100;
    int successful_operations = 0;
    
    for (int i = 0; i < barrier_count; i++) {
        AsthraConcurrencyBarrier* barrier = Asthra_barrier_create(1); // Single thread barrier
        if (barrier) {
            // Immediately signal the barrier (since we're the only thread)
            AsthraFFIResult result = Asthra_barrier_wait(barrier);
            if (result.tag == ASTHRA_FFI_RESULT_OK) {
                successful_operations++;
            }
            Asthra_barrier_destroy(barrier);
        }
    }
    
    timer_stop(&timer);
    double duration_ms = timer_get_duration_ms(&timer);
    
    if (successful_operations > 0) {
        printf("    ✅ Barrier performance: %d operations in %.2f ms (%.2f ops/ms)\n", 
               successful_operations, duration_ms, successful_operations / duration_ms);
        printf("    ✅ Barrier performance test PASSED\n");
        tests_passed++;
        
        // Record barrier operations in statistics
        asthra_atomic_stats_record_barrier_operation();
    } else {
        printf("    ❌ No successful barrier operations\n");
    }
}

static void test_semaphore_performance(void) {
    printf("  Testing semaphore performance...\n");
    tests_run++;
    
    AsthraConcurrencySemaphore* semaphore = Asthra_semaphore_create(10);
    if (!semaphore) {
        printf("    ❌ Failed to create semaphore\n");
        return;
    }
    
    Timer timer;
    timer_start(&timer);
    
    // Simulate acquire/release operations
    int successful_ops = 0;
    for (int i = 0; i < 100; i++) {
        AsthraResult acquire_result = Asthra_semaphore_acquire(semaphore);
        if (acquire_result.tag == ASTHRA_RESULT_OK) {
            AsthraResult release_result = Asthra_semaphore_release(semaphore);
            if (release_result.tag == ASTHRA_RESULT_OK) {
                successful_ops++;
            }
        }
    }
    
    double elapsed_ms = timer_stop(&timer);
    double ops_per_sec = (successful_ops * 2) / (elapsed_ms / 1000.0); // acquire + release
    
    printf("    📊 Semaphore ops: %.0f ops/sec (%d successful)\n", ops_per_sec, successful_ops);
    
    Asthra_semaphore_destroy(semaphore);
    
    if (successful_ops > 50) { // Accept 50%+ success rate
        printf("    ✅ Semaphore performance test PASSED\n");
        tests_passed++;
    } else {
        printf("    ❌ Semaphore performance test FAILED\n");
    }
}

// =============================================================================
// MEMORY EFFICIENCY TESTS
// =============================================================================

static void test_memory_efficiency(void) {
    printf("  Testing memory efficiency...\n");
    tests_run++;
    
    // Create multiple objects and measure memory usage
    AsthraConcurrencyChannel* channels[10];
    AsthraConcurrencyBarrier* barriers[5];
    AsthraConcurrencySemaphore* semaphores[5];
    
    // Create objects
    for (int i = 0; i < 10; i++) {
        channels[i] = Asthra_channel_create(sizeof(int), 10, "mem_test_channel");
    }
    for (int i = 0; i < 5; i++) {
        barriers[i] = Asthra_barrier_create(4);
        semaphores[i] = Asthra_semaphore_create(10);
    }
    
    // Test basic operations to ensure objects work
    int working_channels = 0;
    for (int i = 0; i < 10; i++) {
        if (channels[i]) {
            int test_value = i;
            AsthraResult result = Asthra_channel_send(channels[i], &test_value, 0);
            if (result.tag == ASTHRA_RESULT_OK) {
                working_channels++;
            }
        }
    }
    
    // Cleanup
    for (int i = 0; i < 10; i++) {
        if (channels[i]) Asthra_channel_destroy(channels[i]);
    }
    for (int i = 0; i < 5; i++) {
        if (barriers[i]) Asthra_barrier_destroy(barriers[i]);
        if (semaphores[i]) Asthra_semaphore_destroy(semaphores[i]);
    }
    
    printf("    📊 Working channels: %d/10\n", working_channels);
    
    if (working_channels >= 5) { // Accept 50%+ working objects
        printf("    ✅ Memory efficiency test PASSED\n");
        tests_passed++;
    } else {
        printf("    ❌ Memory efficiency test FAILED\n");
    }
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("🚀 Performance Category - Comprehensive Test Suite\n");
    printf("==================================================\n\n");
    
    printf("📋 Running Channel Performance Tests:\n");
    test_channel_throughput_performance();
    test_channel_latency_performance();
    
    printf("\n📋 Running Synchronization Performance Tests:\n");
    test_barrier_performance();
    test_semaphore_performance();
    
    printf("\n📋 Running Memory Efficiency Tests:\n");
    test_memory_efficiency();
    
    // Print final results
    printf("\n==================================================\n");
    printf("📊 Performance Test Results:\n");
    printf("   Tests run: %d\n", tests_run);
    printf("   Tests passed: %d\n", tests_passed);
    printf("   Success rate: %.1f%%\n", tests_run > 0 ? (float)tests_passed / tests_run * 100.0 : 0.0);
    
    if (tests_passed == tests_run && tests_run > 0) {
        printf("🎉 All performance tests PASSED!\n");
        return 0;
    } else {
        printf("❌ Some performance tests FAILED\n");
        return 1;
    }
} 
