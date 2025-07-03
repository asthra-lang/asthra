#include "bdd_unit_common.h"

// Test scenarios for type annotation syntax validation

// Test 1: Reject Go-style type annotations without colon
void test_go_style_type_annotation(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let x i32 = 42;  // Error: Missing ':' before type\n"
        "  return x;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Go-style type without colon",
                                 "go_style_type.astra",
                                 source,
                                 0,  // should fail
                                 "expected '=' but found 'i32'");
}

// Test 2: Reject var keyword instead of let
void test_var_keyword(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  var x: i32 = 42;  // Error: Use 'let' not 'var'\n"
        "  return x;\n"
        "}\n";
    
    bdd_run_compilation_scenario("var keyword instead of let",
                                 "var_keyword.astra",
                                 source,
                                 0,  // should fail
                                 "expected statement but found 'var'");
}

// Test 3: Reject const for local variables
void test_const_local_variable(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  const x: i32 = 42;  // Error: 'const' is only for top-level\n"
        "  return x;\n"
        "}\n";
    
    bdd_run_compilation_scenario("const for local variable",
                                 "const_local.astra",
                                 source,
                                 0,  // should fail
                                 "'const' declarations are only allowed at top level");
}

// Test 4: Reject missing type in mutable declaration
void test_mut_without_let(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  mut x = 42;  // Error: 'mut' must follow 'let'\n"
        "  return x;\n"
        "}\n";
    
    bdd_run_compilation_scenario("mut without let",
                                 "mut_without_let.astra",
                                 source,
                                 0,  // should fail
                                 "expected 'let' before 'mut'");
}

// Test 5: Positive test - correct type annotation syntax
void test_correct_type_annotations(void) {
    const char* source = 
        "package test;\n"
        "pub const MAX: i32 = 100;\n"
        "pub fn main(none) -> i32 {\n"
        "  let x: i32 = 42;\n"
        "  let mut y: i32 = 0;\n"
        "  let ptr: *i32 = &x;\n"
        "  let opt: Option<i32> = Option.Some(42);\n"
        "  y = 42;\n"
        "  return y;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Correct type annotations",
                                 "correct_types.astra",
                                 source,
                                 1,  // should succeed
                                 NULL);
}

// BDD test registry
BddTestCase type_annotation_validation_tests[] = {
    BDD_TEST_CASE(go_style_type_annotation, test_go_style_type_annotation),
    BDD_TEST_CASE(var_keyword, test_var_keyword),
    BDD_TEST_CASE(const_local_variable, test_const_local_variable),
    BDD_TEST_CASE(mut_without_let, test_mut_without_let),
    BDD_TEST_CASE(correct_type_annotations, test_correct_type_annotations)
};

// Main entry point for the test suite
void run_type_annotation_validation_tests(void) {
    bdd_run_test_suite("Type Annotation Validation",
                       type_annotation_validation_tests,
                       sizeof(type_annotation_validation_tests) / sizeof(BddTestCase),
                       NULL);  // No special cleanup needed
}

// Main test runner
int main(void) {
    printf("=== BDD Test: Type Annotation Validation ===\n\n");
    
    return bdd_run_test_suite("Type Annotation Validation",
                              type_annotation_validation_tests,
                              sizeof(type_annotation_validation_tests) / sizeof(BddTestCase),
                              NULL);
}