/*
 * Stress Testing and Advanced Benchmarks
 * Comprehensive stress testing for concurrency system under heavy load
 *
 * Phase 8: Testing and Validation
 * Focus: System stress testing, bottleneck identification, and edge case performance
 */

#include "test_concurrency_benchmarks_common.h"

// ============================================================================
// Stress Tests
// ============================================================================

void test_stress_many_channels(void) {
    printf("Testing stress with many concurrent channels...\n");

    const int NUM_CHANNELS = 1000;
    const int MESSAGES_PER_CHANNEL = 100;

    AsthraConcurrencyChannelHandle *channels[NUM_CHANNELS];
    pthread_t threads[NUM_CHANNELS];

    Timer timer;
    timer_start(&timer);

    // Create channels and start producers
    for (int i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = asthra_channel_create(10);
        ASSERT_TRUE(channels[i] != NULL);

        int *channel_id = malloc(sizeof(int));
        *channel_id = i;

        pthread_create(
            &threads[i], NULL,
            ^void *(void *arg) {
              int id = *(int *)arg;
              free(arg);

              for (int j = 0; j < MESSAGES_PER_CHANNEL; j++) {
                  int message = id * 1000 + j;
                  AsthraConcurrencyResult result =
                      asthra_channel_send(channels[id], &message, sizeof(int));
                  if (result != ASTHRA_CONCURRENCY_SUCCESS) {
                      return (void *)(intptr_t)result;
                  }
              }

              asthra_channel_close(channels[id]);
              return (void *)(intptr_t)ASTHRA_CONCURRENCY_SUCCESS;
            },
            channel_id);
    }

    // Wait for all producers and consume messages
    int total_consumed = 0;
    for (int i = 0; i < NUM_CHANNELS; i++) {
        pthread_join(threads[i], NULL);

        // Consume all messages from this channel
        while (1) {
            int message;
            AsthraConcurrencyResult result =
                asthra_channel_recv(channels[i], &message, sizeof(int));

            if (result == ASTHRA_CONCURRENCY_SUCCESS) {
                total_consumed++;
            } else {
                break;
            }
        }

        asthra_channel_destroy(channels[i]);
    }

    double elapsed_ms = timer_stop(&timer);

    printf("  Stress test: %d channels, %d total messages in %.2f ms\n", NUM_CHANNELS,
           total_consumed, elapsed_ms);

    ASSERT_TRUE(total_consumed == NUM_CHANNELS * MESSAGES_PER_CHANNEL);

    double throughput = (total_consumed / elapsed_ms) * 1000.0;
    printf("  Stress throughput: %.0f messages/second\n", throughput);

    // Should maintain reasonable throughput under stress
    ASSERT_TRUE(throughput >= 10000.0);
}

// ============================================================================
// Advanced Stress Test Functions
// ============================================================================

