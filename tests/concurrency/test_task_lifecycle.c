/**
 * Asthra Programming Language v1.2 Concurrency Tests - Task Lifecycle
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for task lifecycle management including creation, execution, and cleanup.
 */

#include "concurrency_test_utils.h"

// =============================================================================
// TASK LIFECYCLE TESTS
// =============================================================================

static void* lifecycle_test_function(void *arg) {
    int *phases = (int*)arg;
    
    // Phase 1: Initialization
    phases[0] = 1;
    usleep(1000);
    
    // Phase 2: Processing
    phases[1] = 1;
    usleep(2000);
    
    // Phase 3: Cleanup
    phases[2] = 1;
    usleep(1000);
    
    return phases;
}

AsthraTestResult test_concurrency_task_creation(AsthraV12TestContext *ctx) {
    // Test task creation and initial state
    
    int phases[3] = {0, 0, 0};
    TestTask *task = test_spawn(lifecycle_test_function, phases);
    
    if (!ASTHRA_TEST_ASSERT(ctx, task != NULL,
                           "Task creation should succeed", "")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify initial state
    if (!ASTHRA_TEST_ASSERT(ctx, task->task_id > 0,
                           "Task should have valid ID", "")) {
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(ctx, task->function == lifecycle_test_function,
                           "Task should have correct function pointer", "")) {
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(ctx, task->arg == phases,
                           "Task should have correct argument", "")) {
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    // Task should start running quickly
    usleep(100000); // 100ms
    int status = atomic_load(&task->status);
    if (!ASTHRA_TEST_ASSERT(ctx, status >= TASK_STATUS_RUNNING,
                           "Task should be running or completed, status: %d", status)) {
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    test_task_join(task);
    test_task_destroy(task);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_concurrency_task_execution(AsthraV12TestContext *ctx) {
    // Test task execution phases
    
    int phases[3] = {0, 0, 0};
    TestTask *task = test_spawn(lifecycle_test_function, phases);
    
    if (!ASTHRA_TEST_ASSERT(ctx, task != NULL,
                           "Task creation should succeed", "")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Wait for task to complete
    test_task_join(task);
    
    // Verify all phases completed
    if (!ASTHRA_TEST_ASSERT(ctx, phases[0] == 1,
                           "Phase 1 should have completed", "")) {
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(ctx, phases[1] == 1,
                           "Phase 2 should have completed", "")) {
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(ctx, phases[2] == 1,
                           "Phase 3 should have completed", "")) {
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify timing information
    struct timespec duration;
    duration.tv_sec = task->end_time.tv_sec - task->start_time.tv_sec;
    duration.tv_nsec = task->end_time.tv_nsec - task->start_time.tv_nsec;
    if (duration.tv_nsec < 0) {
        duration.tv_sec--;
        duration.tv_nsec += 1000000000;
    }
    
    long duration_ms = duration.tv_sec * 1000 + duration.tv_nsec / 1000000;
    
    // Should take approximately 4ms (1+2+1)
    if (!ASTHRA_TEST_ASSERT(ctx, duration_ms >= 3 && duration_ms <= 50,
                           "Task duration should be reasonable: %ld ms", duration_ms)) {
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    test_task_destroy(task);
    return ASTHRA_TEST_PASS;
}

static void* cleanup_test_function(void *arg) {
    int *cleanup_flag = (int*)arg;
    
    // Simulate work
    usleep(5000);
    
    // Set cleanup flag
    *cleanup_flag = 42;
    
    return cleanup_flag;
}

AsthraTestResult test_concurrency_task_cleanup(AsthraV12TestContext *ctx) {
    // Test task cleanup and resource management
    
    int cleanup_flag = 0;
    TestTask *task = test_spawn(cleanup_test_function, &cleanup_flag);
    
    if (!ASTHRA_TEST_ASSERT(ctx, task != NULL,
                           "Task creation should succeed", "")) {
        return ASTHRA_TEST_FAIL;
    }
    
    test_task_join(task);
    
    // Verify task completed and cleanup occurred
    if (!ASTHRA_TEST_ASSERT(ctx, cleanup_flag == 42,
                           "Cleanup should have occurred, flag: %d", cleanup_flag)) {
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    int status = atomic_load(&task->status);
    if (!ASTHRA_TEST_ASSERT(ctx, status == TASK_STATUS_COMPLETED,
                           "Task should have completed successfully", "")) {
        test_task_destroy(task);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test multiple task cleanup
    const int num_tasks = 10;
    TestTask *tasks[num_tasks];
    int cleanup_flags[num_tasks];
    
    // Create multiple tasks
    for (int i = 0; i < num_tasks; i++) {
        cleanup_flags[i] = 0;
        tasks[i] = test_spawn(cleanup_test_function, &cleanup_flags[i]);
        
        if (!ASTHRA_TEST_ASSERT(ctx, tasks[i] != NULL,
                               "Task %d creation should succeed", i)) {
            // Cleanup created tasks
            for (int j = 0; j < i; j++) {
                test_task_join(tasks[j]);
                test_task_destroy(tasks[j]);
            }
            test_task_destroy(task);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Wait for all tasks to complete
    for (int i = 0; i < num_tasks; i++) {
        test_task_join(tasks[i]);
        
        if (!ASTHRA_TEST_ASSERT(ctx, cleanup_flags[i] == 42,
                               "Task %d cleanup should have occurred", i)) {
            // Cleanup remaining tasks
            for (int j = i; j < num_tasks; j++) {
                test_task_destroy(tasks[j]);
            }
            test_task_destroy(task);
            return ASTHRA_TEST_FAIL;
        }
        
        test_task_destroy(tasks[i]);
    }
    
    test_task_destroy(task);
    return ASTHRA_TEST_PASS;
} 
