/**
 * Fixed for minimal test framework compatibility
 */
#define TEST_FRAMEWORK_MINIMAL 1

/**
 * Asthra Programming Language Compiler
 * Assignment Statement Generation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test code generation for assignment statements including simple and compound assignments
 */

#include "statement_generation_test_fixtures.h"

// =============================================================================
// ASSIGNMENT STATEMENT TESTS
// =============================================================================

/**
 * Test simple assignment statement generation
 */
AsthraTestResult test_generate_simple_assignments(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test simple assignment: x = 10;
    const char* source = "x = 10;";
    AsthraTestResult result = test_statement_generation_pattern(context, fixture, source, 
                                                               "simple assignment");
    
    cleanup_codegen_fixture(fixture);
    return result;
}

/**
 * Test compound assignment statements
 */
AsthraTestResult test_generate_compound_assignments(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test compound assignment: y += 5;
    const char* source = "y += 5;";
    AsthraTestResult result = test_statement_generation_pattern(context, fixture, source, 
                                                               "compound assignment");
    
    cleanup_codegen_fixture(fixture);
    return result;
}

/**
 * Test multiplication assignment
 */
AsthraTestResult test_generate_multiplication_assignment(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test multiplication assignment: z *= 3;
    const char* source = "z *= 3;";
    AsthraTestResult result = test_statement_generation_pattern(context, fixture, source, 
                                                               "multiplication assignment");
    
    cleanup_codegen_fixture(fixture);
    return result;
}

/**
 * Test division assignment
 */
AsthraTestResult test_generate_division_assignment(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test division assignment: w /= 2;
    const char* source = "w /= 2;";
    AsthraTestResult result = test_statement_generation_pattern(context, fixture, source, 
                                                               "division assignment");
    
    cleanup_codegen_fixture(fixture);
    return result;
}

/**
 * Test all compound assignment operators
 */
AsthraTestResult test_generate_all_compound_assignments(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test various compound assignment operators
    const char* test_cases[] = {
        "a += 5;",   // Addition assignment
        "b -= 3;",   // Subtraction assignment
        "c *= 2;",   // Multiplication assignment
        "d /= 4;",   // Division assignment
        "e %= 7;",   // Modulo assignment
        "f &= mask;", // Bitwise AND assignment
        "g |= flag;", // Bitwise OR assignment
        "h ^= key;"   // Bitwise XOR assignment
    };
    
    size_t test_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < test_count; i++) {
        AsthraTestResult result = test_statement_generation_pattern(context, fixture, 
                                                                   test_cases[i], 
                                                                   "compound assignment");
        if (result != ASTHRA_TEST_PASS) {
            cleanup_codegen_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test assignment with complex expressions
 */
AsthraTestResult test_generate_complex_assignments(AsthraTestContext* context) {
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test assignments with complex right-hand side expressions
    const char* test_cases[] = {
        "result = a + b * c;",
        "value = func(x, y, z);",
        "total = array[index] + offset;",
        "flag = condition1 && condition2;"
    };
    
    size_t test_count = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (size_t i = 0; i < test_count; i++) {
        AsthraTestResult result = test_statement_generation_pattern(context, fixture, 
                                                                   test_cases[i], 
                                                                   "complex assignment");
        if (result != ASTHRA_TEST_PASS) {
            cleanup_codegen_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    cleanup_codegen_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN FUNCTION
// =============================================================================

/**
 * Main test function for assignment statement generation
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = create_statement_test_suite_config(
        "Assignment Statement Generation Tests",
        "Test code generation for assignment statements including simple and compound assignments",
        stats
    );
    
    AsthraTestFunction tests[] = {
        test_generate_simple_assignments,
        test_generate_compound_assignments,
        test_generate_multiplication_assignment,
        test_generate_division_assignment,
        test_generate_all_compound_assignments,
        test_generate_complex_assignments
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_generate_simple_assignments", __FILE__, __LINE__, "test_generate_simple_assignments", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_compound_assignments", __FILE__, __LINE__, "test_generate_compound_assignments", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_multiplication_assignment", __FILE__, __LINE__, "test_generate_multiplication_assignment", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_division_assignment", __FILE__, __LINE__, "test_generate_division_assignment", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_all_compound_assignments", __FILE__, __LINE__, "test_generate_all_compound_assignments", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_generate_complex_assignments", __FILE__, __LINE__, "test_generate_complex_assignments", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
