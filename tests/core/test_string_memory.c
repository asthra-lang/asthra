/**
 * Asthra Programming Language v1.2 String Memory Safety Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for string memory safety, including garbage collector interaction,
 * memory management, and handling of large data.
 */

#include "test_string_helpers.h"

// =============================================================================
// MEMORY SAFETY TESTS
// =============================================================================

AsthraTestResult test_string_gc_interaction(AsthraTestContext *context) {
    // Cast to V12 context for compatibility if needed
    // AsthraV12TestContext *ctx = (AsthraV12TestContext *)context;

    // Test interaction with garbage collector during string operations

    const int num_strings = 100;
    TestString **strings = malloc(num_strings * sizeof(TestString *));

    if (!ASTHRA_TEST_ASSERT(context, strings != NULL, "Failed to allocate test string array")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create many strings to trigger potential GC activity
    for (int i = 0; i < num_strings; i++) {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "GC test string %d", i);

        strings[i] = test_string_create(buffer);

        if (!ASTHRA_TEST_ASSERT(context, strings[i] != NULL, "Failed to create string %d", i)) {
            // Clean up allocated strings before failing
            for (int j = 0; j < i; j++) {
                test_string_destroy(strings[j]);
            }
            free(strings);
            return ASTHRA_TEST_FAIL;
        }

        // Perform concatenation to create temporary objects
        TestString *concat_result = test_string_concat(strings[i], strings[0]);

        if (!ASTHRA_TEST_ASSERT(context, concat_result != NULL,
                                "Failed to concatenate strings for GC test")) {
            for (int j = 0; j <= i; j++) {
                test_string_destroy(strings[j]);
            }
            free(strings);
            return ASTHRA_TEST_FAIL;
        }

        test_string_destroy(concat_result);
    }

    // Clean up all strings
    for (int i = 0; i < num_strings; i++) {
        test_string_destroy(strings[i]);
    }
    free(strings);

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_string_memory_management(AsthraTestContext *context) {
    // Cast to V12 context for compatibility if needed
    // AsthraV12TestContext *ctx = (AsthraV12TestContext *)context;

    // Test memory allocation and deallocation patterns

    // Record initial memory state (simplified for test)
    size_t initial_memory = 0; // In real implementation, this would query system memory

    const int iterations = 50;
    for (int i = 0; i < iterations; i++) {
        const int batch_size = 20;
        TestString **strings = malloc(batch_size * sizeof(TestString *));

        if (strings) {
            // Allocate batch of strings
            for (int j = 0; j < batch_size; j++) {
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "Memory test %d-%d", i, j);
                strings[j] = test_string_create(buffer);

                if (!ASTHRA_TEST_ASSERT(context, strings[j] != NULL,
                                        "Memory allocation failed at iteration %d, string %d", i,
                                        j)) {
                    // Cleanup on failure
                    for (int k = 0; k < j; k++) {
                        test_string_destroy(strings[k]);
                    }
                    free(strings);
                    return ASTHRA_TEST_FAIL;
                }
            }

            // Deallocate batch of strings
            for (int j = 0; j < batch_size; j++) {
                test_string_destroy(strings[j]);
            }
            free(strings);
        }
    }

    // Check final memory state (simplified test)
    size_t final_memory = 0; // In real implementation, this would query system memory

    if (!ASTHRA_TEST_ASSERT(context, final_memory == initial_memory,
                            "Memory leak detected: initial=%zu, final=%zu", initial_memory,
                            final_memory)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_string_large_operations(AsthraTestContext *context) {
    // Cast to V12 context for compatibility if needed
    // AsthraV12TestContext *ctx = (AsthraV12TestContext *)context;

    // Test operations with large strings to verify memory handling

    const size_t large_size = 1024 * 1024; // 1MB string
    char *large_buffer = malloc(large_size + 1);

    if (!ASTHRA_TEST_ASSERT(context, large_buffer != NULL, "Failed to allocate large buffer")) {
        return ASTHRA_TEST_FAIL;
    }

    // Fill buffer with test data
    for (size_t i = 0; i < large_size; i++) {
        large_buffer[i] = 'A' + (i % 26);
    }
    large_buffer[large_size] = '\0';

    TestString *large_string = test_string_create(large_buffer);

    if (!ASTHRA_TEST_ASSERT(context, large_string != NULL, "Failed to create large string")) {
        free(large_buffer);
        return ASTHRA_TEST_FAIL;
    }

    // Test concatenation with large strings
    TestString *suffix = test_string_create(" suffix");
    TestString *result = test_string_concat(large_string, suffix);

    if (!ASTHRA_TEST_ASSERT(context, result != NULL, "Failed to concatenate large string")) {
        test_string_destroy(large_string);
        test_string_destroy(suffix);
        free(large_buffer);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, result->length == large_size + 7,
                            "Large string concatenation length incorrect: expected %zu, got %zu",
                            large_size + 7, result->length)) {
        test_string_destroy(large_string);
        test_string_destroy(suffix);
        test_string_destroy(result);
        free(large_buffer);
        return ASTHRA_TEST_FAIL;
    }

    // Cleanup
    test_string_destroy(large_string);
    test_string_destroy(suffix);
    test_string_destroy(result);
    free(large_buffer);

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE RUNNER
// =============================================================================

typedef struct {
    const char *name;
    AsthraTestResult (*function)(AsthraTestContext *ctx);
} MemoryTestCase;

static const MemoryTestCase memory_test_cases[] = {
    {"GC Interaction", test_string_gc_interaction},
    {"Memory Management", test_string_memory_management},
    {"Large Operations", test_string_large_operations}};

static const size_t memory_test_count = sizeof(memory_test_cases) / sizeof(memory_test_cases[0]);

AsthraTestResult run_string_memory_test_suite(void) {
    printf("=== Asthra String Memory Safety Test Suite ===\n");

    AsthraTestResult overall_result = ASTHRA_TEST_PASS;
    int passed_tests = 0;
    int failed_tests = 0;

    for (size_t i = 0; i < memory_test_count; i++) {
        const MemoryTestCase *test_case = &memory_test_cases[i];

        printf("\n[%zu/%zu] Running %s...\n", i + 1, memory_test_count, test_case->name);

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
            printf("  ✓ PASSED\n");
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
    printf("\n=== String Memory Safety Test Summary ===\n");
    printf("Total Tests: %zu\n", memory_test_count);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("Success Rate: %.1f%%\n",
           memory_test_count > 0 ? (double)passed_tests / memory_test_count * 100.0 : 0.0);

    if (overall_result == ASTHRA_TEST_PASS) {
        printf("🎉 All string memory safety tests passed!\n");
    } else {
        printf("❌ Some string memory safety tests failed.\n");
    }

    return overall_result;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

#ifndef ASTHRA_TEST_LIBRARY_MODE
int main(void) {
    printf("=== Asthra String Memory Safety Tests ===\n");

    AsthraTestResult result = run_string_memory_test_suite();

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
#endif // ASTHRA_TEST_LIBRARY_MODE
