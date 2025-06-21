/**
 * Asthra Programming Language v1.2 Integration Tests
 * Security Features with Concurrent Access
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_integration_common.h"

// =============================================================================
// SECURITY WITH CONCURRENCY INTEGRATION TEST
// =============================================================================

// Scenario 4: Security features with concurrent access
typedef struct {
    uint8_t secure_data[32];
    bool is_initialized;
    pthread_mutex_t access_mutex;
    int access_count;
} SecureBuffer;

static SecureBuffer* secure_buffer_create(void) {
    SecureBuffer *buffer = malloc(sizeof(SecureBuffer));
    if (!buffer) return NULL;
    
    // Initialize with secure random data (mock)
    for (int i = 0; i < 32; i++) {
        buffer->secure_data[i] = (uint8_t)(rand() % 256);
    }
    
    buffer->is_initialized = true;
    buffer->access_count = 0;
    pthread_mutex_init(&buffer->access_mutex, NULL);
    
    return buffer;
}

static bool secure_buffer_constant_time_compare(SecureBuffer *buffer, const uint8_t *compare_data) {
    if (!buffer || !compare_data || !buffer->is_initialized) {
        return false;
    }
    
    pthread_mutex_lock(&buffer->access_mutex);
    buffer->access_count++;
    
    // Constant-time comparison
    uint8_t result = 0;
    for (int i = 0; i < 32; i++) {
        result |= buffer->secure_data[i] ^ compare_data[i];
    }
    
    pthread_mutex_unlock(&buffer->access_mutex);
    
    return result == 0;
}

static void secure_buffer_zero(SecureBuffer *buffer) {
    if (!buffer) return;
    
    pthread_mutex_lock(&buffer->access_mutex);
    
    // Secure zero (volatile to prevent optimization)
    volatile uint8_t *data = buffer->secure_data;
    for (int i = 0; i < 32; i++) {
        data[i] = 0;
    }
    
    buffer->is_initialized = false;
    
    pthread_mutex_unlock(&buffer->access_mutex);
}

static void secure_buffer_destroy(SecureBuffer *buffer) {
    if (!buffer) return;
    
    secure_buffer_zero(buffer);
    pthread_mutex_destroy(&buffer->access_mutex);
    free(buffer);
}

typedef struct {
    SecureBuffer *buffer;
    uint8_t test_data[32];
    bool comparison_result;
    int thread_id;
} SecurityTestArgs;

static void* security_test_worker(void *arg) {
    SecurityTestArgs *args = (SecurityTestArgs*)arg;
    
    // Perform multiple comparisons
    for (int i = 0; i < 100; i++) {
        args->comparison_result = secure_buffer_constant_time_compare(args->buffer, args->test_data);
        usleep(10); // Small delay
    }
    
    return args;
}

AsthraTestResult test_integration_security_with_concurrency(AsthraV12TestContext *ctx) {
    // Integration test: Security Features + Concurrency + Memory Safety
    
    SecureBuffer *secure_buf = secure_buffer_create();
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, secure_buf != NULL,
                           "Secure buffer creation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, secure_buf->is_initialized,
                           "Secure buffer should be initialized")) {
        secure_buffer_destroy(secure_buf);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test constant-time comparison with correct data
    uint8_t correct_data[32];
    memcpy(correct_data, secure_buf->secure_data, 32);
    
    bool correct_result = secure_buffer_constant_time_compare(secure_buf, correct_data);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, correct_result,
                           "Comparison with correct data should succeed")) {
        secure_buffer_destroy(secure_buf);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test with incorrect data
    uint8_t incorrect_data[32];
    memcpy(incorrect_data, secure_buf->secure_data, 32);
    incorrect_data[0] ^= 1; // Flip one bit
    
    bool incorrect_result = secure_buffer_constant_time_compare(secure_buf, incorrect_data);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !incorrect_result,
                           "Comparison with incorrect data should fail")) {
        secure_buffer_destroy(secure_buf);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test concurrent access
    const int num_threads = 4;
    pthread_t threads[num_threads];
    SecurityTestArgs thread_args[num_threads];
    
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].buffer = secure_buf;
        thread_args[i].thread_id = i;
        thread_args[i].comparison_result = false;
        
        // Half threads use correct data, half use incorrect
        if (i % 2 == 0) {
            memcpy(thread_args[i].test_data, correct_data, 32);
        } else {
            memcpy(thread_args[i].test_data, incorrect_data, 32);
        }
        
        int result = pthread_create(&threads[i], NULL, security_test_worker, &thread_args[i]);
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, result == 0,
                               "Security test thread %d creation should succeed", i)) {
            // Cleanup
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            secure_buffer_destroy(secure_buf);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Wait for threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify results
    for (int i = 0; i < num_threads; i++) {
        bool expected = (i % 2 == 0); // Even threads should succeed
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, thread_args[i].comparison_result == expected,
                               "Thread %d comparison result should be %s", 
                               i, expected ? "true" : "false")) {
            secure_buffer_destroy(secure_buf);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Verify access count
    if (!ASTHRA_TEST_ASSERT(&ctx->base, secure_buf->access_count >= num_threads * 100,
                           "Access count should reflect all operations")) {
        secure_buffer_destroy(secure_buf);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test secure zeroing
    secure_buffer_zero(secure_buf);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, !secure_buf->is_initialized,
                           "Buffer should not be initialized after zeroing")) {
        secure_buffer_destroy(secure_buf);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify data is zeroed
    bool is_zeroed = true;
    for (int i = 0; i < 32; i++) {
        if (secure_buf->secure_data[i] != 0) {
            is_zeroed = false;
            break;
        }
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, is_zeroed,
                           "Secure data should be zeroed")) {
        secure_buffer_destroy(secure_buf);
        return ASTHRA_TEST_FAIL;
    }
    
    secure_buffer_destroy(secure_buf);
    return ASTHRA_TEST_PASS;
} 
