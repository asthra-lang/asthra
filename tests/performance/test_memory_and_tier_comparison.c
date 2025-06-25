/*
 * Memory Efficiency and Tier Comparison Tests
 * Performance testing for memory usage and tier performance comparison
 *
 * Phase 8: Testing and Validation
 * Focus: Memory efficiency validation and tier 1 vs tier 2 performance analysis
 */

#include "test_concurrency_benchmarks_common.h"

// ============================================================================
// Memory Performance Tests
// ============================================================================

void test_memory_efficiency(void) {
    printf("Testing memory efficiency of concurrency primitives...\n");

    AsthraConcurrencyStats initial_stats;
    AsthraConcurrencyResult result = asthra_concurrency_get_stats(&initial_stats);
    ASSERT_TRUE(result == ASTHRA_CONCURRENCY_SUCCESS);

    uint64_t initial_memory = initial_stats.memory_usage_bytes;

    // Create many concurrent objects
    const int NUM_CHANNELS = 1000;
    const int NUM_BARRIERS = 100;
    const int NUM_SEMAPHORES = 100;

    AsthraConcurrencyChannelHandle *channels[NUM_CHANNELS];
    AsthraConcurrencyBarrierHandle *barriers[NUM_BARRIERS];
    AsthraConcurrencySemaphoreHandle *semaphores[NUM_SEMAPHORES];

    Timer timer;
    timer_start(&timer);

    // Create channels
    for (int i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = asthra_channel_create(10);
        ASSERT_TRUE(channels[i] != NULL);
    }

    // Create barriers
    for (int i = 0; i < NUM_BARRIERS; i++) {
        barriers[i] = asthra_barrier_create(4);
        ASSERT_TRUE(barriers[i] != NULL);
    }

    // Create semaphores
    for (int i = 0; i < NUM_SEMAPHORES; i++) {
        semaphores[i] = asthra_semaphore_create(10);
        ASSERT_TRUE(semaphores[i] != NULL);
    }

    double creation_time = timer_stop(&timer);

    // Check memory usage
    AsthraConcurrencyStats current_stats;
    result = asthra_concurrency_get_stats(&current_stats);
    ASSERT_TRUE(result == ASTHRA_CONCURRENCY_SUCCESS);

    uint64_t peak_memory = current_stats.memory_usage_bytes;
    uint64_t memory_increase = peak_memory - initial_memory;

    printf("  Memory usage: %llu bytes for %d objects (%.2f KB total, %.2f KB creation time)\n",
           memory_increase, NUM_CHANNELS + NUM_BARRIERS + NUM_SEMAPHORES, memory_increase / 1024.0,
           creation_time);

    // Memory per object should be reasonable
    double bytes_per_object =
        (double)memory_increase / (NUM_CHANNELS + NUM_BARRIERS + NUM_SEMAPHORES);
    printf("  Average memory per object: %.2f bytes\n", bytes_per_object);

    // Should use less than 1KB per object on average
    ASSERT_TRUE(bytes_per_object < 1024.0);

    // Clean up
    timer_start(&timer);

    for (int i = 0; i < NUM_CHANNELS; i++) {
        asthra_channel_destroy(channels[i]);
    }

    for (int i = 0; i < NUM_BARRIERS; i++) {
        asthra_barrier_destroy(barriers[i]);
    }

    for (int i = 0; i < NUM_SEMAPHORES; i++) {
        asthra_semaphore_destroy(semaphores[i]);
    }

    double cleanup_time = timer_stop(&timer);
    printf("  Cleanup time: %.2f ms\n", cleanup_time);

    // Check final memory usage
    result = asthra_concurrency_get_stats(&current_stats);
    ASSERT_TRUE(result == ASTHRA_CONCURRENCY_SUCCESS);

    uint64_t final_memory = current_stats.memory_usage_bytes;
    printf("  Final memory usage: %llu bytes (leaked: %lld bytes)\n", final_memory,
           (int64_t)(final_memory - initial_memory));

    // Memory leak should be minimal
    ASSERT_TRUE((int64_t)(final_memory - initial_memory) < 1024); // Less than 1KB leaked
}

