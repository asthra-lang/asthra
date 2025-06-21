/*
 * Performance Category - Minimal Test
 * Demonstrates correct runtime API usage for performance testing
 * 
 * This test uses the proven minimal framework approach to create
 * a working performance test with correct function names and types.
 */

#include "../framework/test_framework.h"
#include "../../runtime/asthra_concurrency_bridge_modular.h"
#include "../../runtime/concurrency/asthra_concurrency_channels.h"
#include "../../runtime/concurrency/asthra_concurrency_sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

// ============================================================================
// Timing Utilities
// ============================================================================

typedef struct {
    struct timeval start;
    struct timeval end;
} Timer;

// ============================================================================
// Function Prototypes
// ============================================================================

static void timer_start(Timer* timer);
static double timer_stop(Timer* timer);
static void test_channel_performance_minimal(void);
static void test_barrier_performance_minimal(void);
static void test_semaphore_performance_minimal(void);

// ============================================================================
// Test Framework Integration
// ============================================================================

size_t tests_run = 0;
size_t tests_passed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("  ✅ PASS: %s\n", message); \
        } else { \
            printf("  ❌ FAIL: %s\n", message); \
        } \
    } while(0)

static void timer_start(Timer* timer) {
    gettimeofday(&timer->start, NULL);
}

static double timer_stop(Timer* timer) {
    gettimeofday(&timer->end, NULL);
    return (timer->end.tv_sec - timer->start.tv_sec) * 1000.0 + 
           (timer->end.tv_usec - timer->start.tv_usec) / 1000.0;
}

// ============================================================================
// Performance Tests Using Correct API
// ============================================================================

static void test_channel_performance_minimal(void) {
    printf("Testing minimal channel performance...\n");
    
    // Use correct API: Asthra_channel_create with 3 parameters
    AsthraConcurrencyChannel* channel = Asthra_channel_create(sizeof(int), 100, "test_channel");
    TEST_ASSERT(channel != NULL, "Channel creation should succeed");
    
    if (!channel) return;
    
    Timer timer;
    timer_start(&timer);
    
    // Test basic send/receive operations
    const int num_operations = 1000;
    int successful_operations = 0;
    
    for (int i = 0; i < num_operations; i++) {
        int data = i;
        
        // Use correct API: Asthra_channel_send with timeout parameter
        AsthraResult send_result = Asthra_channel_send(channel, &data, 0);
        if (send_result.tag == ASTHRA_RESULT_OK) {
            
            int received_data;
            // Use correct API: Asthra_channel_recv with timeout parameter
            AsthraResult recv_result = Asthra_channel_recv(channel, &received_data, 0);
            if (recv_result.tag == ASTHRA_RESULT_OK && received_data == data) {
                successful_operations++;
            }
        }
    }
    
    double elapsed_ms = timer_stop(&timer);
    
    TEST_ASSERT(successful_operations > 0, "Some operations should succeed");
    TEST_ASSERT(elapsed_ms > 0, "Timer should measure elapsed time");
    
    printf("  📊 Performance: %d operations in %.2f ms (%.2f ops/ms)\n", 
           successful_operations, elapsed_ms, successful_operations / elapsed_ms);
    
    // Use correct API: Asthra_channel_destroy instead of asthra_channel_destroy
    Asthra_channel_destroy(channel);
}

static void test_barrier_performance_minimal(void) {
    printf("Testing minimal barrier performance...\n");
    
    // Test just barrier creation and destruction first
    printf("  🔍 Testing barrier creation...\n");
    AsthraConcurrencyBarrier* barrier = Asthra_barrier_create(1);
    TEST_ASSERT(barrier != NULL, "Barrier creation should succeed");
    
    if (barrier) {
        printf("  🔍 Testing barrier destruction...\n");
        Asthra_barrier_destroy(barrier);
        printf("  ✅ Barrier creation/destruction works\n");
    }
    
    // Performance test: measure barrier creation/destruction
    Timer timer;
    timer_start(&timer);
    
    const int num_operations = 100;
    int successful_operations = 0;
    
    for (int i = 0; i < num_operations; i++) {
        AsthraConcurrencyBarrier* temp_barrier = Asthra_barrier_create(1);
        if (temp_barrier) {
            successful_operations++;
            Asthra_barrier_destroy(temp_barrier);
        }
    }
    
    double elapsed_ms = timer_stop(&timer);
    
    TEST_ASSERT(successful_operations > 0, "Some barrier operations should succeed");
    TEST_ASSERT(elapsed_ms > 0, "Timer should measure elapsed time");
    
    printf("  📊 Performance: %d barrier create/destroy operations in %.2f ms (%.2f ops/ms)\n", 
           successful_operations, elapsed_ms, successful_operations / elapsed_ms);
}

static void test_semaphore_performance_minimal(void) {
    printf("Testing minimal semaphore performance...\n");
    
    // Use correct API: Asthra_semaphore_create instead of asthra_semaphore_create
    AsthraConcurrencySemaphore* semaphore = Asthra_semaphore_create(10);
    TEST_ASSERT(semaphore != NULL, "Semaphore creation should succeed");
    
    if (!semaphore) return;
    
    Timer timer;
    timer_start(&timer);
    
    // Test semaphore acquire/release operations
    const int num_operations = 1000;
    int successful_operations = 0;
    
    for (int i = 0; i < num_operations; i++) {
        // Use correct API: Asthra_semaphore_acquire
        AsthraResult acquire_result = Asthra_semaphore_acquire(semaphore);
        if (acquire_result.tag == ASTHRA_RESULT_OK) {
            
            // Use correct API: Asthra_semaphore_release
            AsthraResult release_result = Asthra_semaphore_release(semaphore);
            if (release_result.tag == ASTHRA_RESULT_OK) {
                successful_operations++;
            }
        }
    }
    
    double elapsed_ms = timer_stop(&timer);
    
    TEST_ASSERT(successful_operations > 0, "Some semaphore operations should succeed");
    TEST_ASSERT(elapsed_ms > 0, "Timer should measure elapsed time");
    
    printf("  📊 Performance: %d semaphore operations in %.2f ms (%.2f ops/ms)\n", 
           successful_operations, elapsed_ms, successful_operations / elapsed_ms);
    
    // Use correct API: Asthra_semaphore_destroy instead of asthra_semaphore_destroy
    Asthra_semaphore_destroy(semaphore);
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main(void) {
    printf("🚀 Performance Category - Minimal Test Suite\n");
    printf("==============================================\n\n");
    
    // Initialize runtime if needed
    printf("Initializing performance test runtime...\n");
    
    // Run performance tests
    test_channel_performance_minimal();
    test_barrier_performance_minimal();
    test_semaphore_performance_minimal();
    
    // Print results
    printf("\n==============================================\n");
    printf("📊 Performance Test Results:\n");
    printf("   Tests run: %zu\n", tests_run);
    printf("   Tests passed: %zu\n", tests_passed);
    printf("   Success rate: %.1f%%\n", tests_run > 0 ? (float)tests_passed / tests_run * 100.0 : 0.0);
    
    if (tests_passed == tests_run && tests_run > 0) {
        printf("🎉 All performance tests PASSED!\n");
        return 0;
    } else {
        printf("❌ Some performance tests FAILED\n");
        return 1;
    }
} 
