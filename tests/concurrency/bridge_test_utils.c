/**
 * Asthra Enhanced Concurrency Bridge Test Suite - Helper Functions
 * Common helper functions and test data for bridge tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_test_common.h"

// Global test counters
size_t tests_run = 0;
size_t tests_passed = 0;

// =============================================================================
// TEST HELPER FUNCTIONS
// =============================================================================

AsthraResult simple_task_function(void *args, size_t args_size) {
    printf("[TASK FUNC DEBUG] simple_task_function called with args: %p, size: %zu\n", args, args_size);
    fflush(stdout);
    
    // Simple task that just returns success
    int *value = (int*)args;
    printf("[TASK FUNC DEBUG] Input value pointer: %p\n", (void*)value);
    if (value) {
        printf("[TASK FUNC DEBUG] Input value: %d\n", *value);
    }
    fflush(stdout);
    
    // Allocate new memory for the result since args will be freed
    int *result = malloc(sizeof(int));
    printf("[TASK FUNC DEBUG] Allocated result pointer: %p\n", (void*)result);
    fflush(stdout);
    
    if (!result) {
        printf("[TASK FUNC DEBUG] Malloc failed, returning error\n");
        fflush(stdout);
        return asthra_result_err(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_C);
    }
    
    if (value) {
        *result = (*value) + 1;  // Increment and store in result
        printf("[TASK FUNC DEBUG] Set result value to: %d\n", *result);
    } else {
        *result = 1;  // Default value if no input
        printf("[TASK FUNC DEBUG] Set default result value to: %d\n", *result);
    }
    fflush(stdout);
    
    usleep(10000); // 10ms delay to simulate work
    
    AsthraResult final_result = asthra_result_ok(result, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_C);
    printf("[TASK FUNC DEBUG] Created final result, tag: %d, value ptr: %p\n", 
           final_result.tag, final_result.data.ok.value);
    fflush(stdout);
    
    return final_result;
}

AsthraResult failing_task_function(void *args, size_t args_size) {
    // Task that always fails
    int *error_code = malloc(sizeof(int));
    if (!error_code) {
        return asthra_result_err(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_C);
    }
    *error_code = 42;
    return asthra_result_err(error_code, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_C);
}

AsthraResult long_running_task_function(void *args, size_t args_size) {
    // Task that runs for a longer time
    sleep(1); // 1 second delay
    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_C);
}

AsthraResult callback_test_function(void *data, size_t data_size, void *context) {
    int *counter = (int*)context;
    if (counter) {
        (*counter)++;
    }
    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_C);
}

void* thread_registration_test_worker(void *arg) {
    int *success_count = (int*)arg;
    
    // Register thread
    AsthraResult result = Asthra_register_c_thread();
    if (asthra_result_is_ok(result)) {
        // Use atomic increment to avoid race condition
        __sync_fetch_and_add(success_count, 1);
    } else {
        // Debug: print error
        const AsthraErrorData *error = asthra_result_error_data(&result);
        if (error && error->message) {
            printf("Thread registration failed: %s\n", error->message);
        }
    }
    
    // Do some work
    usleep(10000); // 10ms
    
    // Unregister thread
    Asthra_unregister_c_thread();
    
    return NULL;
}

AsthraResult mutex_increment_task(void *args, size_t args_size) {
    MutexTestData **data_ptr = (MutexTestData**)args;
    MutexTestData *data = *data_ptr;
    
    for (int i = 0; i < data->num_increments; i++) {
        AsthraResult result = Asthra_mutex_lock(data->mutex);
        if (asthra_result_is_err(result)) {
            return result;
        }
        
        data->shared_counter++;
        
        result = Asthra_mutex_unlock(data->mutex);
        if (asthra_result_is_err(result)) {
            return result;
        }
    }
    
    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_C);
} 
