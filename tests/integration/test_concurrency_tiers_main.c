/*
 * Concurrency Tiers Integration Tests - Main Test Runner
 * 
 * Main entry point for the modular concurrency tiers test suite.
 * Coordinates execution of all test modules and provides overall results.
 * 
 * Phase 8: Testing and Validation
 * Focus: Comprehensive testing of three-tier concurrency system
 */

#include "test_concurrency_tiers_common.h"

// Module test runner declarations
extern void run_tier_progression_tests(void);
extern void run_real_world_scenario_tests(void);
extern void run_performance_pattern_tests(void);
extern void run_migration_pattern_tests(void);
extern void run_error_detection_tests(void);

// ============================================================================
// Main Test Runner
// ============================================================================

int main(void) {
    printf("=== Asthra Concurrency Tiers Integration Tests (Phase 8) ===\n");
    printf("Testing three-tier concurrency system integration:\n");
    printf("- Tier 1 → Tier 2 progression patterns\n");
    printf("- Real-world scenarios (web servers, pipelines, workloads)\n");
    printf("- Performance patterns (CPU/I/O intensive workloads)\n");
    printf("- Migration patterns from other languages\n");
    printf("- Error detection and edge cases\n\n");
    
    // Initialize concurrency runtime
    if (initialize_concurrency_runtime() != 0) {
        return 1;
    }
    
    // Initialize test counters
    tests_run = 0;
    tests_passed = 0;
    
    // Run all test modules
    run_tier_progression_tests();
    run_real_world_scenario_tests();
    run_performance_pattern_tests();
    run_migration_pattern_tests();
    run_error_detection_tests();
    
    // Cleanup concurrency runtime
    cleanup_concurrency_runtime();
    
    // Results
    printf("\n=== Concurrency Tiers Integration Test Results ===\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", (float)tests_passed / tests_run * 100.0);
    
    if (tests_passed == tests_run) {
        printf("✅ All concurrency tiers integration tests passed!\n");
        printf("Three-tier concurrency system is working correctly.\n");
        return 0;
    } else {
        printf("❌ Some integration tests failed. Please review the output above.\n");
        return 1;
    }
} 
