/**
 * Asthra Concurrency Bridge FFI - Task Management
 * Task spawning, waiting, and management functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_ffi_common.h"

// =============================================================================
// TASK MANAGEMENT
// =============================================================================

// Task worker thread function
static void* task_worker_thread(void *arg) {
    AsthraConcurrencyTaskHandle *handle = (AsthraConcurrencyTaskHandle*)arg;
    
    if (!handle || !handle->task_data) {
        return NULL;
    }
    
    // Register thread with the concurrency bridge
    Asthra_register_c_thread();
    
    // Extract function and arguments
    struct {
        AsthraConcurrencyTaskFunction func;
        void *args;
        size_t args_size;
    } *task_info = handle->task_data;
    
    // Execute the task
    AsthraResult result = task_info->func(task_info->args, task_info->args_size);
    
    // Debug: Print result information
    printf("[TASK DEBUG] Task executed, result tag: %d, is_ok: %s\n", 
           result.tag, asthra_result_is_ok(result) ? "true" : "false");
    if (asthra_result_is_ok(result)) {
        printf("[TASK DEBUG] OK result value ptr: %p\n", result.data.ok.value);
    }
    
    // Store result
    pthread_mutex_lock(&handle->result_mutex);
    handle->result_ptr = (uintptr_t)malloc(sizeof(AsthraResult));
    if (handle->result_ptr) {
        *(AsthraResult*)handle->result_ptr = result;
        printf("[TASK DEBUG] Result stored at ptr: %p\n", (void*)handle->result_ptr);
    }
    atomic_store(&handle->is_complete, true);
    pthread_cond_signal(&handle->completion_cond);
    pthread_mutex_unlock(&handle->result_mutex);
    
    // Update statistics
    pthread_mutex_lock(&bridge_state.mutex);
    if (asthra_result_is_ok(result)) {
        atomic_fetch_add(&bridge_state.stats.tasks_completed, 1);
    } else {
        atomic_fetch_add(&bridge_state.stats.tasks_failed, 1);
    }
    pthread_mutex_unlock(&bridge_state.mutex);
    
    // Unregister thread
    Asthra_unregister_c_thread();
    
    return NULL;
}

AsthraConcurrencyTaskHandle* Asthra_spawn_task(AsthraConcurrencyTaskFunction func, void* args, 
                                               size_t args_size, const AsthraConcurrencyTaskSpawnOptions* options) {
    if (!func || !asthra_concurrency_is_initialized()) {
        return NULL;
    }
    
    // Allocate task handle
    AsthraConcurrencyTaskHandle *handle = calloc(1, sizeof(AsthraConcurrencyTaskHandle));
    if (!handle) {
        return NULL;
    }
    
    // Initialize handle
    handle->base_handle.task_id = atomic_fetch_add(&bridge_state.next_task_id, 1);
    atomic_store(&handle->enhanced_state, 0);
    atomic_store(&handle->is_complete, false);
    atomic_store(&handle->is_detached, options && options->detached);
    atomic_store(&handle->result_ptr, (uintptr_t)NULL);
    pthread_mutex_init(&handle->result_mutex, NULL);
    pthread_cond_init(&handle->completion_cond, NULL);
    atomic_store(&handle->creation_time, get_time_ns());
    
    // Store task function and arguments
    struct {
        AsthraConcurrencyTaskFunction func;
        void *args;
        size_t args_size;
    } *task_info = malloc(sizeof(*task_info) + args_size);
    
    if (!task_info) {
        pthread_mutex_destroy(&handle->result_mutex);
        pthread_cond_destroy(&handle->completion_cond);
        free(handle);
        return NULL;
    }
    
    task_info->func = func;
    task_info->args_size = args_size;
    if (args_size > 0 && args) {
        task_info->args = (char*)task_info + sizeof(*task_info);
        memcpy(task_info->args, args, args_size);
    } else {
        task_info->args = NULL;
    }
    
    handle->task_data = task_info;
    handle->task_data_size = sizeof(*task_info) + args_size;
    
    // Create thread for task
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    if (options && options->detached) {
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    }
    
    if (options && options->stack_size > 0) {
        pthread_attr_setstacksize(&attr, options->stack_size);
    }
    
    int ret = pthread_create(&thread, &attr, task_worker_thread, handle);
    pthread_attr_destroy(&attr);
    
    if (ret != 0) {
        free(task_info);
        pthread_mutex_destroy(&handle->result_mutex);
        pthread_cond_destroy(&handle->completion_cond);
        free(handle);
        return NULL;
    }
    
    // platform_handle field not available in current AsthraTaskHandle structure
    // handle->base_handle.platform_handle = (void*)(uintptr_t)thread;
    
    // Register task
    pthread_mutex_lock(&bridge_state.mutex);
    
    TaskRegistryEntry *entry = malloc(sizeof(TaskRegistryEntry));
    if (entry) {
        entry->handle = handle;
        entry->next = bridge_state.task_registry;
        bridge_state.task_registry = entry;
    }
    
    atomic_fetch_add(&bridge_state.stats.tasks_spawned, 1);
    
    pthread_mutex_unlock(&bridge_state.mutex);
    
    return handle;
}

AsthraResult Asthra_task_get_result(AsthraConcurrencyTaskHandle* handle) {
    if (!handle) {
        return create_error("Invalid task handle");
    }
    
    pthread_mutex_lock(&handle->result_mutex);
    
    // Wait for completion
    while (!atomic_load(&handle->is_complete)) {
        pthread_cond_wait(&handle->completion_cond, &handle->result_mutex);
    }
    
    AsthraResult result = handle->result_ptr ? *(AsthraResult*)handle->result_ptr : create_ok();
    
    // Debug: Print retrieved result information
    printf("[GET_RESULT DEBUG] Task completed, result_ptr: %p\n", (void*)handle->result_ptr);
    printf("[GET_RESULT DEBUG] Retrieved result tag: %d, is_ok: %s\n", 
           result.tag, asthra_result_is_ok(result) ? "true" : "false");
    if (asthra_result_is_ok(result)) {
        printf("[GET_RESULT DEBUG] OK result value ptr: %p\n", result.data.ok.value);
    }
    
    pthread_mutex_unlock(&handle->result_mutex);
    
    return result;
}

bool Asthra_task_is_complete(AsthraConcurrencyTaskHandle* handle) {
    if (!handle) {
        return false;
    }
    return atomic_load(&handle->is_complete);
}

void Asthra_task_handle_free(AsthraConcurrencyTaskHandle* handle) {
    if (!handle) {
        return;
    }
    
    // Remove from task registry
    pthread_mutex_lock(&bridge_state.mutex);
    
    TaskRegistryEntry **prev_ptr = &bridge_state.task_registry;
    TaskRegistryEntry *entry = bridge_state.task_registry;
    
    while (entry) {
        if (entry->handle == handle) {
            *prev_ptr = entry->next;
            free(entry);
            break;
        }
        prev_ptr = &entry->next;
        entry = entry->next;
    }
    
    pthread_mutex_unlock(&bridge_state.mutex);
    
    // Cleanup handle resources
    if (handle->task_data) {
        free(handle->task_data);
    }
    
    if (handle->result_ptr) {
        free((void*)handle->result_ptr);
    }
    
    pthread_mutex_destroy(&handle->result_mutex);
    pthread_cond_destroy(&handle->completion_cond);
    
    free(handle);
}

AsthraResult Asthra_task_wait_timeout(AsthraConcurrencyTaskHandle* handle, uint64_t timeout_ms) {
    if (!handle) {
        return create_error("Invalid task handle");
    }
    
    pthread_mutex_lock(&handle->result_mutex);
    
    // Check if already complete
    if (atomic_load(&handle->is_complete)) {
        AsthraResult result = handle->result_ptr ? *(AsthraResult*)handle->result_ptr : create_ok();
        pthread_mutex_unlock(&handle->result_mutex);
        return result;
    }
    
    // Wait with timeout
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout_ms / 1000;
    ts.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    
    int ret = pthread_cond_timedwait(&handle->completion_cond, &handle->result_mutex, &ts);
    
    if (ret == ETIMEDOUT) {
        pthread_mutex_unlock(&handle->result_mutex);
        atomic_fetch_add(&bridge_state.stats.tasks_timeout, 1);
        return create_error("Task timeout");
    }
    
    AsthraResult result = handle->result_ptr ? *(AsthraResult*)handle->result_ptr : create_ok();
    pthread_mutex_unlock(&handle->result_mutex);
    
    return result;
}