// ============================================================================
// Tier Comparison Tests
// ============================================================================

void test_tier1_vs_tier2_performance(void) {
    printf("Testing Tier 1 vs Tier 2 performance comparison...\n");

    const int NUM_TASKS = 1000;
    const int WORK_ITERATIONS = 1000;

    // Simulate Tier 1 performance (spawn_with_handle + await)
    Timer tier1_timer;
    timer_start(&tier1_timer);

    // Simulate parallel task execution
    pthread_t tier1_threads[4];
    for (int i = 0; i < 4; i++) {
        int *thread_data = malloc(sizeof(int));
        *thread_data = NUM_TASKS / 4;

        pthread_create(
            &tier1_threads[i], NULL,
            ^void *(void *arg) {
              int task_count = *(int *)arg;
              free(arg);

              for (int j = 0; j < task_count; j++) {
                  // Simulate compute work
                  volatile int sum = 0;
                  for (int k = 0; k < WORK_ITERATIONS; k++) {
                      sum += k;
                  }
              }

              return NULL;
            },
            thread_data);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(tier1_threads[i], NULL);
    }

    double tier1_time = timer_stop(&tier1_timer);

    // Simulate Tier 2 performance (channels + coordination)
    Timer tier2_timer;
    timer_start(&tier2_timer);

    AsthraConcurrencyChannelHandle *work_channel = asthra_channel_create(100);
    AsthraConcurrencyChannelHandle *result_channel = asthra_channel_create(100);

    // Producer thread
    pthread_t producer;
    pthread_create(
        &producer, NULL,
        ^void *(void *arg) {
          for (int i = 0; i < NUM_TASKS; i++) {
              asthra_channel_send(work_channel, &i, sizeof(int));
          }
          asthra_channel_close(work_channel);
          return NULL;
        },
        NULL);

    // Worker threads
    pthread_t tier2_threads[4];
    for (int i = 0; i < 4; i++) {
        pthread_create(
            &tier2_threads[i], NULL,
            ^void *(void *arg) {
              while (1) {
                  int task;
                  AsthraConcurrencyResult result =
                      asthra_channel_recv(work_channel, &task, sizeof(int));

                  if (result == ASTHRA_CONCURRENCY_ERROR_CHANNEL_CLOSED) {
                      break;
                  } else if (result == ASTHRA_CONCURRENCY_SUCCESS) {
                      // Simulate compute work
                      volatile int sum = 0;
                      for (int k = 0; k < WORK_ITERATIONS; k++) {
                          sum += k;
                      }

                      asthra_channel_send(result_channel, &sum, sizeof(int));
                  }
              }
              return NULL;
            },
            NULL);
    }

    // Wait for completion
    pthread_join(producer, NULL);
    for (int i = 0; i < 4; i++) {
        pthread_join(tier2_threads[i], NULL);
    }

    // Collect results
    int collected = 0;
    while (collected < NUM_TASKS) {
        int result;
        if (asthra_channel_recv(result_channel, &result, sizeof(int)) ==
            ASTHRA_CONCURRENCY_SUCCESS) {
            collected++;
        } else {
            break;
        }
    }

    double tier2_time = timer_stop(&tier2_timer);

    printf("  Tier 1 time: %.2f ms\n", tier1_time);
    printf("  Tier 2 time: %.2f ms\n", tier2_time);
    printf("  Overhead ratio: %.2fx\n", tier2_time / tier1_time);

    // Tier 2 should not be more than 3x slower than Tier 1
    ASSERT_TRUE(tier2_time / tier1_time < 3.0);

    asthra_channel_destroy(work_channel);
    asthra_channel_destroy(result_channel);
}