void test_stress_mixed_operations(void) {
    printf("Testing stress with mixed concurrency operations...\n");

    const int NUM_OPERATIONS = 5000;
    const int NUM_WORKER_THREADS = 8;

    // Create mix of concurrency primitives
    AsthraConcurrencyChannelHandle *ch = asthra_channel_create(100);
    AsthraConcurrencyBarrierHandle *barrier = asthra_barrier_create(NUM_WORKER_THREADS);
    AsthraConcurrencySemaphoreHandle *semaphore = asthra_semaphore_create(4);

    ASSERT_TRUE(ch != NULL);
    ASSERT_TRUE(barrier != NULL);
    ASSERT_TRUE(semaphore != NULL);

    pthread_t threads[NUM_WORKER_THREADS];
    int thread_results[NUM_WORKER_THREADS];

    Timer timer;
    timer_start(&timer);

    // Start worker threads performing mixed operations
    for (int i = 0; i < NUM_WORKER_THREADS; i++) {
        int *thread_id = malloc(sizeof(int));
        *thread_id = i;

        pthread_create(
            &threads[i], NULL,
            ^void *(void *arg) {
              int id = *(int *)arg;
              free(arg);

              int operations_completed = 0;

              for (int op = 0; op < NUM_OPERATIONS / NUM_WORKER_THREADS; op++) {
                  // Rotate through different operation types
                  int operation_type = (id + op) % 4;

                  switch (operation_type) {
                  case 0: {
                      // Channel send/recv
                      int data = id * 1000 + op;
                      if (asthra_channel_send(ch, &data, sizeof(int)) ==
                          ASTHRA_CONCURRENCY_SUCCESS) {
                          int received;
                          if (asthra_channel_recv(ch, &received, sizeof(int)) ==
                              ASTHRA_CONCURRENCY_SUCCESS) {
                              operations_completed++;
                          }
                      }
                      break;
                  }
                  case 1: {
                      // Semaphore acquire/release
                      if (asthra_semaphore_acquire(semaphore) == ASTHRA_CONCURRENCY_SUCCESS) {
                          usleep(10); // Small critical section
                          if (asthra_semaphore_release(semaphore) == ASTHRA_CONCURRENCY_SUCCESS) {
                              operations_completed++;
                          }
                      }
                      break;
                  }
                  case 2: {
                      // Barrier wait (every 10th operation)
                      if (op % 10 == 0) {
                          bool is_leader;
                          if (asthra_barrier_wait(barrier, &is_leader) ==
                              ASTHRA_CONCURRENCY_SUCCESS) {
                              operations_completed++;
                          }
                      } else {
                          operations_completed++; // Count as completed
                      }
                      break;
                  }
                  case 3: {
                      // Mixed channel operations with timeout simulation
                      int data = id * 2000 + op;
                      if (asthra_channel_send(ch, &data, sizeof(int)) ==
                          ASTHRA_CONCURRENCY_SUCCESS) {
                          operations_completed++;
                      }
                      break;
                  }
                  }

                  // Small delay to allow interleaving
                  if (op % 100 == 0) {
                      usleep(100);
                  }
              }

              return (void *)(intptr_t)operations_completed;
            },
            thread_id);
    }

    // Wait for all threads and collect results
    int total_operations = 0;
    for (int i = 0; i < NUM_WORKER_THREADS; i++) {
        void *result;
        pthread_join(threads[i], &result);
        thread_results[i] = (int)(intptr_t)result;
        total_operations += thread_results[i];
    }

    double elapsed_ms = timer_stop(&timer);

    printf("  Mixed operations stress test: %d total operations in %.2f ms\n", total_operations,
           elapsed_ms);

    double ops_per_second = (total_operations / elapsed_ms) * 1000.0;
    printf("  Mixed operations throughput: %.0f operations/second\n", ops_per_second);

    // Should complete most operations successfully
    ASSERT_TRUE(total_operations >= NUM_OPERATIONS * 0.8); // At least 80% success
    ASSERT_TRUE(ops_per_second >= 1000.0);                 // At least 1K ops/sec

    // Cleanup
    asthra_channel_destroy(ch);
    asthra_barrier_destroy(barrier);
    asthra_semaphore_destroy(semaphore);
}

void test_stress_resource_exhaustion(void) {
    printf("Testing stress with resource exhaustion scenarios...\n");

    const int MAX_RESOURCES = 100;
    Timer timer;
    timer_start(&timer);

    // Test channel resource limits
    AsthraConcurrencyChannelHandle *channels[MAX_RESOURCES];
    int channels_created = 0;

    for (int i = 0; i < MAX_RESOURCES; i++) {
        channels[i] = asthra_channel_create(1000); // Large buffer
        if (channels[i] != NULL) {
            channels_created++;
        } else {
            break;
        }
    }

    printf("  Created %d channels before resource exhaustion\n", channels_created);
    ASSERT_TRUE(channels_created >= 50); // Should create at least 50 channels

    // Test rapid creation/destruction
    for (int cycle = 0; cycle < 10; cycle++) {
        for (int i = 0; i < channels_created; i++) {
            asthra_channel_destroy(channels[i]);
            channels[i] = asthra_channel_create(100);
            ASSERT_TRUE(channels[i] != NULL);
        }
    }

    // Cleanup all channels
    for (int i = 0; i < channels_created; i++) {
        asthra_channel_destroy(channels[i]);
    }

    double elapsed_ms = timer_stop(&timer);
    printf("  Resource exhaustion test completed in %.2f ms\n", elapsed_ms);

    // Test should complete in reasonable time
    ASSERT_TRUE(elapsed_ms < 5000.0); // Less than 5 seconds
}

