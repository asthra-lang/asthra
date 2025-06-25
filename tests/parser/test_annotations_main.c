/*
 * Annotation Tests - Main Test Runner
 * Orchestrates all annotation test modules
 *
 * Coordinates:
 * - Basic annotation parsing tests
 * - Annotation validation tests
 * - Complex annotation scenarios
 * - Error detection tests
 * - Performance and edge cases
 * - Comprehensive results reporting
 */

#include "test_annotations_common.h"

// ============================================================================
// Module Test Runners (declared in common header)
// ============================================================================

// External module test runners
extern void run_basic_annotation_tests(void);
extern void run_validation_annotation_tests(void);
extern void run_complex_annotation_tests(void);
extern void run_error_annotation_tests(void);
extern void run_performance_annotation_tests(void);

// ============================================================================
// Main Test Runner
// ============================================================================

int main(void) {
    printf("=== Asthra Annotation Tests (Modular Suite) ===\n");
    printf("Testing #[non_deterministic] annotation system:\n");
    printf("- Annotation parsing and validation\n");
    printf("- Tier 2 feature requirement enforcement\n");
    printf("- Complex annotation scenarios\n");
    printf("- Error detection and edge cases\n");
    printf("- Performance and edge case handling\n\n");

    // Initialize test counters
    init_test_counters();

    // Run all test modules
    printf("=== Running Modular Annotation Test Suite ===\n\n");

    // Basic Annotation Parsing Tests
    run_basic_annotation_tests();
    printf("\n");

    // Annotation Validation Tests
    run_validation_annotation_tests();
    printf("\n");

    // Complex Annotation Scenarios
    run_complex_annotation_tests();
    printf("\n");

    // Error Detection Tests
    run_error_annotation_tests();
    printf("\n");

    // Performance and Edge Cases
    run_performance_annotation_tests();
    printf("\n");

    // Final Results Summary
    int total_tests, passed_tests;
    get_test_stats(&total_tests, &passed_tests);

    printf("=== Final Annotation Test Results ===\n");
    printf("Total tests run: %d\n", total_tests);
    printf("Total tests passed: %d\n", passed_tests);
    printf("Total tests failed: %d\n", total_tests - passed_tests);

    if (total_tests > 0) {
        printf("Overall success rate: %.1f%%\n", (float)passed_tests / total_tests * 100.0);
    } else {
        printf("Overall success rate: N/A (no tests run)\n");
    }

    if (passed_tests == total_tests && total_tests > 0) {
        printf("✅ All annotation tests passed!\n");
        printf("#[non_deterministic] annotation system is working correctly.\n");
        printf("\nModular test suite benefits:\n");
        printf("- Enhanced maintainability through focused modules\n");
        printf("- Superior debugging with isolated test execution\n");
        printf("- Scalable development with parallel workflows\n");
        printf("- Comprehensive coverage across all annotation features\n");
        return 0;
    } else if (total_tests > 0) {
        printf("❌ Some annotation tests failed. Please review the output above.\n");
        printf("\nModular test suite provides:\n");
        printf("- Isolated failure analysis per module\n");
        printf("- Focused debugging capabilities\n");
        printf("- Clear separation of test concerns\n");
        return 1;
    } else {
        printf("⚠️  No annotation tests were run.\n");
        return 1;
    }
}
