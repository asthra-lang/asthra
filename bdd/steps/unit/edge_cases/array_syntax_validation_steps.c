#include "bdd_unit_common.h"

// Test scenarios for array and slice syntax validation

// Test 1: Reject C-style array declaration with size after name
void test_c_style_array_declaration(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr[5]: i32;  // Error: Size goes in type, not after variable name\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("C-style array with size after name",
                                 "c_style_array.astra",
                                 source,
                                 0,  // should fail
                                 "expected ':' but found '['");
}

// Test 2: Reject Rust-style array declaration
void test_go_style_array_declaration(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: [i32; 5];  // Error: Asthra uses [size]Type syntax, not [Type; size]\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Rust-style array declaration",
                                 "go_style_array.astra",
                                 source,
                                 0,  // should fail
                                 "Expected type annotation after ':'");
}

// Test 3: Reject incorrect slice range syntax with dots
void test_slice_range_dots(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: [i32; 5] = [1, 2, 3, 4, 5];\n"
        "  let slice: [i32] = arr[1..3];  // Error: Use ':' not '..' for slices\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Slice range with dots",
                                 "slice_range_dots.astra",
                                 source,
                                 0,  // should fail
                                 "Expected type annotation after ':'");
}

// Test 4: Reject empty array literal without explicit type
void test_empty_array_no_type(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr = [];  // Error: Empty array needs type annotation\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Empty array without type",
                                 "empty_array_no_type.astra",
                                 source,
                                 0,  // should fail
                                 "Undefined variable 'arr' in assignment");
}

// Test 5: Positive test - correct array and slice syntax
void test_correct_array_syntax(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let fixed: [3]i32 = [1, 2, 3];\n"
        "  let slice: []i32 = [10, 20, 30, 40];\n"
        "  let sub: []i32 = slice[1:3];\n"
        "  return fixed[1] + slice[0];\n"
        "}\n";
    
    bdd_run_compilation_scenario("Correct array and slice syntax",
                                 "correct_arrays.astra",
                                 source,
                                 1,  // should succeed
                                 NULL);
}

// BDD test registry
BddTestCase array_syntax_validation_tests[] = {
    BDD_TEST_CASE(c_style_array_declaration, test_c_style_array_declaration),
    BDD_TEST_CASE(go_style_array_declaration, test_go_style_array_declaration),
    BDD_TEST_CASE(slice_range_dots, test_slice_range_dots),
    BDD_TEST_CASE(empty_array_no_type, test_empty_array_no_type),
    BDD_TEST_CASE(correct_array_syntax, test_correct_array_syntax)
};

// Main entry point for the test suite
void run_array_syntax_validation_tests(void) {
    bdd_run_test_suite("Array Syntax Validation",
                       array_syntax_validation_tests,
                       sizeof(array_syntax_validation_tests) / sizeof(BddTestCase),
                       NULL);  // No special cleanup needed
}

// Main test runner
int main(void) {
    printf("=== BDD Test: Array Syntax Validation ===\n\n");
    
    return bdd_run_test_suite("Array Syntax Validation",
                              array_syntax_validation_tests,
                              sizeof(array_syntax_validation_tests) / sizeof(BddTestCase),
                              NULL);
}