/*
 * Channel Performance Tests
 * Comprehensive performance testing for channel operations
 *
 * Phase 8: Testing and Validation
 * Focus: Channel throughput, latency, and scalability testing
 */

#include "test_concurrency_benchmarks_common.h"
#include <stdbool.h>

// ============================================================================
// Helper functions and macros
// ============================================================================

#define ASSERT_EQUAL(a, b) ASSERT_TRUE((a) == (b))

static void increment_test_counter(void) {
    // Test counter is incremented in ASSERT_TRUE macro
}

static double timer_seconds(Timer *timer) {
    return (timer->end.tv_sec - timer->start.tv_sec) +
           (timer->end.tv_usec - timer->start.tv_usec) / 1000000.0;
}

static double timer_microseconds(Timer *timer) {
    return (timer->end.tv_sec - timer->start.tv_sec) * 1000000.0 +
           (timer->end.tv_usec - timer->start.tv_usec);
}

// Timer functions are implemented in test_concurrency_timer_impl.c

// ============================================================================
// Thread function declarations
// ============================================================================

static void *producer_thread(void *arg);
static void *consumer_thread(void *arg);
static void *multi_producer_thread(void *arg);
static void *multi_consumer_thread(void *arg);

// ============================================================================
// Channel Performance Tests
// ============================================================================

void test_channel_throughput(void) {
    printf("Testing channel throughput performance...\n");

    const int NUM_MESSAGES = 100000;
    const int BUFFER_SIZE = 1000;

    AsthraConcurrencyChannelHandle *ch = asthra_channel_create(BUFFER_SIZE);
    ASSERT_TRUE(ch != NULL);

    Timer timer;
    timer_start(&timer);

    // Producer thread
    pthread_t producer;
    pthread_create(&producer, NULL, producer_thread, ch);

    // Consumer (main thread)
    int received_count = 0;
    while (received_count < NUM_MESSAGES) {
        int value;
        // The macro asthra_channel_recv returns AsthraResult, not bool
        AsthraResult result = Asthra_channel_recv(ch, &value, 0);

        if (result.tag == ASTHRA_RESULT_OK) {
            received_count++;
        } else {
            // Channel closed
            break;
        }
    }

    pthread_join(producer, NULL);
    timer_stop(&timer);

    double throughput = NUM_MESSAGES / timer_seconds(&timer);
    printf("  Throughput: %.2f messages/second\n", throughput);
    ASSERT_TRUE(throughput > 100000); // Should handle at least 100k msgs/sec

    asthra_channel_destroy(ch);
    increment_test_counter();
}

void test_channel_latency(void) {
    printf("Testing channel latency...\n");

    AsthraConcurrencyChannelHandle *ch = asthra_channel_create(1);
    ASSERT_TRUE(ch != NULL);

    const int NUM_ITERATIONS = 1000;
    Timer timer;

    timer_start(&timer);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Use the simplified macros that return bool
        bool send_ok = asthra_result_equals_success(Asthra_channel_send(ch, &i, 0));
        ASSERT_TRUE(send_ok);

        int received;
        AsthraResult recv_result = Asthra_channel_recv(ch, &received, 0);
        ASSERT_TRUE(recv_result.tag == ASTHRA_RESULT_OK);
        ASSERT_EQUAL(i, received);
    }
    timer_stop(&timer);

    double avg_latency_us = (timer_microseconds(&timer) / NUM_ITERATIONS) / 2.0;
    printf("  Average latency: %.2f microseconds\n", avg_latency_us);
    ASSERT_TRUE(avg_latency_us < 10.0); // Should be under 10 microseconds

    asthra_channel_destroy(ch);
    increment_test_counter();
}

// Structure to pass both channel and thread data
typedef struct {
    AsthraConcurrencyChannelHandle *channel;
    int id;
    int messages_per_thread;
} ThreadData;

