/**
 * Asthra Enhanced Concurrency Bridge Usage Example
 * Demonstrates thread-safe C interop with task management and callbacks
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "../runtime/asthra_concurrency_bridge.h"

// =============================================================================
// EXAMPLE DATA STRUCTURES
// =============================================================================

typedef struct {
    int worker_id;
    int iterations;
    AsthraConcurrencyMutex *shared_mutex;
    int *shared_counter;
    AsthraConcurrencyCallbackFunction progress_callback;
} WorkerData;

typedef struct {
    int progress_percentage;
    int worker_id;
    char message[256];
} ProgressData;

// =============================================================================
// TASK FUNCTIONS
// =============================================================================

/**
 * CPU-intensive task that performs calculations and reports progress
 */
static AsthraResult cpu_intensive_task(void *args, size_t args_size) {
    WorkerData *data = (WorkerData*)args;
    
    printf("Worker %d: Starting CPU-intensive task with %d iterations\n", 
           data->worker_id, data->iterations);
    
    for (int i = 0; i < data->iterations; i++) {
        // Simulate CPU work
        volatile double result = 0.0;
        for (int j = 0; j < 10000; j++) {
            result += (double)j * 3.14159;
        }
        
        // Update shared counter safely
        if (data->shared_mutex && data->shared_counter) {
            AsthraResult lock_result = Asthra_mutex_lock(data->shared_mutex);
            if (asthra_result_is_ok(lock_result)) {
                (*data->shared_counter)++;
                Asthra_mutex_unlock(data->shared_mutex);
            }
        }
        
        // Report progress every 10% completion
        if (i % (data->iterations / 10) == 0) {
            ProgressData progress = {
                .progress_percentage = (i * 100) / data->iterations,
                .worker_id = data->worker_id
            };
            snprintf(progress.message, sizeof(progress.message), 
                    "Worker %d: %d%% complete", data->worker_id, progress.progress_percentage);
            
            // Enqueue progress callback
            if (data->progress_callback) {
                Asthra_enqueue_callback(data->progress_callback, &progress, 
                                      sizeof(ProgressData), NULL, 1);
            }
        }
    }
    
    printf("Worker %d: Task completed\n", data->worker_id);
    
    // Return worker ID as result
    int *result_id = malloc(sizeof(int));
    *result_id = data->worker_id;
    return asthra_result_ok(result_id, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_GC);
}

/**
 * I/O simulation task that performs file operations
 */
static AsthraResult io_simulation_task(void *args, size_t args_size) {
    WorkerData *data = (WorkerData*)args;
    
    printf("Worker %d: Starting I/O simulation task\n", data->worker_id);
    
    // Simulate file operations with delays
    for (int i = 0; i < 5; i++) {
        printf("Worker %d: Simulating file operation %d/5\n", data->worker_id, i + 1);
        
        // Simulate I/O delay
        usleep(200000); // 200ms
        
        // Report progress
        ProgressData progress = {
            .progress_percentage = ((i + 1) * 100) / 5,
            .worker_id = data->worker_id
        };
        snprintf(progress.message, sizeof(progress.message), 
                "Worker %d: I/O operation %d/5 complete", data->worker_id, i + 1);
        
        if (data->progress_callback) {
            Asthra_enqueue_callback(data->progress_callback, &progress, 
                                  sizeof(ProgressData), NULL, 2);
        }
    }
    
    printf("Worker %d: I/O simulation completed\n", data->worker_id);
    
    int *result_id = malloc(sizeof(int));
    *result_id = data->worker_id;
    return asthra_result_ok(result_id, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_GC);
}

/**
 * Network simulation task
 */
static AsthraResult network_simulation_task(void *args, size_t args_size) {
    WorkerData *data = (WorkerData*)args;
    
    printf("Worker %d: Starting network simulation task\n", data->worker_id);
    
    // Simulate network requests
    for (int i = 0; i < 3; i++) {
        printf("Worker %d: Simulating network request %d/3\n", data->worker_id, i + 1);
        
        // Simulate network latency
        usleep(500000); // 500ms
        
        // Report progress
        ProgressData progress = {
            .progress_percentage = ((i + 1) * 100) / 3,
            .worker_id = data->worker_id
        };
        snprintf(progress.message, sizeof(progress.message), 
                "Worker %d: Network request %d/3 complete", data->worker_id, i + 1);
        
        if (data->progress_callback) {
            Asthra_enqueue_callback(data->progress_callback, &progress, 
                                  sizeof(ProgressData), NULL, 3);
        }
    }
    
    printf("Worker %d: Network simulation completed\n", data->worker_id);
    
    int *result_id = malloc(sizeof(int));
    *result_id = data->worker_id;
    return asthra_result_ok(result_id, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_GC);
}

