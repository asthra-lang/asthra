/**
 * Asthra Programming Language v1.2 Integration Tests
 * Concurrent Slice Management with Memory Safety
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_integration_common.h"

// =============================================================================
// CONCURRENT SLICE MANAGEMENT INTEGRATION TEST
// =============================================================================

// Scenario 2: Slice management with concurrent access and memory safety
typedef struct {
    void **slice_data;
    size_t slice_len;
    size_t slice_capacity;
    pthread_mutex_t mutex;
    int access_count;
} ConcurrentSlice;

static ConcurrentSlice *concurrent_slice_create(size_t capacity) {
    ConcurrentSlice *slice = malloc(sizeof(ConcurrentSlice));
    if (!slice)
        return NULL;

    slice->slice_data = malloc(capacity * sizeof(void *));
    if (!slice->slice_data) {
        free(slice);
        return NULL;
    }

    slice->slice_len = 0;
    slice->slice_capacity = capacity;
    slice->access_count = 0;
    pthread_mutex_init(&slice->mutex, NULL);

    return slice;
}

static bool concurrent_slice_push(ConcurrentSlice *slice, void *item) {
    if (!slice)
        return false;

    pthread_mutex_lock(&slice->mutex);

    bool success = false;
    if (slice->slice_len < slice->slice_capacity) {
        slice->slice_data[slice->slice_len] = item;
        slice->slice_len++;
        slice->access_count++;
        success = true;
    }

    pthread_mutex_unlock(&slice->mutex);
    return success;
}

static void *concurrent_slice_get(ConcurrentSlice *slice, size_t index) {
    if (!slice)
        return NULL;

    pthread_mutex_lock(&slice->mutex);

    void *result = NULL;
    if (index < slice->slice_len) {
        result = slice->slice_data[index];
        slice->access_count++;
    }

    pthread_mutex_unlock(&slice->mutex);
    return result;
}

static void concurrent_slice_destroy(ConcurrentSlice *slice) {
    if (!slice)
        return;

    pthread_mutex_destroy(&slice->mutex);
    free(slice->slice_data);
    free(slice);
}

typedef struct {
    ConcurrentSlice *slice;
    int thread_id;
    int operations_performed;
} SliceWorkerArgs;

static void *slice_worker(void *arg) {
    SliceWorkerArgs *args = (SliceWorkerArgs *)arg;

    // Each worker performs multiple operations
    for (int i = 0; i < 10; i++) {
        // Allocate some data
        int *data = malloc(sizeof(int));
        *data = args->thread_id * 100 + i;

        // Try to add to slice
        if (concurrent_slice_push(args->slice, data)) {
            args->operations_performed++;
        } else {
            free(data); // Cleanup if push failed
        }

        // Small delay to increase contention
        usleep(100);
    }

    return args;
}

AsthraTestResult test_integration_concurrent_slice_management(AsthraV12TestContext *ctx) {
    // Integration test: Slice Management + Concurrency + Memory Safety

    const int num_workers = 5;
    const size_t slice_capacity = 30;

    ConcurrentSlice *shared_slice = concurrent_slice_create(slice_capacity);

    if (!ASTHRA_TEST_ASSERT(&ctx->base, shared_slice != NULL,
                            "Shared slice creation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    pthread_t workers[num_workers];
    SliceWorkerArgs worker_args[num_workers];

    // Start workers
    for (int i = 0; i < num_workers; i++) {
        worker_args[i].slice = shared_slice;
        worker_args[i].thread_id = i;
        worker_args[i].operations_performed = 0;

        int result = pthread_create(&workers[i], NULL, slice_worker, &worker_args[i]);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, result == 0, "Worker %d creation should succeed", i)) {
            // Cleanup
            for (int j = 0; j < i; j++) {
                pthread_join(workers[j], NULL);
            }
            concurrent_slice_destroy(shared_slice);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Wait for workers
    for (int i = 0; i < num_workers; i++) {
        pthread_join(workers[i], NULL);
    }

    // Verify results
    if (!ASTHRA_TEST_ASSERT(&ctx->base, shared_slice->slice_len <= slice_capacity,
                            "Slice length should not exceed capacity")) {
        concurrent_slice_destroy(shared_slice);
        return ASTHRA_TEST_FAIL;
    }

    // Count total operations
    int total_operations = 0;
    for (int i = 0; i < num_workers; i++) {
        total_operations += worker_args[i].operations_performed;
    }

    if (!ASTHRA_TEST_ASSERT(&ctx->base, total_operations == (int)shared_slice->slice_len,
                            "Total operations should match slice length")) {
        concurrent_slice_destroy(shared_slice);
        return ASTHRA_TEST_FAIL;
    }

    // Verify data integrity
    for (size_t i = 0; i < shared_slice->slice_len; i++) {
        int *data = (int *)concurrent_slice_get(shared_slice, i);

        if (!ASTHRA_TEST_ASSERT(&ctx->base, data != NULL, "Slice element %zu should not be NULL",
                                i)) {
            concurrent_slice_destroy(shared_slice);
            return ASTHRA_TEST_FAIL;
        }

        // Verify data format (thread_id * 100 + operation_id)
        int thread_id = *data / 100;
        int operation_id = *data % 100;

        if (!ASTHRA_TEST_ASSERT(&ctx->base, thread_id >= 0 && thread_id < num_workers,
                                "Thread ID should be valid: %d", thread_id)) {
            concurrent_slice_destroy(shared_slice);
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(&ctx->base, operation_id >= 0 && operation_id < 10,
                                "Operation ID should be valid: %d", operation_id)) {
            concurrent_slice_destroy(shared_slice);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Cleanup allocated data
    for (size_t i = 0; i < shared_slice->slice_len; i++) {
        void *data = shared_slice->slice_data[i];
        if (data)
            free(data);
    }

    concurrent_slice_destroy(shared_slice);
    return ASTHRA_TEST_PASS;
}
