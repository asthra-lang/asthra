/**
 * Asthra Programming Language v1.2 String Deterministic Behavior Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for deterministic string operations, including concatenation,
 * interpolation, and cross-platform consistency.
 */

#include "test_string_helpers.h"

// =============================================================================
// DETERMINISTIC BEHAVIOR TESTS
// =============================================================================

AsthraTestResult test_string_deterministic_concatenation(AsthraTestContext *context) {
    // Cast to V12 context for compatibility if needed
    // AsthraV12TestContext *ctx = (AsthraV12TestContext *)context;
    
    // Test that string concatenation produces deterministic results
    
    const char *test_cases[][3] = {
        {"Hello", " World", "Hello World"},
        {"", "test", "test"},
        {"test", "", "test"},
        {"αβγ", "δεζ", "αβγδεζ"},  // Unicode test
        {"123", "456", "123456"},
        {NULL, NULL, NULL}
    };
    
    for (int i = 0; test_cases[i][0] != NULL; i++) {
        const char *str1 = test_cases[i][0];
        const char *str2 = test_cases[i][1];
        const char *expected = test_cases[i][2];
        
        // Perform concatenation multiple times to verify determinism
        for (int trial = 0; trial < 5; trial++) {
            TestString *ts1 = test_string_create(str1);
            TestString *ts2 = test_string_create(str2);
            
            if (!ASTHRA_TEST_ASSERT(context, ts1 != NULL && ts2 != NULL,
                                   "Failed to create test strings for case %d, trial %d", i, trial)) {
                if (ts1) test_string_destroy(ts1);
                if (ts2) test_string_destroy(ts2);
                return ASTHRA_TEST_FAIL;
            }
            
            TestString *result = test_string_concat(ts1, ts2);
            
            if (!ASTHRA_TEST_ASSERT(context, result != NULL,
                                   "Concatenation failed for case %d, trial %d", i, trial)) {
                test_string_destroy(ts1);
                test_string_destroy(ts2);
                return ASTHRA_TEST_FAIL;
            }
            
            if (!ASTHRA_TEST_ASSERT(context, strcmp(result->data, expected) == 0,
                                   "Concatenation result incorrect for case %d, trial %d: "
                                   "expected '%s', got '%s'", i, trial, expected, result->data)) {
                test_string_destroy(ts1);
                test_string_destroy(ts2);
                test_string_destroy(result);
                return ASTHRA_TEST_FAIL;
            }
            
            if (!ASTHRA_TEST_ASSERT(context, result->length == strlen(expected),
                                   "Concatenation length incorrect for case %d, trial %d: "
                                   "expected %zu, got %zu", i, trial, strlen(expected), result->length)) {
                test_string_destroy(ts1);
                test_string_destroy(ts2);
                test_string_destroy(result);
                return ASTHRA_TEST_FAIL;
            }
            
            test_string_destroy(ts1);
            test_string_destroy(ts2);
            test_string_destroy(result);
        }
    }
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_string_deterministic_interpolation(AsthraTestContext *context) {
    // Cast to V12 context for compatibility if needed
    // AsthraV12TestContext *ctx = (AsthraV12TestContext *)context;
    
    // Test deterministic string interpolation behavior
    // (Simulated through multiple concatenations)
    
    const char *templates[][4] = {
        {"Hello, ", "John", "! You have ", " messages."},
        {"User ", "admin", " logged in at ", "12:00"},
        {"Processing ", "100", " items in ", " seconds"},
        {NULL, NULL, NULL, NULL}
    };
    
    const char *variables[][2] = {
        {"5"},
        {"2023-12-01"},
        {"3.14"},
        {NULL}
    };
    
    for (int template_idx = 0; templates[template_idx][0] != NULL; template_idx++) {
        for (int var_idx = 0; variables[var_idx][0] != NULL; var_idx++) {
            // Build expected result string
            char expected[256];
            snprintf(expected, sizeof(expected), "%s%s%s%s", 
                    templates[template_idx][0],
                    templates[template_idx][1],
                    templates[template_idx][2],
                    variables[var_idx][0]);
            
            // Test multiple times for determinism
            char first_result[256] = {0};
            
            for (int trial = 0; trial < 3; trial++) {
                TestString *part1 = test_string_create(templates[template_idx][0]);
                TestString *part2 = test_string_create(templates[template_idx][1]);
                TestString *part3 = test_string_create(templates[template_idx][2]);
                TestString *var = test_string_create(variables[var_idx][0]);
                
                if (part1 && part2 && part3 && var) {
                    TestString *temp1 = test_string_concat(part1, part2);
                    if (temp1) {
                        TestString *temp2 = test_string_concat(temp1, part3);
                        if (temp2) {
                            TestString *result = test_string_concat(temp2, var);
                            
                            if (!ASTHRA_TEST_ASSERT(context, result != NULL,
                                                   "Interpolation failed for template %d, var %d, trial %d", 
                                                   template_idx, var_idx, trial)) {
                                test_string_destroy(part1);
                                test_string_destroy(part2);
                                test_string_destroy(part3);
                                test_string_destroy(var);
                                test_string_destroy(temp1);
                                test_string_destroy(temp2);
                                return ASTHRA_TEST_FAIL;
                            }
                            
                            if (trial == 0) {
                                strcpy(first_result, result->data);
                            } else {
                                if (!ASTHRA_TEST_ASSERT(context, strcmp(result->data, first_result) == 0,
                                                       "Interpolation result not deterministic for template %d, var %d", 
                                                       template_idx, var_idx)) {
                                    test_string_destroy(part1);
                                    test_string_destroy(part2);
                                    test_string_destroy(part3);
                                    test_string_destroy(var);
                                    test_string_destroy(temp1);
                                    test_string_destroy(temp2);
                                    test_string_destroy(result);
                                    return ASTHRA_TEST_FAIL;
                                }
                            }
                            
                            test_string_destroy(result);
                            test_string_destroy(temp2);
                        }
                        test_string_destroy(temp1);
                    }
                }
                
                test_string_destroy(part1);
                test_string_destroy(part2);
                test_string_destroy(part3);
                test_string_destroy(var);
            }
        }
    }
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_string_cross_platform_consistency(AsthraTestContext *context) {
    // Cast to V12 context for compatibility if needed
    // AsthraV12TestContext *ctx = (AsthraV12TestContext *)context;
    
    // Test that string operations produce consistent results across platforms
    
    const char *unicode_strings[] = {
        "Hello, 世界",  // Mixed ASCII and Chinese
        "Café résumé",  // French accents
        "Москва",       // Cyrillic
        "🌟🎉🚀",      // Emojis
        NULL
    };
    
    // Test string creation consistency
    for (int i = 0; unicode_strings[i] != NULL; i++) {
        TestString *ts = test_string_create(unicode_strings[i]);
        
        if (!ASTHRA_TEST_ASSERT(context, ts != NULL,
                               "Failed to create Unicode string %d", i)) {
            return ASTHRA_TEST_FAIL;
        }
        
        if (!ASTHRA_TEST_ASSERT(context, strcmp(ts->data, unicode_strings[i]) == 0,
                               "Unicode string %d content mismatch", i)) {
            test_string_destroy(ts);
            return ASTHRA_TEST_FAIL;
        }
        
        test_string_destroy(ts);
    }
    
    // Test concatenation with mixed Unicode
    const char *mixed_parts[][2] = {
        {"ASCII", "世界"},
        {"🌟", "text"},
        {"café", "モンド"},
        {NULL, NULL}
    };
    
    for (int i = 0; mixed_parts[i][0] != NULL; i++) {
        TestString *part1 = test_string_create(mixed_parts[i][0]);
        TestString *part2 = test_string_create(mixed_parts[i][1]);
        
        if (part1 && part2) {
            TestString *result = test_string_concat(part1, part2);
            
            if (!ASTHRA_TEST_ASSERT(context, result != NULL,
                                   "Mixed Unicode concatenation failed for case %d", i)) {
                test_string_destroy(part1);
                test_string_destroy(part2);
                return ASTHRA_TEST_FAIL;
            }
            
            test_string_destroy(result);
        }
        
        test_string_destroy(part1);
        test_string_destroy(part2);
    }
    
    // Test line ending consistency
    const char *line_endings[] = {
        "Line 1\nLine 2",      // Unix
        "Line 1\r\nLine 2",    // Windows
        "Line 1\rLine 2",      // Old Mac
        NULL
    };
    
    for (int i = 0; line_endings[i] != NULL; i++) {
        TestString *ts = test_string_create(line_endings[i]);
        
        if (!ASTHRA_TEST_ASSERT(context, ts != NULL,
                               "Failed to create line ending test string %d", i)) {
            return ASTHRA_TEST_FAIL;
        }
        
        if (!ASTHRA_TEST_ASSERT(context, strcmp(ts->data, line_endings[i]) == 0,
                               "Line ending string %d content mismatch", i)) {
            test_string_destroy(ts);
            return ASTHRA_TEST_FAIL;
        }
        
        test_string_destroy(ts);
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE RUNNER
// =============================================================================

typedef struct {
    const char *name;
    AsthraTestResult (*function)(AsthraTestContext *ctx);
} DeterministicTestCase;

static const DeterministicTestCase deterministic_test_cases[] = {
    {"Deterministic Concatenation", test_string_deterministic_concatenation},
    {"Deterministic Interpolation", test_string_deterministic_interpolation},
    {"Cross-Platform Consistency", test_string_cross_platform_consistency}
};

static const size_t deterministic_test_count = sizeof(deterministic_test_cases) / sizeof(deterministic_test_cases[0]);

AsthraTestResult run_string_deterministic_test_suite(void) {
    printf("=== Asthra String Deterministic Test Suite ===\n");
    
    AsthraTestResult overall_result = ASTHRA_TEST_PASS;
    int passed_tests = 0;
    int failed_tests = 0;
    
    for (size_t i = 0; i < deterministic_test_count; i++) {
        const DeterministicTestCase *test_case = &deterministic_test_cases[i];
        
        printf("\n[%zu/%zu] Running %s...\n", 
               i + 1, deterministic_test_count, test_case->name);
        
        // Create test metadata
        AsthraTestMetadata metadata = {
            .name = test_case->name,
            .file = __FILE__,
            .line = __LINE__,
            .function = test_case->name,
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
            .timeout_ns = 30000000000ULL,  // 30 seconds
            .skip = false,
            .skip_reason = NULL
        };
        
        // Create test context
        AsthraTestContext ctx = {
            .metadata = metadata,
            .result = ASTHRA_TEST_PASS,
            .start_time_ns = 0,
            .end_time_ns = 0,
            .duration_ns = 0,
            .error_message = NULL,
            .error_message_allocated = false,
            .assertions_in_test = 0,
            .global_stats = NULL
        };
        
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
    printf("\n=== String Deterministic Test Summary ===\n");
    printf("Total Tests: %zu\n", deterministic_test_count);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", failed_tests);
    printf("Success Rate: %.1f%%\n", 
           deterministic_test_count > 0 ? (double)passed_tests / deterministic_test_count * 100.0 : 0.0);
    
    if (overall_result == ASTHRA_TEST_PASS) {
        printf("🎉 All string deterministic tests passed!\n");
    } else {
        printf("❌ Some string deterministic tests failed.\n");
    }
    
    return overall_result;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

#ifndef ASTHRA_TEST_LIBRARY_MODE
int main(void) {
    printf("=== Asthra String Deterministic Tests ===\n");
    
    AsthraTestResult result = run_string_deterministic_test_suite();
    
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
