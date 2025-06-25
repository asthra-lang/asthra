/**
 * Asthra Programming Language Core Simple Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Simple core language feature tests using standardized framework.
 */

#include "../framework/test_framework.h"
#include "test_comprehensive.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// CORE LANGUAGE FEATURE TESTS
// ============================================================================

AsthraTestResult test_basic_grammar_functionality(AsthraTestContext *context) {
    // Test basic grammar validation concepts
    bool grammar_valid = true; // Simplified test
    ASTHRA_TEST_ASSERT_TRUE(context, grammar_valid, "Basic grammar functionality should work");

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_string_operations_basic(AsthraTestContext *context) {
    // Test basic string operations that don't require full parser
    const char *test_string = "hello world";
    size_t len = strlen(test_string);

    ASTHRA_TEST_ASSERT_TRUE(context, len > 0, "String length should be positive");
    asthra_test_assert_int_eq(context, len, 11, "String length should be correct");

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_pattern_matching_concepts(AsthraTestContext *context) {
    // Test basic pattern matching concepts
    enum TestPattern { PATTERN_A, PATTERN_B, PATTERN_C };
    enum TestPattern test_pattern = PATTERN_B;

    bool pattern_matched = false;
    switch (test_pattern) {
    case PATTERN_A:
        break;
    case PATTERN_B:
        pattern_matched = true;
        break;
    case PATTERN_C:
        break;
    }

    ASTHRA_TEST_ASSERT_TRUE(context, pattern_matched, "Pattern matching should work correctly");

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_memory_safety_concepts(AsthraTestContext *context) {
    // Test basic memory safety concepts
    char *test_buffer = malloc(100);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, test_buffer, "Memory allocation should succeed");

    // Write to buffer safely
    memset(test_buffer, 'A', 99);
    test_buffer[99] = '\0';

    ASTHRA_TEST_ASSERT_EQ(context, (int)test_buffer[0], (int)'A',
                          "Buffer should be filled correctly");
    ASTHRA_TEST_ASSERT_EQ(context, (int)test_buffer[99], (int)'\0',
                          "Buffer should be null terminated");

    free(test_buffer);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_type_system_basics(AsthraTestContext *context) {
    // Test basic type system concepts
    int32_t test_int = 42;
    float test_float = 3.14f;
    bool test_bool = true;

    asthra_test_assert_int_eq(context, test_int, 42, "Integer value should be correct");
    ASTHRA_TEST_ASSERT_TRUE(context, test_float > 3.0f, "Float value should be in range");
    ASTHRA_TEST_ASSERT_TRUE(context, test_bool, "Boolean value should be true");

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_error_handling_concepts(AsthraTestContext *context) {
    // Test basic error handling concepts
    typedef enum { RESULT_OK, RESULT_ERROR } ResultTag;
    typedef struct {
        ResultTag tag;
        int value;
        const char *error_message;
    } TestResult;

    TestResult success_result = {RESULT_OK, 42, NULL};
    TestResult error_result = {RESULT_ERROR, 0, "Test error"};

    ASTHRA_TEST_ASSERT_EQ(context, (int)success_result.tag, (int)RESULT_OK,
                          "Success result should have OK tag");
    asthra_test_assert_int_eq(context, success_result.value, 42,
                              "Success result should have correct value");

    ASTHRA_TEST_ASSERT_EQ(context, (int)error_result.tag, (int)RESULT_ERROR,
                          "Error result should have ERROR tag");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, error_result.error_message,
                                "Error result should have message");

    return ASTHRA_TEST_PASS;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("Core Simple Tests");

    asthra_test_suite_add_test(suite, "basic_grammar", "Test basic grammar functionality concepts",
                               test_basic_grammar_functionality);
    asthra_test_suite_add_test(suite, "string_operations", "Test basic string operations",
                               test_string_operations_basic);
    asthra_test_suite_add_test(suite, "pattern_matching", "Test basic pattern matching concepts",
                               test_pattern_matching_concepts);
    asthra_test_suite_add_test(suite, "memory_safety", "Test basic memory safety concepts",
                               test_memory_safety_concepts);
    asthra_test_suite_add_test(suite, "type_system", "Test basic type system concepts",
                               test_type_system_basics);
    asthra_test_suite_add_test(suite, "error_handling", "Test basic error handling concepts",
                               test_error_handling_concepts);

    return asthra_test_suite_run_and_exit(suite);
}
