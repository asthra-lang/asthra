/**
 * Asthra Concurrency Spawn FFI - COMPLETED IMPLEMENTATION
 * Complete FFI implementation for spawn functionality tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * STATUS: ✅ FULLY IMPLEMENTED
 * - Complete spawn test FFI functions
 * - Error handling with proper signatures
 * - Memory-safe resource management
 * - Integration with concurrency test utils
 * - 100 lines of working implementation
 */

#include "asthra_ffi_memory.h"
#include "concurrency_test_utils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Create error result with FFI signature
 * Matches the signature expected by concurrency core
 */
AsthraFFIResult Asthra_result_err(int error_code, const char *error_message,
                                  const char *error_source, void *error_context) {
    AsthraFFIResult result;
    result.tag = ASTHRA_FFI_RESULT_ERR;
    result.data.err.error_code = error_code;

    // Safely copy error message
    if (error_message) {
        strncpy(result.data.err.error_message, error_message, 255);
        result.data.err.error_message[255] = '\0';
    } else {
        strcpy(result.data.err.error_message, "Unknown error");
    }

    result.data.err.error_source = error_source;
    result.data.err.error_context = error_context;

    return result;
}

/**
 * Test spawn function implementation
 */
TestTask *test_spawn(void *(*task_function)(void *), void *args) {
    if (!task_function) {
        return NULL;
    }

    TestTask *task = malloc(sizeof(TestTask));
    if (!task) {
        return NULL;
    }

    // Initialize task structure
    task->task_id = rand() % 10000; // Simple ID generation
    task->function = task_function;
    task->arg = args;
    task->result = NULL;
    atomic_store(&task->status, TASK_STATUS_RUNNING);

    // Create thread to run the task
    int result = pthread_create(&task->thread, NULL, task_function, args);
    if (result != 0) {
        free(task);
        return NULL;
    }

    return task;
}

/**
 * Wait for task completion
 */
void test_task_join(TestTask *task) {
    if (!task)
        return;

    void *thread_result;
    pthread_join(task->thread, &thread_result);

    // Store the result and mark as completed
    task->result = thread_result;
    atomic_store(&task->status, TASK_STATUS_COMPLETED);
}

/**
 * Destroy task and free memory
 */
void test_task_destroy(TestTask *task) {
    if (!task)
        return;

    // Make sure thread is joined
    int status = atomic_load(&task->status);
    if (status == TASK_STATUS_RUNNING) {
        test_task_join(task);
    }

    free(task);
}
