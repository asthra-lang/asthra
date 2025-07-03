#include "bdd_unit_common.h"

// Test scenarios for method and associated function syntax validation

// Test 1: Reject missing visibility modifier on methods
void test_missing_visibility_modifier(void) {
    const char* source = 
        "package test;\n"
        "pub struct Counter {\n"
        "  value: i32\n"
        "}\n"
        "impl Counter {\n"
        "  fn increment(self) -> i32 {  // Error: Missing pub/priv\n"
        "    return self.value + 1;\n"
        "  }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Method without visibility modifier",
                                 "method_no_visibility.astra",
                                 source,
                                 0,  // should fail
                                 "methods must have explicit visibility");
}

// Test 2: Reject mutable self parameter
void test_mutable_self_parameter(void) {
    const char* source = 
        "package test;\n"
        "pub struct Counter {\n"
        "  value: i32\n"
        "}\n"
        "impl Counter {\n"
        "  pub fn increment(mut self) -> void {  // Error: self is always immutable\n"
        "    self.value = self.value + 1;\n"
        "    return ();\n"
        "  }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Mutable self parameter",
                                 "mutable_self.astra",
                                 source,
                                 0,  // should fail
                                 "'self' parameter cannot be mutable");
}

// Test 3: Reject dot notation for associated function calls
void test_dot_notation_associated_function(void) {
    const char* source = 
        "package test;\n"
        "pub struct Calculator {\n"
        "  none\n"
        "}\n"
        "impl Calculator {\n"
        "  pub fn add(a: i32, b: i32) -> i32 {\n"
        "    return a + b;\n"
        "  }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return Calculator.add(20, 22);  // Error: Must use :: for associated functions\n"
        "}\n";
    
    bdd_run_compilation_scenario("Dot notation for associated function",
                                 "dot_assoc_func.astra",
                                 source,
                                 0,  // should fail
                                 "use '::' for associated function calls");
}

// Test 4: Reject this keyword instead of self
void test_this_keyword(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "  x: i32,\n"
        "  y: i32\n"
        "}\n"
        "impl Point {\n"
        "  pub fn get_x(this) -> i32 {  // Error: Use 'self' not 'this'\n"
        "    return this.x;\n"
        "  }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("this keyword instead of self",
                                 "this_keyword.astra",
                                 source,
                                 0,  // should fail
                                 "expected 'self' for instance method");
}

// Test 5: Positive test - correct method and associated function syntax
void test_correct_method_syntax(void) {
    const char* source = 
        "package test;\n"
        "pub struct Calculator {\n"
        "  value: i32\n"
        "}\n"
        "impl Calculator {\n"
        "  pub fn new(v: i32) -> Calculator {\n"
        "    return Calculator { value: v };\n"
        "  }\n"
        "  pub fn add(self, x: i32) -> i32 {\n"
        "    return self.value + x;\n"
        "  }\n"
        "  priv fn internal(self) -> i32 {\n"
        "    return self.value;\n"
        "  }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let calc: Calculator = Calculator::new(40);\n"
        "  return calc.add(2);\n"
        "}\n";
    
    bdd_run_compilation_scenario("Correct method syntax",
                                 "correct_methods.astra",
                                 source,
                                 1,  // should succeed
                                 NULL);
}

// BDD test registry
BddTestCase method_syntax_validation_tests[] = {
    BDD_TEST_CASE(missing_visibility_modifier, test_missing_visibility_modifier),
    BDD_TEST_CASE(mutable_self_parameter, test_mutable_self_parameter),
    BDD_TEST_CASE(dot_notation_associated_function, test_dot_notation_associated_function),
    BDD_TEST_CASE(this_keyword, test_this_keyword),
    BDD_TEST_CASE(correct_method_syntax, test_correct_method_syntax)
};

// Main entry point for the test suite
void run_method_syntax_validation_tests(void) {
    bdd_run_test_suite("Method Syntax Validation",
                       method_syntax_validation_tests,
                       sizeof(method_syntax_validation_tests) / sizeof(BddTestCase),
                       NULL);  // No special cleanup needed
}

// Main test runner
int main(void) {
    printf("=== BDD Test: Method Syntax Validation ===\n\n");
    
    return bdd_run_test_suite("Method Syntax Validation",
                              method_syntax_validation_tests,
                              sizeof(method_syntax_validation_tests) / sizeof(BddTestCase),
                              NULL);
}