// =============================================================================
// CALLBACK FUNCTIONS
// =============================================================================

/**
 * Progress reporting callback
 */
static AsthraResult progress_callback(void *data, size_t data_size, void *context) {
    ProgressData *progress = (ProgressData*)data;
    
    printf("PROGRESS: %s (%d%%)\n", progress->message, progress->progress_percentage);
    
    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
}

/**
 * Error handling callback
 */
static AsthraResult error_callback(void *data, size_t data_size, void *context) {
    char *error_message = (char*)data;
    
    printf("ERROR: %s\n", error_message);
    
    return asthra_result_ok(NULL, 0, ASTHRA_TYPE_VOID, ASTHRA_OWNERSHIP_GC);
}

// =============================================================================
// C THREAD WORKER FUNCTIONS
// =============================================================================

/**
 * C thread that registers with Asthra and enqueues callbacks
 */
static void* c_thread_worker(void *arg) {
    int thread_id = *(int*)arg;
    
    printf("C Thread %d: Starting and registering with Asthra\n", thread_id);
    
    // Register with Asthra GC
    AsthraResult result = Asthra_register_c_thread();
    if (asthra_result_is_err(result)) {
        printf("C Thread %d: Failed to register with Asthra\n", thread_id);
        return NULL;
    }
    
    // Simulate some work and enqueue callbacks
    for (int i = 0; i < 5; i++) {
        // Simulate work
        usleep(100000); // 100ms
        
        // Create progress data
        ProgressData progress = {
            .progress_percentage = ((i + 1) * 100) / 5,
            .worker_id = thread_id + 1000 // Offset to distinguish from task workers
        };
        snprintf(progress.message, sizeof(progress.message), 
                "C Thread %d: Background work %d/5 complete", thread_id, i + 1);
        
        // Enqueue callback
        result = Asthra_enqueue_callback(progress_callback, &progress, 
                                       sizeof(ProgressData), NULL, 0);
        if (asthra_result_is_err(result)) {
            printf("C Thread %d: Failed to enqueue callback\n", thread_id);
        }
    }
    
    printf("C Thread %d: Work completed, unregistering\n", thread_id);
    
    // Unregister from Asthra GC
    Asthra_unregister_c_thread();
    
    return NULL;
}

// =============================================================================
// MAIN EXAMPLE FUNCTION
// =============================================================================