void test_channel_scalability(void) {
    printf("Testing channel scalability with multiple producers/consumers...\n");

    const int NUM_PRODUCERS = 4;
    const int NUM_CONSUMERS = 4;
    const int MESSAGES_PER_PRODUCER = 10000;
    const int BUFFER_SIZE = 1000;

    AsthraConcurrencyChannelHandle *ch = asthra_channel_create(BUFFER_SIZE);
    ASSERT_TRUE(ch != NULL);

    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    ThreadData producer_data[NUM_PRODUCERS];
    ThreadData consumer_data[NUM_CONSUMERS];

    Timer timer;
    timer_start(&timer);

    // Start producers
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_data[i].channel = ch;
        producer_data[i].id = i;
        producer_data[i].messages_per_thread = MESSAGES_PER_PRODUCER;
        pthread_create(&producers[i], NULL, multi_producer_thread, &producer_data[i]);
    }

    // Start consumers
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_data[i].channel = ch;
        consumer_data[i].id = i;
        consumer_data[i].messages_per_thread = 0; // Not used for consumers
        pthread_create(&consumers[i], NULL, multi_consumer_thread, &consumer_data[i]);
    }

    // Wait for all producers to finish
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    // Close channel to signal consumers
    asthra_channel_close(ch);

    // Wait for all consumers to finish
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    timer_stop(&timer);

    int total_messages = NUM_PRODUCERS * MESSAGES_PER_PRODUCER;
    double throughput = total_messages / timer_seconds(&timer);
    printf("  Multi-threaded throughput: %.2f messages/second\n", throughput);
    ASSERT_TRUE(throughput > 200000); // Should handle at least 200k msgs/sec with multiple threads

    asthra_channel_destroy(ch);
    increment_test_counter();
}

// ============================================================================
// Thread function implementations
// ============================================================================

static void *producer_thread(void *arg) {
    AsthraConcurrencyChannelHandle *channel = (AsthraConcurrencyChannelHandle *)arg;
    const int NUM_MESSAGES = 100000;

    for (int i = 0; i < NUM_MESSAGES; i++) {
        AsthraResult result = Asthra_channel_send(channel, &i, 0);
        if (result.tag != ASTHRA_RESULT_OK) {
            return NULL;
        }
    }

    asthra_channel_close(channel);
    return NULL;
}

static void *consumer_thread(void *arg) {
    AsthraConcurrencyChannelHandle *channel = (AsthraConcurrencyChannelHandle *)arg;
    int count = 0;

    while (1) {
        int value;
        AsthraResult result = Asthra_channel_recv(channel, &value, 0);

        if (result.tag == ASTHRA_RESULT_OK) {
            count++;
        } else {
            // Channel closed or error
            break;
        }
    }

    return (void *)(intptr_t)count;
}

static void *multi_producer_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    for (int j = 0; j < data->messages_per_thread; j++) {
        int message = data->id * 10000 + j;
        AsthraResult result = Asthra_channel_send(data->channel, &message, 0);
        if (result.tag != ASTHRA_RESULT_OK) {
            // Channel closed or error
            break;
        }
    }

    return NULL;
}

static void *multi_consumer_thread(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int count = 0;

    while (1) {
        int message;
        AsthraResult result = Asthra_channel_recv(data->channel, &message, 0);

        if (result.tag == ASTHRA_RESULT_OK) {
            count++;
        } else {
            // Channel closed or error
            break;
        }
    }

    return (void *)(intptr_t)count;
}

// ============================================================================
// Main Test Runner
// ============================================================================

int main(void) {
    printf("=== Channel Performance Tests ===\n\n");

    tests_run = 0;
    tests_passed = 0;

    test_channel_throughput();
    test_channel_latency();
    test_channel_scalability();

    printf("\n=== Test Summary ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Pass rate: %.1f%%\n", (double)tests_passed / tests_run * 100);

    return tests_passed == tests_run ? 0 : 1;
}