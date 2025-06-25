/**
 * Asthra Programming Language - Spawn Task Function Implementations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Shared task function implementations for spawn tests.
 * These replace the stub implementations with real functionality.
 */

#include "test_spawn_common.h"
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Define task status constants for compatibility
#define TASK_SUCCESS 1
#define TASK_FAILED 0

// =============================================================================
// TASK FUNCTION IMPLEMENTATIONS
// =============================================================================

/**
 * Simple task function for basic testing
 * Returns the integer value passed as argument
 */
void *simple_task_function(void *arg) {
    if (!arg) {
        return NULL;
    }

    // Simulate some work
    int value = *(int *)arg;
    usleep(1000); // 1ms of work

    // Allocate result
    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) {
        return NULL;
    }

    result->is_ok = true;
    result->data.ok_value = malloc(sizeof(int));
    if (result->data.ok_value) {
        *(int *)result->data.ok_value = value * 2; // Double the input value
    }

    return result;
}

/**
 * Task function that accepts TaskArgs structure
 * Performs arithmetic operations based on the operation character
 */
void *task_with_arguments(void *arg) {
    if (!arg) {
        return NULL;
    }

    struct TaskArgs *args = (struct TaskArgs *)arg;

    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) {
        return NULL;
    }

    result->is_ok = true;

    // Allocate memory for the result value
    result->data.ok_value = malloc(sizeof(int));
    if (!result->data.ok_value) {
        result->is_ok = false;
        result->data.error_message = "Memory allocation failed";
        return result;
    }

    // Perform operation based on args->operation
    switch (args->operation) {
    case '+':
        *(int *)result->data.ok_value = args->a + args->b;
        break;
    case '-':
        *(int *)result->data.ok_value = args->a - args->b;
        break;
    case '*':
        *(int *)result->data.ok_value = args->a * args->b;
        break;
    case '/':
        if (args->b == 0) {
            result->is_ok = false;
            result->data.error_message = "Division by zero";
            free(result->data.ok_value);
        } else {
            *(int *)result->data.ok_value = args->a / args->b;
        }
        break;
    default:
        result->is_ok = false;
        result->data.error_message = "Unknown operation";
        free(result->data.ok_value);
        break;
    }

    // Simulate processing time
    usleep(2000); // 2ms

    return result;
}

/**
 * Multi-statement task function
 * Performs multiple operations in sequence
 */
void *multi_statement_task(void *arg) {
    int base_value = arg ? *(int *)arg : 10;

    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) {
        return NULL;
    }

    // Statement 1: Initialize
    int accumulator = base_value;

    // Statement 2: First computation
    accumulator *= 2;
    usleep(500);

    // Statement 3: Second computation
    accumulator += 10;
    usleep(500);

    // Statement 4: Third computation
    accumulator = accumulator % 100;
    usleep(500);

    // Statement 5: Final computation
    accumulator = accumulator * accumulator;

    result->is_ok = true;
    result->data.ok_value = malloc(sizeof(int));
    if (result->data.ok_value) {
        *(int *)result->data.ok_value = accumulator;
    } else {
        result->is_ok = false;
        result->data.error_message = "Memory allocation failed";
    }

    return result;
}

/**
 * Handle task function
 * Used for testing task handle operations
 */
void *handle_task_function(void *arg) {
    HandleTaskData *data = (HandleTaskData *)arg;

    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) {
        return NULL;
    }

    if (!data) {
        result->is_ok = false;
        result->data.error_message = "No task data provided";
        return result;
    }

    // Process based on task_id
    int processing_time = (data->task_id % 5 + 1) * 1000; // 1-5ms
    usleep(processing_time);

    // Compute result based on task_id and data
    result->is_ok = true;
    result->data.ok_value = malloc(sizeof(int));
    if (result->data.ok_value) {
        *(int *)result->data.ok_value = data->task_id * 100 + data->data;
    } else {
        result->is_ok = false;
        result->data.error_message = "Memory allocation failed";
    }

    return result;
}

/**
 * Timeout task function
 * Simulates a long-running task for timeout testing
 */
