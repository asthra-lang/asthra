/**
 * Asthra Programming Language - Bridge Test Global Counters
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Global test counters required by the RUN_TEST macro.
 * All test function implementations are in their respective specialized files.
 */

#include "bridge_test_common.h"

// =============================================================================
// GLOBAL TEST COUNTERS (required by RUN_TEST macro)
// =============================================================================

int tests_run = 0;
int tests_passed = 0;

// =============================================================================
// HELPER FUNCTION IMPLEMENTATIONS
// =============================================================================

// Task functions for testing
AsthraResult simple_task_function(void *args, size_t args_size) {
    printf("[STUB DEBUG] simple_task_function called with args: %p, size: %zu\n", args, args_size);
    fflush(stdout);
    
    int *value = (int*)args;
    
    // Allocate new memory for the result since args will be freed
    int *result = malloc(sizeof(int));
    if (!result) {
        printf("[STUB DEBUG] Malloc failed, returning error\n");
        fflush(stdout);
        return asthra_result_err(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_C);
    }
    
    if (value) {
        *result = (*value) + 1;  // Increment and store in result
        printf("[STUB DEBUG] Set result value to: %d\n", *result);
    } else {
        *result = 1;  // Default value if no input
        printf("[STUB DEBUG] Set default result value to: %d\n", *result);
    }
    fflush(stdout);
    
    usleep(10000); // 10ms delay to simulate work
    
    AsthraResult final_result = asthra_result_ok(result, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_C);
    printf("[STUB DEBUG] Created final result, tag: %d, value ptr: %p\n", 
           final_result.tag, final_result.data.ok.value);
    fflush(stdout);
    
    return final_result;
}

AsthraResult failing_task_function(void *args, size_t args_size) {
    (void)args;
    (void)args_size;
    
    printf("[STUB DEBUG] failing_task_function called\n");
    fflush(stdout);
    
    // Task that always fails with error code 42
    int *error_code = malloc(sizeof(int));
    if (!error_code) {
        return asthra_result_err(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_C);
    }
    *error_code = 42;
    
    AsthraResult result = asthra_result_err(error_code, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_C);
    printf("[STUB DEBUG] Created error result with code: %d\n", *error_code);
    fflush(stdout);
    
    return result;
}

AsthraResult long_running_task_function(void *args, size_t args_size) {
    (void)args;
    (void)args_size;
    
    // Sleep for a while to simulate long-running task
    sleep(1); // 1 second
    
    AsthraResult result = { .tag = ASTHRA_RESULT_OK };
    return result;
}

AsthraResult mutex_increment_task(void *args, size_t args_size) {
    if (!args || args_size != sizeof(MutexTestData*)) {
        AsthraResult result = {
            .tag = ASTHRA_RESULT_ERR,
            .data.err = {
                .error = (void*)"Invalid arguments",
                .error_size = 17,
                .error_type_id = 0
            }
        };
        return result;
    }
    
    MutexTestData *data = *(MutexTestData**)args;
    
    for (int i = 0; i < data->num_increments; i++) {
        Asthra_mutex_lock(data->mutex);
        data->shared_counter++;
        Asthra_mutex_unlock(data->mutex);
    }
    
    AsthraResult result = { .tag = ASTHRA_RESULT_OK };
    return result;
}

// Callback functions for testing
AsthraResult callback_test_function(void *data, size_t data_size, void *context) {
    (void)data;
    (void)data_size;
    
    // Increment counter if provided
    int *counter = (int*)context;
    if (counter) {
        (*counter)++;
    }
    
    // Simple callback that always succeeds
    AsthraResult result = { .tag = ASTHRA_RESULT_OK };
    return result;
}

// Thread worker functions for testing
void* thread_registration_test_worker(void *arg) {
    int *success_count = (int*)arg;
    
    // Register thread
    AsthraResult result = Asthra_register_c_thread();
    if (asthra_result_is_ok(result)) {
        // Use atomic increment to avoid race condition
        __sync_fetch_and_add(success_count, 1);
    } else {
        // Debug: print error
        printf("Thread registration failed\n");
    }
    
    // Do some work
    usleep(10000); // 10ms
    
    // Unregister thread
    Asthra_unregister_c_thread();
    
    return NULL;
}