int main(void) {
    printf("=== Asthra Enhanced Concurrency Bridge Example ===\n\n");
    
    // Initialize the concurrency bridge
    printf("Initializing concurrency bridge...\n");
    AsthraResult result = Asthra_concurrency_bridge_init(50, 500);
    if (asthra_result_is_err(result)) {
        printf("Failed to initialize concurrency bridge\n");
        return 1;
    }
    
    // Create shared synchronization primitives
    AsthraConcurrencyMutex *shared_mutex = Asthra_mutex_create("shared_counter_mutex", false);
    if (!shared_mutex) {
        printf("Failed to create shared mutex\n");
        Asthra_concurrency_bridge_cleanup();
        return 1;
    }
    
    int shared_counter = 0;
    
    // Create worker data for different types of tasks
    WorkerData cpu_worker = {
        .worker_id = 1,
        .iterations = 100,
        .shared_mutex = shared_mutex,
        .shared_counter = &shared_counter,
        .progress_callback = progress_callback
    };
    
    WorkerData io_worker = {
        .worker_id = 2,
        .iterations = 0, // Not used for I/O task
        .shared_mutex = shared_mutex,
        .shared_counter = &shared_counter,
        .progress_callback = progress_callback
    };
    
    WorkerData network_worker = {
        .worker_id = 3,
        .iterations = 0, // Not used for network task
        .shared_mutex = shared_mutex,
        .shared_counter = &shared_counter,
        .progress_callback = progress_callback
    };
    
    // Spawn different types of tasks
    printf("\nSpawning tasks...\n");
    
    AsthraConcurrencyTaskSpawnOptions cpu_options = {
        .stack_size = 1024 * 1024, // 1MB stack
        .priority = 0,
        .detached = false,
        .name = "cpu_intensive_task",
        .affinity_mask = NULL
    };
    
    AsthraConcurrencyTaskSpawnOptions io_options = {
        .stack_size = 512 * 1024, // 512KB stack
        .priority = 1,
        .detached = false,
        .name = "io_simulation_task",
        .affinity_mask = NULL
    };
    
    AsthraConcurrencyTaskSpawnOptions network_options = {
        .stack_size = 512 * 1024, // 512KB stack
        .priority = 2,
        .detached = false,
        .name = "network_simulation_task",
        .affinity_mask = NULL
    };
    
    AsthraConcurrencyTaskHandle *cpu_handle = Asthra_spawn_task(cpu_intensive_task, &cpu_worker, 
                                                               sizeof(WorkerData), &cpu_options);
    AsthraConcurrencyTaskHandle *io_handle = Asthra_spawn_task(io_simulation_task, &io_worker, 
                                                              sizeof(WorkerData), &io_options);
    AsthraConcurrencyTaskHandle *network_handle = Asthra_spawn_task(network_simulation_task, &network_worker, 
                                                                   sizeof(WorkerData), &network_options);
    
    if (!cpu_handle || !io_handle || !network_handle) {
        printf("Failed to spawn one or more tasks\n");
        Asthra_mutex_destroy(shared_mutex);
        Asthra_concurrency_bridge_cleanup();
        return 1;
    }
    
    // Start C threads that will register with Asthra
    printf("\nStarting C threads...\n");
    
    const int num_c_threads = 3;
    pthread_t c_threads[num_c_threads];
    int thread_ids[num_c_threads];
    
    for (int i = 0; i < num_c_threads; i++) {
        thread_ids[i] = i + 1;
        if (pthread_create(&c_threads[i], NULL, c_thread_worker, &thread_ids[i]) != 0) {
            printf("Failed to create C thread %d\n", i + 1);
        }
    }
    
    // Process callbacks periodically while tasks are running
    printf("\nProcessing callbacks and monitoring tasks...\n");
    
    bool all_tasks_complete = false;
    int callback_process_cycles = 0;
    
    while (!all_tasks_complete) {
        // Process callbacks
        size_t processed = Asthra_process_callbacks(10);
        if (processed > 0) {
            printf("Processed %zu callbacks\n", processed);
        }
        
        // Check task completion status
        bool cpu_complete = Asthra_task_is_complete(cpu_handle);
        bool io_complete = Asthra_task_is_complete(io_handle);
        bool network_complete = Asthra_task_is_complete(network_handle);
        
        all_tasks_complete = cpu_complete && io_complete && network_complete;
        
        if (!all_tasks_complete) {
            usleep(100000); // 100ms
            callback_process_cycles++;
            
            // Print status every 2 seconds
            if (callback_process_cycles % 20 == 0) {
                printf("Task status - CPU: %s, I/O: %s, Network: %s\n",
                       cpu_complete ? "DONE" : "RUNNING",
                       io_complete ? "DONE" : "RUNNING",
                       network_complete ? "DONE" : "RUNNING");
            }
        }
    }
    
    // Wait for task results
    printf("\nWaiting for task results...\n");
    
    AsthraResult cpu_result = Asthra_task_get_result(cpu_handle);
    AsthraResult io_result = Asthra_task_get_result(io_handle);
    AsthraResult network_result = Asthra_task_get_result(network_handle);
    
    // Process results
    if (asthra_result_is_ok(cpu_result)) {
        int *worker_id = (int*)asthra_result_unwrap_ok(cpu_result);
        printf("CPU task completed successfully (Worker ID: %d)\n", *worker_id);
    } else {
        printf("CPU task failed\n");
    }
    
    if (asthra_result_is_ok(io_result)) {
        int *worker_id = (int*)asthra_result_unwrap_ok(io_result);
        printf("I/O task completed successfully (Worker ID: %d)\n", *worker_id);
    } else {
        printf("I/O task failed\n");
    }
    
    if (asthra_result_is_ok(network_result)) {
        int *worker_id = (int*)asthra_result_unwrap_ok(network_result);
        printf("Network task completed successfully (Worker ID: %d)\n", *worker_id);
    } else {
        printf("Network task failed\n");
    }
    
    // Wait for C threads to complete
    printf("\nWaiting for C threads to complete...\n");
    for (int i = 0; i < num_c_threads; i++) {
        pthread_join(c_threads[i], NULL);
    }
    
    // Process any remaining callbacks
    printf("\nProcessing remaining callbacks...\n");
    size_t remaining = Asthra_process_callbacks(100);
    printf("Processed %zu remaining callbacks\n", remaining);
    
    // Display final statistics
    printf("\nFinal Statistics:\n");
    AsthraConcurrencyStats stats = Asthra_get_concurrency_stats();
    printf("Tasks spawned: %llu\n", stats.tasks_spawned);
    printf("Tasks completed: %llu\n", stats.tasks_completed);
    printf("Tasks failed: %llu\n", stats.tasks_failed);
    printf("Callbacks enqueued: %llu\n", stats.callbacks_enqueued);
    printf("Callbacks processed: %llu\n", stats.callbacks_processed);
    printf("Threads registered: %llu\n", stats.threads_registered);
    printf("Mutex contentions: %llu\n", stats.mutex_contentions);
    printf("Shared counter final value: %d\n", shared_counter);
    
    // Dump detailed state
    printf("\nDetailed State Dump:\n");
    Asthra_dump_concurrency_state(stdout);
    
    // Cleanup
    printf("\nCleaning up...\n");
    Asthra_task_handle_free(cpu_handle);
    Asthra_task_handle_free(io_handle);
    Asthra_task_handle_free(network_handle);
    Asthra_mutex_destroy(shared_mutex);
    Asthra_concurrency_bridge_cleanup();
    
    printf("\nExample completed successfully!\n");
    return 0;
}