void *timeout_task_function(void *arg) {
    int sleep_ms = arg ? *(int *)arg : 10000; // Default 10 seconds

    // Sleep for specified duration
    usleep(sleep_ms * 1000);

    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) {
        return NULL;
    }

    result->is_ok = true;
    result->data.ok_value = malloc(sizeof(int));
    if (result->data.ok_value) {
        *(int *)result->data.ok_value = sleep_ms;
    } else {
        result->is_ok = false;
        result->data.error_message = "Memory allocation failed";
    }

    return result;
}

/**
 * Deterministic task function
 * Produces consistent results for deterministic testing
 */
void *deterministic_task(void *arg) {
    int task_id = arg ? *(int *)arg : 0;

    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) {
        return NULL;
    }

    // Deterministic computation based on task_id
    int value = 0;
    for (int i = 0; i <= task_id; i++) {
        value += i * i; // Sum of squares
    }

    // Fixed processing time for determinism
    usleep(1000); // Exactly 1ms

    result->is_ok = true;
    result->data.ok_value = malloc(sizeof(int));
    if (result->data.ok_value) {
        *(int *)result->data.ok_value = value;
    } else {
        result->is_ok = false;
        result->data.error_message = "Memory allocation failed";
    }

    return result;
}

/**
 * Performance task function
 * Used for performance and scalability testing
 */
void *performance_task(void *arg) {
    int iterations = arg ? *(int *)arg : 1000;

    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) {
        return NULL;
    }

    // Perform CPU-bound work
    volatile long sum = 0;
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 100; j++) {
            sum += i * j;
        }
    }

    result->is_ok = true;
    result->data.ok_value = malloc(sizeof(int));
    if (result->data.ok_value) {
        *(int *)result->data.ok_value = (int)(sum % INT32_MAX);
    } else {
        result->is_ok = false;
        result->data.error_message = "Memory allocation failed";
    }

    return result;
}

/**
 * Error task that returns NULL
 * Tests error handling for NULL returns
 */
void *error_task_null_return(void *arg) {
    (void)arg; // Unused

    // Simulate some work before failing
    usleep(500);

    // Return NULL to indicate failure
    return NULL;
}

/**
 * Error task that simulates an exception
 * Tests error handling for exceptional conditions
 */
void *error_task_exception(void *arg) {
    (void)arg; // Unused

    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) {
        return NULL;
    }

    // Simulate an error condition
    result->is_ok = false;
    result->data.error_message = "Simulated exception: Invalid operation";

    return result;
}

/**
 * C function callable from Asthra tasks
 * Tests C integration functionality
 */
void *c_function_for_asthra(void *arg) {
    int input = arg ? *(int *)arg : 0;

    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) {
        return NULL;
    }

    // Simulate calling C library function
    double sqrt_val = sqrt((double)input);

    result->is_ok = true;
    result->data.ok_value = malloc(sizeof(int));
    if (result->data.ok_value) {
        *(int *)result->data.ok_value = (int)sqrt_val;
    } else {
        result->is_ok = false;
        result->data.error_message = "Memory allocation failed";
    }

    return result;
}

/**
 * C thread calling Asthra function
 * Tests bidirectional integration
 */
void *c_thread_calling_asthra(void *arg) {
    int value = arg ? *(int *)arg : 42;

    // Call mock Asthra function
    ConcurrentResult asthra_result = asthra_function_from_c(value);

    // Wrap result for thread return
    ConcurrentResult *result = malloc(sizeof(ConcurrentResult));
    if (!result) {
        return NULL;
    }

    *result = asthra_result;
    return result;
}

/**
 * Mock Asthra function callable from C
 * Simulates an Asthra runtime function
 */
ConcurrentResult asthra_function_from_c(int value) {
    ConcurrentResult result;

    // Simulate Asthra function processing
    result.is_ok = true;
    result.data.ok_value = malloc(sizeof(int));
    if (result.data.ok_value) {
        *(int *)result.data.ok_value = value * 3 + 7; // Some computation
    } else {
        result.is_ok = false;
        result.data.error_message = "Memory allocation failed";
    }

    return result;
}