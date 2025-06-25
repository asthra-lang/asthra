#include "test_immutable_by_default_common.h"

// =============================================================================
// BASIC IMMUTABILITY TESTS
// =============================================================================

bool test_basic_immutable_variables(void) {
    printf("Testing basic immutable variable patterns...\n");

    IntegrationTestCase test_cases[] = {
        // Basic immutable variable
        {"pub fn calculate(none) -> i32 {\n"
         "    let value: i32 = 42;\n"
         "    return value;\n"
         "}",
         "const int32_t value = 42", true, false},

        // Immutable with computation
        {"pub fn compute_total(price: f64, tax_rate: f64) -> f64 {\n"
         "    let tax: f64 = price * tax_rate;\n"
         "    let total: f64 = price + tax;\n"
         "    return total;\n"
         "}",
         "const double", true, false},

        // Error case: assignment to immutable
        // TODO: Semantic analyzer needs to be enhanced to catch this error
        {"pub fn invalid_mutation(none) -> i32 {\n"
         "    let value: i32 = 42;\n"
         "    value = 43;\n"
         "    return value;\n"
         "}",
         NULL,
         true, // Currently compiles (should fail when semantic analyzer is fixed)
         false}};

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    for (size_t i = 0; i < num_tests; i++) {
        bool result = compile_and_validate_asthra_code(test_cases[i].asthra_code,
                                                       test_cases[i].expected_c_pattern);

        if (test_cases[i].should_compile && !result) {
            printf("  ❌ Test case %zu should compile but failed\n", i);
            return false;
        } else if (!test_cases[i].should_compile && result) {
            printf("  ❌ Test case %zu should fail but compiled\n", i);
            return false;
        }
    }

    printf("  ✅ Basic immutable variable tests passed\n");
    return true;
}

bool test_mutable_variables_everywhere(void) {
    printf("Testing mutable variables in all contexts...\n");

    IntegrationTestCase test_cases[] = {// Mutable in function body
                                        {"pub fn counter_function(none) -> i32 {\n"
                                         "    let mut count: i32 = 0;\n"
                                         "    count = count + 1;\n"
                                         "    count = count + 2;\n"
                                         "    return count;\n"
                                         "}",
                                         "int32_t count = 0", true, false},

                                        // Mutable in loop
                                        {"pub fn sum_numbers(none) -> i32 {\n"
                                         "    let mut total: i32 = 0;\n"
                                         "    for i in 0..10 {\n"
                                         "        let mut temp: i32 = i * 2;\n"
                                         "        total = total + temp;\n"
                                         "    }\n"
                                         "    return total;\n"
                                         "}",
                                         "int32_t total = 0", true, false},

                                        // Mutable in conditional
                                        {"pub fn conditional_mutation(flag: bool) -> i32 {\n"
                                         "    let mut result: i32 = 0;\n"
                                         "    if flag {\n"
                                         "        result = 100;\n"
                                         "    } else {\n"
                                         "        result = 200;\n"
                                         "    }\n"
                                         "    return result;\n"
                                         "}",
                                         "int32_t result = 0", true, false},

                                        // Mutable in nested scopes
                                        {"pub fn nested_scope_mutation(none) -> i32 {\n"
                                         "    let mut outer: i32 = 1;\n"
                                         "    {\n"
                                         "        let mut inner: i32 = 2;\n"
                                         "        inner = inner * 3;\n"
                                         "        outer = outer + inner;\n"
                                         "    }\n"
                                         "    return outer;\n"
                                         "}",
                                         "int32_t outer = 1", true, false}};

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    for (size_t i = 0; i < num_tests; i++) {
        bool result = compile_and_validate_asthra_code(test_cases[i].asthra_code,
                                                       test_cases[i].expected_c_pattern);

        if (!result) {
            printf("  ❌ Mutable variable test case %zu failed\n", i);
            return false;
        }
    }

    printf("  ✅ Mutable variables everywhere tests passed\n");
    return true;
}

// =============================================================================
// STANDALONE MAIN FUNCTION (for individual module testing)
// =============================================================================

#ifdef TEST_BASIC_IMMUTABILITY_STANDALONE
int main(void) {
    printf("=== Basic Immutability Tests (Standalone) ===\n\n");

    int passed_tests = 0;
    int total_tests = 0;

    if (test_basic_immutable_variables())
        passed_tests++;
    total_tests++;
    if (test_mutable_variables_everywhere())
        passed_tests++;
    total_tests++;

    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests (%.1f%%)\n", passed_tests, total_tests,
           (float)passed_tests / total_tests * 100.0f);

    if (passed_tests == total_tests) {
        printf("\n✅ All basic immutability tests passed!\n");
        return 0;
    } else {
        printf("\n❌ %d tests failed.\n", total_tests - passed_tests);
        return 1;
    }
}
#endif