void test_stress_high_contention(void) {
    printf("Testing stress with high contention scenarios...\n");

    const int NUM_CONTENDING_THREADS = 16;
    const int CONTENTION_OPERATIONS = 1000;

    // Single shared resources for maximum contention
    AsthraConcurrencyChannelHandle *shared_channel = asthra_channel_create(1); // Single slot
    AsthraConcurrencySemaphoreHandle *shared_semaphore =
        asthra_semaphore_create(1); // Single permit

    ASSERT_TRUE(shared_channel != NULL);
    ASSERT_TRUE(shared_semaphore != NULL);

    pthread_t threads[NUM_CONTENDING_THREADS];
    int successful_operations[NUM_CONTENDING_THREADS];

    Timer timer;
    timer_start(&timer);

    // Start highly contending threads
    for (int i = 0; i < NUM_CONTENDING_THREADS; i++) {
        int *thread_id = malloc(sizeof(int));
        *thread_id = i;

        pthread_create(
            &threads[i], NULL,
            ^void *(void *arg) {
              int id = *(int *)arg;
              free(arg);

              int success_count = 0;

              for (int op = 0; op < CONTENTION_OPERATIONS; op++) {
                  bool operation_succeeded = false;

                  // Alternate between semaphore and channel operations
                  if (op % 2 == 0) {
                      // High contention semaphore operations
                      if (asthra_semaphore_acquire(shared_semaphore) ==
                          ASTHRA_CONCURRENCY_SUCCESS) {
                          // Very brief critical section
                          volatile int dummy = id + op;
                          (void)dummy;

                          if (asthra_semaphore_release(shared_semaphore) ==
                              ASTHRA_CONCURRENCY_SUCCESS) {
                              operation_succeeded = true;
                          }
                      }
                  } else {
                      // High contention channel operations
                      int data = id * 10000 + op;
                      if (asthra_channel_send(shared_channel, &data, sizeof(int)) ==
                          ASTHRA_CONCURRENCY_SUCCESS) {
                          int received;
                          if (asthra_channel_recv(shared_channel, &received, sizeof(int)) ==
                              ASTHRA_CONCURRENCY_SUCCESS) {
                              operation_succeeded = true;
                          }
                      }
                  }

                  if (operation_succeeded) {
                      success_count++;
                  }

                  // Yield to increase contention
                  if (op % 10 == 0) {
                      usleep(1);
                  }
              }

              return (void *)(intptr_t)success_count;
            },
            thread_id);
    }

    // Wait for all threads
    int total_successful = 0;
    for (int i = 0; i < NUM_CONTENDING_THREADS; i++) {
        void *result;
        pthread_join(threads[i], &result);
        successful_operations[i] = (int)(intptr_t)result;
        total_successful += successful_operations[i];
    }

    double elapsed_ms = timer_stop(&timer);

    printf("  High contention test: %d successful operations from %d total attempts\n",
           total_successful, NUM_CONTENDING_THREADS * CONTENTION_OPERATIONS);
    printf("  Success rate: %.1f%% in %.2f ms\n",
           (float)total_successful / (NUM_CONTENDING_THREADS * CONTENTION_OPERATIONS) * 100.0,
           elapsed_ms);

    // Should handle contention gracefully with reasonable success rate
    float success_rate = (float)total_successful / (NUM_CONTENDING_THREADS * CONTENTION_OPERATIONS);
    ASSERT_TRUE(success_rate >= 0.7); // At least 70% success under high contention

    double ops_per_second = (total_successful / elapsed_ms) * 1000.0;
    printf("  Contended operations throughput: %.0f operations/second\n", ops_per_second);

    // Should maintain some throughput even under contention
    ASSERT_TRUE(ops_per_second >= 500.0);

    // Cleanup
    asthra_channel_destroy(shared_channel);
    asthra_semaphore_destroy(shared_semaphore);
}
