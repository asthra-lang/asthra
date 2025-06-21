/**
 * Asthra Enhanced Concurrency Bridge Test Suite - Main Test Runner
 * Orchestrates and executes all bridge test modules
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "bridge_test_common.h"

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=== Asthra Enhanced Concurrency Bridge Test Suite ===\n\n");
    
    // Initialization tests
    RUN_TEST(test_bridge_initialization);
    RUN_TEST(test_bridge_cleanup);
    
    // Task management tests
    RUN_TEST(test_task_spawn_and_completion);
    RUN_TEST(test_task_failure_handling);
    RUN_TEST(test_task_timeout);
    RUN_TEST(test_task_spawn_options);
    
    // Callback system tests
    RUN_TEST(test_callback_enqueue_and_process);
    RUN_TEST(test_callback_priority_ordering);
    RUN_TEST(test_callback_queue_limits);
    
    // Thread registration tests
    RUN_TEST(test_thread_registration);
    RUN_TEST(test_multiple_thread_registration);
    
    // Synchronization tests
    RUN_TEST(test_mutex_basic_operations);
    RUN_TEST(test_recursive_mutex);
    
    // Integration tests
    RUN_TEST(test_concurrent_task_synchronization);
    
    // Statistics and monitoring tests
    RUN_TEST(test_statistics_tracking);
    RUN_TEST(test_state_dump);
    
    // Summary
    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", 
           tests_run > 0 ? (double)tests_passed / tests_run * 100.0 : 0.0);
    
    return (tests_passed == tests_run) ? 0 : 1;
} 
