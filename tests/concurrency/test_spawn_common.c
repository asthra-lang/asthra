/**
 * Asthra Programming Language v1.2 Concurrency Tests - Spawn Common Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Shared utilities and task functions for spawn functionality tests
 */

#include "test_spawn_common.h"

// =============================================================================
// SHARED TASK FUNCTION IMPLEMENTATIONS
// =============================================================================

// Simple test function for spawning (exported for use by other test modules)
void* simple_task_function(void *arg) {
    int *input = (int*)arg;
    int *result = malloc(sizeof(int));
    if (!result) return NULL;
    
    *result = (*input) * 2;
    
    // Simulate some work
    usleep(10000); // 10ms
    
    return result;
}

// Enhanced test for spawn with arguments
void* task_with_arguments(void *arg) {
    struct TaskArgs *args = (struct TaskArgs*)arg;
    
    int *result = malloc(sizeof(int));
    if (!result) return NULL;
    
    switch (args->operation) {
        case '+':
            *result = args->a + args->b;
            break;
        case '*':
            *result = args->a * args->b;
            break;
        case '-':
            *result = args->a - args->b;
            break;
        default:
            *result = 0;
    }
    
    return result;
}

// Test spawn with multiple statements
void* multi_statement_task(void *arg) {
    int *input = (int*)arg;
    int *result = malloc(sizeof(int));
    if (!result) return NULL;
    
    // Multiple statements in task
    int temp = *input;
    temp = temp * 2;
    temp = temp + 10;
    temp = temp / 3;
    *result = temp;
    
    // Simulate computational work
    for (int i = 0; i < 1000; i++) {
        temp += (i % 7);
    }
    *result += (temp % 100);
    
    return result;
}

// Handle task function for spawn_with_handle operations
void* handle_task_function(void *arg) {
    HandleTaskData *data = (HandleTaskData*)arg;
    HandleTaskData *result = malloc(sizeof(HandleTaskData));
    if (!result) return NULL;
    
    result->task_id = data->task_id;
    result->data = data->data * data->task_id;
    
    // Simulate different work times
    usleep(data->task_id * 5000); // 5ms per task_id
    
    return result;
}

// Test await expressions with timeouts
void* timeout_task_function(void *arg) {
    int *delay_ms = (int*)arg;
    
    // Sleep for specified time
    usleep((*delay_ms) * 1000);
    
    int *result = malloc(sizeof(int));
    if (!result) return NULL;
    *result = *delay_ms;
    
    return result;
}

// Test deterministic behavior
void* deterministic_task(void *arg) {
    int *input = (int*)arg;
    int *result = malloc(sizeof(int));
    if (!result) return NULL;
    
    // Pure computation - should always return same result for same input
    *result = (*input * *input) + (*input * 2) + 1;
    
    return result;
}

// Test performance and scalability
void* performance_task(void *arg) {
    int *work_amount = (int*)arg;
    
    // Simulate computational work
    volatile int sum = 0;
    for (int i = 0; i < *work_amount; i++) {
        sum += i % 17;
    }
    
    int *result = malloc(sizeof(int));
    if (!result) return NULL;
    *result = sum;
    
    return result;
}

// Test error cases
void* error_task_null_return(void *arg) {
    (void)arg;
    return NULL; // Simulate error condition
}

void* error_task_exception(void *arg) {
    int *input = (int*)arg;
    if (*input < 0) {
        return NULL; // Error condition
    }
    
    int *result = malloc(sizeof(int));
    if (!result) return NULL;
    *result = *input * 2;
    return result;
}

// =============================================================================
// C INTEGRATION TASK FUNCTIONS
// =============================================================================

// C function that can be called from Asthra tasks
void* c_function_for_asthra(void *arg) {
    char *input = (char*)arg;
    char *result = malloc(strlen(input) + 20);
    if (!result) return NULL;
    
    snprintf(result, strlen(input) + 20, "Processed: %s", input);
    
    // Simulate C library work
    usleep(5000); // 5ms
    
    return result;
}

// Mock Asthra function that can be called from C threads
ConcurrentResult asthra_function_from_c(int value) {
    if (value < 0) {
        return concurrent_result_err("Negative values not allowed");
    }
    
    int *result = malloc(sizeof(int));
    if (!result) {
        return concurrent_result_err("Memory allocation failed");
    }
    
    *result = value * value;
    return concurrent_result_ok(result);
}

// C thread function that calls Asthra
void* c_thread_calling_asthra(void *arg) {
    int input = *(int*)arg;
    
    ConcurrentResult result = asthra_function_from_c(input);
    
    if (!result.is_ok) {
        return NULL; // Error case
    }
    
    return result.data.ok_value;
} 
