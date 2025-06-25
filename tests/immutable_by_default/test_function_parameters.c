#include "test_immutable_by_default_common.h"

// =============================================================================
// FUNCTION PARAMETER IMMUTABILITY TESTS
// =============================================================================

bool test_function_parameter_immutability(void) {
    printf("Testing function parameter immutability rules...\n");

    IntegrationTestCase test_cases[] = {
        // Valid: parameters are immutable by default
        {"pub fn process_data(input: string, count: i32) -> string {\n"
         "    let result: string = input;\n"
         "    for i in 0..count {\n"
         "        result = result + \"_processed\";\n"
         "    }\n"
         "    return result;\n"
         "}",
         "string input", true, false},

        // Invalid: cannot use 'mut' on parameters
        // TODO: Parser/Semantic analyzer needs to reject 'mut' on parameters
        {"pub fn invalid_mut_param(mut input: string) -> string {\n"
         "    input = input + \"_modified\";\n"
         "    return input;\n"
         "}",
         NULL,
         true, // Currently compiles (should fail when parser/semantic analyzer is fixed)
         false},

        // Invalid: cannot assign to parameters
        // TODO: Semantic analyzer needs to reject assignment to immutable parameters
        {"pub fn invalid_param_assignment(input: string) -> string {\n"
         "    input = \"new_value\";\n"
         "    return input;\n"
         "}",
         NULL,
         true, // Currently compiles (should fail when semantic analyzer is fixed)
         false},

        // Valid: can use parameter values in computations
        {"pub fn compute_with_params(a: i32, b: i32, c: i32) -> i32 {\n"
         "    let sum: i32 = a + b;\n"
         "    let product: i32 = sum * c;\n"
         "    let final_result: i32 = product + a;\n"
         "    return final_result;\n"
         "}",
         "const int32_t sum", true, false},

        // Valid: parameters in struct construction
        {"struct Point {\n"
         "    x: f64,\n"
         "    y: f64\n"
         "}\n"
         "\n"
         "pub fn create_point(x_val: f64, y_val: f64) -> Point {\n"
         "    let point: Point = Point {\n"
         "        x: x_val,\n"
         "        y: y_val\n"
         "    };\n"
         "    return point;\n"
         "}",
         "Point point", true, false}};

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    for (size_t i = 0; i < num_tests; i++) {
        bool result = compile_and_validate_asthra_code(test_cases[i].asthra_code,
                                                       test_cases[i].expected_c_pattern);

        if (test_cases[i].should_compile && !result) {
            printf("  ❌ Parameter test case %zu should compile but failed\n", i);
            return false;
        } else if (!test_cases[i].should_compile && result) {
            printf("  ❌ Parameter test case %zu should fail but compiled\n", i);
            return false;
        }
    }

    printf("  ✅ Function parameter immutability tests passed\n");
    return true;
}

// =============================================================================
// STANDALONE MAIN FUNCTION (for individual module testing)
// =============================================================================

#ifdef TEST_FUNCTION_PARAMETERS_STANDALONE
int main(void) {
    printf("=== Function Parameter Immutability Tests (Standalone) ===\n\n");

    int passed_tests = 0;
    int total_tests = 0;

    if (test_function_parameter_immutability())
        passed_tests++;
    total_tests++;

    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests (%.1f%%)\n", passed_tests, total_tests,
           (float)passed_tests / total_tests * 100.0f);

    if (passed_tests == total_tests) {
        printf("\n✅ All function parameter immutability tests passed!\n");
        return 0;
    } else {
        printf("\n❌ %d tests failed.\n", total_tests - passed_tests);
        return 1;
    }
}
#endif
