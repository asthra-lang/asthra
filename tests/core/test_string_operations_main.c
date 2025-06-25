/**
 * Asthra Programming Language v1.2 String Operations Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test runner for all string operation tests, including deterministic
 * behavior, performance, memory safety, and FFI integration tests.
 */

#include "test_string_helpers.h"

// =============================================================================
// FUNCTION PROTOTYPES
// =============================================================================

// Main test suite runners
AsthraTestResult run_string_operations_test_suite(void);
AsthraTestResult run_string_category_tests(const char *category);
AsthraTestResult run_string_deterministic_tests(void);
AsthraTestResult run_string_performance_tests(void);
AsthraTestResult run_string_memory_safety_tests(void);
AsthraTestResult run_string_ffi_integration_tests(void);

// Include external test function declarations
// Forward declarations for test functions
extern AsthraTestResult test_string_deterministic_concatenation(AsthraTestContext *ctx);
extern AsthraTestResult test_string_deterministic_interpolation(AsthraTestContext *ctx);
extern AsthraTestResult test_string_cross_platform_consistency(AsthraTestContext *ctx);

// Performance test functions
extern AsthraTestResult test_string_concatenation_performance(AsthraTestContext *ctx);
extern AsthraTestResult test_string_performance_comparison(AsthraTestContext *ctx);

// Memory safety test functions
extern AsthraTestResult test_string_gc_interaction(AsthraTestContext *ctx);
extern AsthraTestResult test_string_memory_management(AsthraTestContext *ctx);
extern AsthraTestResult test_string_large_operations(AsthraTestContext *ctx);

// FFI integration test functions
extern AsthraTestResult test_string_c_string_conversion(AsthraTestContext *ctx);
extern AsthraTestResult test_string_ffi_ownership_transfer(AsthraTestContext *ctx);
extern AsthraTestResult test_string_ffi_borrowed_references(AsthraTestContext *ctx);

// =============================================================================
// TEST CASE DEFINITIONS
// =============================================================================

typedef struct {
    const char *name;
    AsthraTestResult (*function)(AsthraTestContext *ctx);
    const char *category;
    bool is_performance_test;
} StringTestCase;

static const StringTestCase string_test_cases[] = {
    // Deterministic behavior tests
    {"Deterministic Concatenation", test_string_deterministic_concatenation, "Deterministic",
     false},
    {"Deterministic Interpolation", test_string_deterministic_interpolation, "Deterministic",
     false},
    {"Cross-Platform Consistency", test_string_cross_platform_consistency, "Deterministic", false},

    // Performance tests
    {"Concatenation Performance", test_string_concatenation_performance, "Performance", true},
    {"Performance Comparison", test_string_performance_comparison, "Performance", true},

    // Memory safety tests
    {"GC Interaction", test_string_gc_interaction, "Memory Safety", false},
    {"Memory Management", test_string_memory_management, "Memory Safety", false},
    {"Large Operations", test_string_large_operations, "Memory Safety", false},

    // FFI integration tests
    {"C String Conversion", test_string_c_string_conversion, "FFI Integration", false},
    {"FFI Ownership Transfer", test_string_ffi_ownership_transfer, "FFI Integration", false},
    {"FFI Borrowed References", test_string_ffi_borrowed_references, "FFI Integration", false}};

static const size_t string_test_count = sizeof(string_test_cases) / sizeof(string_test_cases[0]);

