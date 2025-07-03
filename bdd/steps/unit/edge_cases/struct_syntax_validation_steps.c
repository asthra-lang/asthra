#include "bdd_unit_common.h"

// Test scenarios for struct syntax validation

// Test 1: Reject C-style struct with semicolon after fields
void test_struct_field_semicolon(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "  x: i32;  // Error: No semicolons after fields\n"
        "  y: i32;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Struct fields with semicolons",
                                 "struct_field_semicolon.astra",
                                 source,
                                 0,  // should fail
                                 "expected ',' or '}' but found ';'");
}

// Test 2: Reject mutable struct fields
void test_mutable_struct_fields(void) {
    const char* source = 
        "package test;\n"
        "pub struct Counter {\n"
        "  mut count: i32  // Error: Struct fields cannot have mut modifier\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Mutable struct fields",
                                 "mutable_struct_fields.astra",
                                 source,
                                 0,  // should fail
                                 "struct fields cannot be declared mutable");
}

// Test 3: Reject struct initialization with equals sign
void test_struct_init_equals(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "  x: i32,\n"
        "  y: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let p: Point = Point { x = 10, y = 20 };  // Error: Use ':' not '='\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Struct initialization with equals",
                                 "struct_init_equals.astra",
                                 source,
                                 0,  // should fail
                                 "expected ':' but found '='");
}

// Test 4: Reject missing none keyword for empty struct
void test_empty_struct_without_none(void) {
    const char* source = 
        "package test;\n"
        "pub struct Empty {}  // Error: Must use 'none' for empty structs\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Empty struct without none",
                                 "empty_struct_no_none.astra",
                                 source,
                                 0,  // should fail
                                 "empty struct must contain 'none'");
}

// Test 5: Positive test - correct struct syntax
void test_correct_struct_syntax(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "  x: i32,\n"
        "  y: i32\n"
        "}\n"
        "pub struct Empty {\n"
        "  none\n"
        "}\n"
        "pub struct Mixed {\n"
        "  pub value: i32,\n"
        "  priv internal: string\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let p: Point = Point { x: 42, y: 0 };\n"
        "  return p.x;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Correct struct syntax",
                                 "correct_structs.astra",
                                 source,
                                 1,  // should succeed
                                 NULL);
}

// BDD test registry
BddTestCase struct_syntax_validation_tests[] = {
    BDD_TEST_CASE(struct_field_semicolon, test_struct_field_semicolon),
    BDD_TEST_CASE(mutable_struct_fields, test_mutable_struct_fields),
    BDD_TEST_CASE(struct_init_equals, test_struct_init_equals),
    BDD_TEST_CASE(empty_struct_without_none, test_empty_struct_without_none),
    BDD_TEST_CASE(correct_struct_syntax, test_correct_struct_syntax)
};

// Main entry point for the test suite
void run_struct_syntax_validation_tests(void) {
    bdd_run_test_suite("Struct Syntax Validation",
                       struct_syntax_validation_tests,
                       sizeof(struct_syntax_validation_tests) / sizeof(BddTestCase),
                       NULL);  // No special cleanup needed
}

// Main test runner
int main(void) {
    printf("=== BDD Test: Struct Syntax Validation ===\n\n");
    
    return bdd_run_test_suite("Struct Syntax Validation",
                              struct_syntax_validation_tests,
                              sizeof(struct_syntax_validation_tests) / sizeof(BddTestCase),
                              NULL);
}