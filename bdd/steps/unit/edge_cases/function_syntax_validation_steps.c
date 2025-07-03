#include "bdd_unit_common.h"

// Test scenarios for function syntax validation

// Test 1: Reject Go-style function syntax without return type arrow
void test_go_style_function_syntax(void) {
    const char* source = 
        "package test;\n"
        "pub fn add(a: i32, b: i32) i32 {  // Error: Missing '->' before return type\n"
        "  return a + b;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return add(1, 2);\n"
        "}\n";
    
    bdd_run_compilation_scenario("Go-style function without arrow",
                                 "go_style_function.astra",
                                 source,
                                 0,  // should fail
                                 "expected '->' but found 'i32'");
}

// Test 2: Reject empty parentheses for no parameters
void test_empty_parentheses_params(void) {
    const char* source = 
        "package test;\n"
        "pub fn get_value() -> i32 {  // Error: Must use 'none' for no parameters\n"
        "  return 42;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return get_value();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Empty parentheses for no parameters",
                                 "empty_params.astra",
                                 source,
                                 0,  // should fail
                                 "expected parameter or 'none'");
}

// Test 3: Reject mutable parameters
void test_mutable_parameters(void) {
    const char* source = 
        "package test;\n"
        "pub fn increment(mut x: i32) -> i32 {  // Error: Parameters are always immutable\n"
        "  x = x + 1;\n"
        "  return x;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Mutable function parameters",
                                 "mutable_params.astra",
                                 source,
                                 0,  // should fail
                                 "parameters cannot be declared mutable");
}

// Test 4: Positive test - correct function syntax
void test_correct_function_syntax(void) {
    const char* source = 
        "package test;\n"
        "pub fn add(a: i32, b: i32) -> i32 {\n"
        "  return a + b;\n"
        "}\n"
        "pub fn no_params(none) -> i32 {\n"
        "  return 42;\n"
        "}\n"
        "pub fn void_return(none) -> void {\n"
        "  return ();\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return add(20, 22);\n"
        "}\n";
    
    bdd_run_compilation_scenario("Correct function syntax",
                                 "correct_functions.astra",
                                 source,
                                 1,  // should succeed
                                 NULL);
}

// BDD test registry
BddTestCase function_syntax_validation_tests[] = {
    BDD_TEST_CASE(go_style_function_syntax, test_go_style_function_syntax),
    BDD_TEST_CASE(empty_parentheses_params, test_empty_parentheses_params),
    BDD_TEST_CASE(mutable_parameters, test_mutable_parameters),
    BDD_TEST_CASE(correct_function_syntax, test_correct_function_syntax)
};

// Main entry point for the test suite
void run_function_syntax_validation_tests(void) {
    bdd_run_test_suite("Function Syntax Validation",
                       function_syntax_validation_tests,
                       sizeof(function_syntax_validation_tests) / sizeof(BddTestCase),
                       NULL);  // No special cleanup needed
}

// Main test runner
int main(void) {
    printf("=== BDD Test: Function Syntax Validation ===\n\n");
    
    return bdd_run_test_suite("Function Syntax Validation",
                              function_syntax_validation_tests,
                              sizeof(function_syntax_validation_tests) / sizeof(BddTestCase),
                              NULL);
}