AsthraTestResult run_string_operations_test_suite(void) {
    printf("=== Asthra v1.2 String Operations Test Suite ===\n");

    AsthraTestResult overall_result = ASTHRA_TEST_PASS;
    int passed_tests = 0;
    int failed_tests = 0;

    for (size_t i = 0; i < string_test_count; i++) {
        const StringTestCase *test_case = &string_test_cases[i];

        printf("\n[%zu/%zu] Running %s (%s)...\n", i + 1, string_test_count, test_case->name,
               test_case->category);

        // Create test metadata
        AsthraTestMetadata metadata = {.name = test_case->name,
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = test_case->name,
                                       .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
                                       .timeout_ns = 30000000000ULL, // 30 seconds
                                       .skip = false,
                                       .skip_reason = NULL};

        // Create test context
        AsthraTestContext ctx = {.metadata = metadata,
                                 .result = ASTHRA_TEST_PASS,
                                 .start_time_ns = 0,
                                 .end_time_ns = 0,
                                 .duration_ns = 0,
                                 .error_message = NULL,
                                 .error_message_allocated = false,
                                 .assertions_in_test = 0,
                                 .global_stats = NULL};

        // Run the test
        AsthraTestResult result = test_case->function(&ctx);

        switch (result) {
        case ASTHRA_TEST_PASS:
            printf("  ✓ PASSED");
            if (test_case->is_performance_test) {
                printf(" (performance test completed)");
            }
            printf("\n");
            passed_tests++;
            break;

        case ASTHRA_TEST_FAIL:
            printf("  ✗ FAILED\n");
            failed_tests++;
            overall_result = ASTHRA_TEST_FAIL;
            break;

        case ASTHRA_TEST_SKIP:
            printf("  - SKIPPED\n");
            break;

        case ASTHRA_TEST_ERROR:
            printf("  ! ERROR\n");
            failed_tests++;
            overall_result = ASTHRA_TEST_ERROR;
            break;

        case ASTHRA_TEST_TIMEOUT:
            printf("  ⏰ TIMEOUT\n");
            failed_tests++;
            overall_result = ASTHRA_TEST_ERROR;
            break;

        case ASTHRA_TEST_RESULT_COUNT:
            // This is not a real result, just a count marker
            printf("  ? INVALID RESULT\n");
            failed_tests++;
            overall_result = ASTHRA_TEST_ERROR;
            break;
        }
    }

    // Print summary
    printf("\n=== String Operations Test Summary ===\n");
    printf("Total Tests: %zu\n", string_test_count);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("Success Rate: %.1f%%\n",
           string_test_count > 0 ? (double)passed_tests / string_test_count * 100.0 : 0.0);

    if (overall_result == ASTHRA_TEST_PASS) {
        printf("🎉 All string operation tests passed!\n");
    } else {
        printf("❌ Some string operation tests failed.\n");
    }

    return overall_result;
}

AsthraTestResult run_string_category_tests(const char *category) {
    printf("=== Running String Operations Tests: %s ===\n", category);

    AsthraTestResult overall_result = ASTHRA_TEST_PASS;
    int tests_run = 0;
    int passed_tests = 0;

    for (size_t i = 0; i < string_test_count; i++) {
        const StringTestCase *test_case = &string_test_cases[i];

        if (strcmp(test_case->category, category) != 0) {
            continue; // Skip tests not in this category
        }

        tests_run++;
        printf("\n[%d] Running %s...\n", tests_run, test_case->name);

        AsthraTestContext ctx = {.metadata = {.name = test_case->name,
                                              .file = __FILE__,
                                              .line = __LINE__,
                                              .function = test_case->name,
                                              .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
                                              .timeout_ns = 30000000000ULL, // 30 seconds
                                              .skip = false,
                                              .skip_reason = NULL},
                                 .result = ASTHRA_TEST_PASS,
                                 .start_time_ns = 0,
                                 .end_time_ns = 0,
                                 .duration_ns = 0,
                                 .error_message = NULL,
                                 .error_message_allocated = false,
                                 .assertions_in_test = 0,
                                 .global_stats = NULL};

        AsthraTestResult result = test_case->function(&ctx);

        if (result == ASTHRA_TEST_PASS) {
            printf("  ✓ PASSED\n");
            passed_tests++;
        } else {
            printf("  ✗ FAILED\n");
            overall_result = ASTHRA_TEST_FAIL;
        }
    }

    printf("\nCategory '%s': %d/%d tests passed\n", category, passed_tests, tests_run);
    return overall_result;
}

// Individual category runners for easier testing
AsthraTestResult run_string_deterministic_tests(void) {
    return run_string_category_tests("Deterministic");
}

AsthraTestResult run_string_performance_tests(void) {
    return run_string_category_tests("Performance");
}

AsthraTestResult run_string_memory_safety_tests(void) {
    return run_string_category_tests("Memory Safety");
}

AsthraTestResult run_string_ffi_integration_tests(void) {
    return run_string_category_tests("FFI Integration");
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    printf("=== Asthra String Operations Test Suite ===\n");

    AsthraTestResult result = run_string_operations_test_suite();

    switch (result) {
    case ASTHRA_TEST_PASS:
        printf("\n🎉 All tests passed!\n");
        return 0;
    case ASTHRA_TEST_FAIL:
        printf("\n❌ Some tests failed.\n");
        return 1;
    case ASTHRA_TEST_ERROR:
        printf("\n💥 Test execution error.\n");
        return 1;
    case ASTHRA_TEST_SKIP:
        printf("\n⏭️  Tests were skipped.\n");
        return 0;
    default:
        printf("\n❓ Unknown test result.\n");
        return 1;
    }
}