// =============================================================================
// ADDITIONAL UTILITY FUNCTIONS FOR DEMONSTRATION
// =============================================================================

/**
 * Demonstrates timeout functionality
 */
void demonstrate_timeout_functionality(void) {
    printf("\n=== Timeout Functionality Demonstration ===\n");
    
    // Initialize bridge
    Asthra_concurrency_bridge_init(10, 100);
    
    // Spawn a long-running task
    WorkerData long_worker = {.worker_id = 999, .iterations = 1000000};
    AsthraConcurrencyTaskHandle *long_handle = Asthra_spawn_task(cpu_intensive_task, &long_worker, 
                                                               sizeof(WorkerData), NULL);
    
    if (long_handle) {
        printf("Spawned long-running task, testing timeout...\n");
        
        // Try to wait with a short timeout
        AsthraResult timeout_result = Asthra_task_wait_timeout(long_handle, 500); // 500ms
        
        if (asthra_result_is_err(timeout_result)) {
            printf("Task wait timed out as expected\n");
        } else {
            printf("Task completed unexpectedly quickly\n");
        }
        
        // Cancel the task
        AsthraResult cancel_result = Asthra_task_cancel(long_handle);
        if (asthra_result_is_ok(cancel_result)) {
            printf("Task cancelled successfully\n");
        }
        
        Asthra_task_handle_free(long_handle);
    }
    
    Asthra_concurrency_bridge_cleanup();
}

/**
 * Demonstrates error handling patterns
 */
void demonstrate_error_handling(void) {
    printf("\n=== Error Handling Demonstration ===\n");
    
    // Initialize bridge
    Asthra_concurrency_bridge_init(10, 100);
    
    // Try to spawn task without initialization (should fail)
    Asthra_concurrency_bridge_cleanup();
    
    WorkerData worker = {.worker_id = 1, .iterations = 10};
    AsthraConcurrencyTaskHandle *handle = Asthra_spawn_task(cpu_intensive_task, &worker, 
                                                           sizeof(WorkerData), NULL);
    
    if (!handle) {
        printf("Task spawn failed as expected (bridge not initialized)\n");
    }
    
    // Re-initialize for proper cleanup
    Asthra_concurrency_bridge_init(10, 100);
    Asthra_concurrency_bridge_cleanup();
} 
