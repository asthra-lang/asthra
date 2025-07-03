#include "bdd_unit_common.h"

// Test scenarios for enum variant syntax validation

// Test 1: Reject enum variant construction with double colon
void test_enum_double_colon_construction(void) {
    const char* source = 
        "package test;\n"
        "pub enum Status {\n"
        "    Active,\n"
        "    Inactive\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let s: Status = Status::Active;  // Error: double colon not allowed\n"
        "    return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Enum variant construction with double colon",
                                 "enum_double_colon_construct.astra",
                                 source,
                                 0,  // should fail
                                 "Invalid postfix '::' usage");
}

// Test 2: Reject enum variant with data using double colon
void test_enum_double_colon_with_data(void) {
    const char* source = 
        "package test;\n"
        "pub enum Option<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let opt: Option<i32> = Option::Some(42);  // Error: double colon not allowed\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Enum variant with data using double colon",
                                 "enum_double_colon_data.astra",
                                 source,
                                 0,  // should fail
                                 "Invalid postfix '::' usage");
}

// Test 3: Reject enum pattern matching with double colon
void test_enum_double_colon_pattern(void) {
    const char* source = 
        "package test;\n"
        "pub enum Result<T, E> {\n"
        "    Ok(T),\n"
        "    Err(E)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let r: Result<i32, string> = Result.Ok(42);\n"
        "    match r {\n"
        "        Result::Ok(value) => { return value; }  // Error: double colon in pattern\n"
        "        Result::Err(_) => { return 0; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_compilation_scenario("Enum pattern matching with double colon",
                                 "enum_double_colon_pattern.astra",
                                 source,
                                 0,  // should fail
                                 "expected '=>' but found '::'");
}

// Test 4: Reject if-let pattern with double colon
void test_enum_double_colon_if_let(void) {
    const char* source = 
        "package test;\n"
        "pub enum Option<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let opt: Option<i32> = Option.Some(42);\n"
        "    if let Option::Some(value) = opt {  // Error: double colon in pattern\n"
        "        return value;\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_compilation_scenario("If-let pattern with double colon",
                                 "enum_double_colon_if_let.astra",
                                 source,
                                 0,  // should fail
                                 "expected '=' but found '::'");
}

// Test 5: Correct enum variant construction with dot notation
void test_enum_dot_notation_correct(void) {
    const char* source = 
        "package test;\n"
        "pub enum Status {\n"
        "    Active,\n"
        "    Inactive\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let s: Status = Status.Active;  // Correct: dot notation\n"
        "    match s {\n"
        "        Status.Active => { return 42; }\n"
        "        Status.Inactive => { return 0; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Correct enum variant with dot notation",
                               "enum_dot_notation_correct.astra",
                               source,
                               "",  // No expected output
                               42);  // Expected exit code
}

// Test 6: Accept dot notation for enums (simplified without impl blocks)
void test_mixed_syntax_correct(void) {
    const char* source = 
        "package test;\n"
        "pub enum MyEnum {\n"
        "    First,\n"
        "    Second,\n"
        "    Third\n"
        "}\n"
        "pub struct Vec<T> {\n"
        "    data: T\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    // Using :: for type-level functions is allowed\n"
        "    // But for this test, we'll just focus on enum variants using dot notation\n"
        "    let e: MyEnum = MyEnum.Second;       // OK: . for enum variants\n"
        "    match e {\n"
        "        MyEnum.First => { return 1; }\n"
        "        MyEnum.Second => { return 2; }\n"
        "        MyEnum.Third => { return 3; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Correct enum dot notation (simplified)",
                               "mixed_syntax_correct.astra",
                               source,
                               "",  // No expected output
                               2);   // Expected exit code - should return 2 for Second variant
}

// Test 7: Reject generic enum with double colon
void test_generic_enum_double_colon(void) {
    const char* source = 
        "package test;\n"
        "pub enum Result<T, E> {\n"
        "    Ok(T),\n"
        "    Err(E)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let r: Result<i32, string> = Result<i32, string>::Ok(42);  // Error\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Generic enum with double colon",
                                 "generic_enum_double_colon.astra",
                                 source,
                                 0,  // should fail
                                 "Invalid postfix '::' usage");
}

// BDD test registry
BddTestCase enum_variant_syntax_tests[] = {
    BDD_TEST_CASE(enum_double_colon_construction, test_enum_double_colon_construction),
    BDD_TEST_CASE(enum_double_colon_with_data, test_enum_double_colon_with_data),
    BDD_TEST_CASE(enum_double_colon_pattern, test_enum_double_colon_pattern),
    BDD_TEST_CASE(enum_double_colon_if_let, test_enum_double_colon_if_let),
    BDD_TEST_CASE(enum_dot_notation_correct, test_enum_dot_notation_correct),
    BDD_TEST_CASE(mixed_syntax_correct, test_mixed_syntax_correct),
    BDD_TEST_CASE(generic_enum_double_colon, test_generic_enum_double_colon)
};

// Main entry point for the test suite
void run_enum_variant_syntax_tests(void) {
    bdd_run_test_suite("Enum Variant Syntax Validation",
                       enum_variant_syntax_tests,
                       sizeof(enum_variant_syntax_tests) / sizeof(BddTestCase),
                       NULL);  // No special cleanup needed
}

// Main test runner
int main(void) {
    printf("=== BDD Test: Enum Variant Syntax Validation ===\n\n");
    
    return bdd_run_test_suite("Enum Variant Syntax Validation",
                              enum_variant_syntax_tests,
                              sizeof(enum_variant_syntax_tests) / sizeof(BddTestCase),
                              NULL);
}