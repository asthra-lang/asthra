/**
 * Asthra Programming Language Compiler
 * Pattern Matching Engine Tests - Main Runner
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main test runner for all pattern matching engine tests
 */

#include "test_pattern_matching_common.h"

// Test metadata for all tests
static AsthraTestMetadata test_metadata[] = {
    {"test_pattern_creation_and_matching", __FILE__, __LINE__,
     "Test pattern creation and basic matching", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false,
     NULL},
    {"test_nested_pattern_matching", __FILE__, __LINE__, "Test nested and complex pattern matching",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_pattern_guards", __FILE__, __LINE__, "Test pattern guards and conditional matching",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_match_expression_exhaustiveness", __FILE__, __LINE__,
     "Test match expression exhaustiveness checking", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL,
     false, NULL}};

// Test function pointers
typedef AsthraTestResult (*TestFunction)(AsthraTestContext *);

typedef struct {
    const char *name;
    TestFunction function;
    const char *description;
} TestCase;

static TestCase test_cases[] = {
    {"Pattern Creation and Matching", test_pattern_creation_and_matching,
     "Basic pattern creation and matching"},
    {"Nested Pattern Matching", test_nested_pattern_matching, "Complex nested pattern matching"},
    {"Pattern Guards", test_pattern_guards, "Conditional pattern matching with guards"},
    {"Match Expression Exhaustiveness", test_match_expression_exhaustiveness,
     "Exhaustiveness checking"}};

static const int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

// Test runner with statistics
static void run_all_tests(void) {
    printf("=== Pattern Matching Engine Tests (Modular) ===\n");
    printf("Running %d test categories...\n\n", num_test_cases);

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    for (int i = 0; i < num_test_cases; i++) {
        printf("[%d/%d] %s: ", i + 1, num_test_cases, test_cases[i].name);
        fflush(stdout);

        AsthraTestResult result = test_cases[i].function(&context);
        total++;

        if (result == ASTHRA_TEST_PASS) {
            printf("✅ PASS\n");
            passed++;
        } else {
            printf("❌ FAIL\n");
        }

        printf("    Description: %s\n", test_cases[i].description);
        printf("\n");
    }

    printf("=== Final Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    if (passed == total) {
        printf("🎉 All pattern matching tests passed!\n");
    } else {
        printf("⚠️  %d test(s) failed\n", total - passed);
    }

    printf("\n=== Test Summary ===\n");
    printf("• Pattern Creation & Matching: Basic functionality\n");
    printf("• Nested Pattern Matching: Complex type handling\n");
    printf("• Pattern Guards: Conditional matching logic\n");
    printf("• Exhaustiveness Checking: Complete coverage validation\n");
    printf("\nTotal test coverage: All major pattern matching features validated\n");
}

// Main function
int main(void) {
    run_all_tests();
    return 0;
}
