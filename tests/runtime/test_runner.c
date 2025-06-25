/**
 * Asthra Programming Language Runtime v1.2
 * Main Test Runner for Split Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This test runner coordinates execution of all split test modules
 * to validate the complete runtime functionality.
 */

#include "test_common.h"

// Include test function declarations
extern int test_runtime_initialization(void);
extern int test_runtime_custom_config(void);
extern int test_memory_zones(void);
extern int test_gc_operations(void);
extern int test_slice_operations(void);
extern int test_string_operations(void);
// String interpolation test removed - feature deprecated for AI generation efficiency
extern int test_result_pattern_matching(void);
extern int test_task_system(void);
extern int test_cryptographic_primitives(void);
extern int test_synchronization_primitives(void);
extern int test_error_handling(void);
extern int test_observability(void);

// Test registry structure
typedef struct {
    const char *name;
    int (*test_func)(void);
    const char *category;
} TestEntry;

// Test registry
static TestEntry test_registry[] = {
    // Runtime initialization tests
    {"Runtime Initialization", test_runtime_initialization, "Init"},
    {"Runtime Custom Config", test_runtime_custom_config, "Init"},

    // Memory management tests
    {"Memory Zones", test_memory_zones, "Memory"},
    {"GC Operations", test_gc_operations, "Memory"},

    // Slice management tests
    {"Slice Operations", test_slice_operations, "Slices"},

    // String operations tests
    {"String Operations", test_string_operations, "Strings"},

    // Result type and pattern matching tests
    {"Result Pattern Matching", test_result_pattern_matching, "Patterns"},

    // Task system tests
    {"Task System", test_task_system, "Tasks"},

    // Cryptographic primitives tests
    {"Cryptographic Primitives", test_cryptographic_primitives, "Crypto"},

    // Synchronization primitives tests
    {"Synchronization Primitives", test_synchronization_primitives, "Sync"},

    // Error handling tests
    {"Error Handling", test_error_handling, "Errors"},

    // Observability tests
    {"Observability", test_observability, "Observability"}};

static const size_t test_count = sizeof(test_registry) / sizeof(TestEntry);

int main(void) {
    printf("Asthra Runtime v1.2 - Comprehensive Test Suite (Split)\n");
    printf("====================================================\n\n");

    int passed = 0;
    int total = 0;
    const char *current_category = "";

    for (size_t i = 0; i < test_count; i++) {
        TestEntry *test = &test_registry[i];

        // Print category header if changed
        if (strcmp(current_category, test->category) != 0) {
            if (i > 0)
                printf("\n");
            printf("[%s Tests]\n", test->category);
            printf("--------------------\n");
            current_category = test->category;
        }

        printf("Running: %s... ", test->name);
        fflush(stdout);

        total++;
        if (test->test_func()) {
            passed++;
        }
    }

    printf("\n====================================================\n");
    printf("Test Results: %d/%d passed (%.1f%%)\n", passed, total, (float)passed / total * 100.0f);

    if (passed == total) {
        printf("🎉 All tests passed! Runtime is working correctly.\n");
        return 0;
    } else {
        printf("❌ Some tests failed. Please check the implementation.\n");
        return 1;
    }
}
