/*
 * Main Test Runner for Generic Structs Phase 5 Integration Tests
 * Orchestrates all test modules and provides comprehensive reporting
 *
 * Part of test_generic_structs_phase5.c split (580 lines -> 6 focused modules)
 * Coordinates: basic integration, advanced integration, performance/consistency tests
 */

#include "test_generic_structs_phase5_common.h"

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("Phase 5: Generic Structs Integration Testing Suite\n");
    printf("=============================================================================\n");

    // Initialize test statistics
    tests_run = 0;
    tests_passed = 0;

    // Run all integration tests
    printf("\n--- BASIC INTEGRATION TESTS ---\n");
    test_basic_generic_struct_e2e();
    test_multiple_type_parameters_integration();
    test_nested_generic_types_integration();

    printf("\n--- ADVANCED INTEGRATION TESTS ---\n");
    test_generic_struct_with_methods_integration();
    test_error_handling_invalid_generic_usage();

    printf("\n--- PERFORMANCE AND CONSISTENCY TESTS ---\n");
    test_deduplication_integration();
    test_performance_many_instantiations();
    test_type_system_consistency();

    // Print comprehensive summary
    printf("\n=============================================================================\n");
    printf("Integration Test Summary: %d/%d tests passed (%.1f%%)\n", tests_passed, tests_run,
           tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    printf("=============================================================================\n");

    // Detailed results analysis
    if (tests_passed == tests_run) {
        printf("🎉 All integration tests passed! Generic structs work end-to-end.\n");
        printf("\n✅ ACHIEVEMENTS:\n");
        printf("   • Basic generic struct compilation pipeline working\n");
        printf("   • Multiple type parameters support validated\n");
        printf("   • Nested generic types integration confirmed\n");
        printf("   • Generic struct methods compilation verified\n");
        printf("   • Error handling for invalid usage working\n");
        printf("   • Deduplication system preventing code bloat\n");
        printf("   • Performance with many instantiations acceptable\n");
        printf("   • Type system consistency between enums and structs\n");
        printf("\n🚀 READY FOR PRODUCTION: Generic structs implementation complete!\n");
        return 0;
    } else {
        printf("❌ Some integration tests failed. Check the implementation.\n");
        printf("\n🔍 FAILURE ANALYSIS:\n");
        printf("   • Failed tests: %d/%d\n", tests_run - tests_passed, tests_run);
        printf("   • Success rate: %.1f%%\n",
               tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);

        if (tests_passed > 0) {
            printf("   • Partial functionality working - investigate specific failures\n");
        } else {
            printf("   • Complete failure - check basic compilation pipeline\n");
        }

        printf("\n📋 NEXT STEPS:\n");
        printf("   1. Review error messages from failed tests\n");
        printf("   2. Check parser implementation for generic struct support\n");
        printf("   3. Verify semantic analysis handles generic type parameters\n");
        printf("   4. Validate code generation for generic instantiations\n");
        printf("   5. Test individual compilation pipeline components\n");

        return 1;
    }
}
