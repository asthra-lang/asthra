#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"

// Test scenarios using the new reusable framework

void test_generic_struct_single(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Box<T> {\n"
        "    value: T\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let int_box: Box<i32> = Box { value: 42 };\n"
        "    let str_box: Box<string> = Box { value: \"hello\" };\n"
        "    log(\"Generic struct works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Generic struct with single type parameter",
                               "generic_struct_single.asthra",
                               source,
                               "Generic struct works",
                               0);
}

void test_generic_struct_multiple(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Pair<T, U> {\n"
        "    first: T,\n"
        "    second: U\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p1: Pair<i32, string> = Pair { first: 42, second: \"answer\" };\n"
        "    let p2: Pair<bool, f64> = Pair { first: true, second: 3.14 };\n"
        "    log(\"Multiple type parameters work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Generic struct with multiple type parameters",
                               "generic_struct_multiple.asthra",
                               source,
                               "Multiple type parameters work",
                               0);
}

void test_generic_enum(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub enum Option<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let some_int: Option<i32> = Option::Some(42);\n"
        "    let some_str: Option<string> = Option::Some(\"hello\");\n"
        "    let none_int: Option<i32> = Option::None;\n"
        "    log(\"Generic enum works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Generic enum",
                               "generic_enum.asthra",
                               source,
                               "Generic enum works",
                               0);
}

void test_generic_function(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn identity<T>(value: T) -> T {\n"
        "    return value;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = identity(42);\n"
        "    let s: string = identity(\"hello\");\n"
        "    log(\"Generic function works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Generic function",
                               "generic_function.asthra",
                               source,
                               "Generic function works",
                               0);
}

void test_generic_methods(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Container<T> {\n"
        "    value: T\n"
        "}\n"
        "\n"
        "impl<T> Container<T> {\n"
        "    pub fn new(val: T) -> Container<T> {\n"
        "        return Container { value: val };\n"
        "    }\n"
        "    \n"
        "    pub fn get(self) -> T {\n"
        "        return self.value;\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let c1: Container<i32> = Container::new(42);\n"
        "    let c2: Container<string> = Container::new(\"hello\");\n"
        "    log(\"Generic methods work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Generic methods",
                               "generic_methods.asthra",
                               source,
                               "Generic methods work",
                               0);
}

void test_nested_generics(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Box<T> {\n"
        "    value: T\n"
        "}\n"
        "\n"
        "pub enum Option<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let nested: Box<Option<i32>> = Box { \n"
        "        value: Option::Some(42) \n"
        "    };\n"
        "    log(\"Nested generics work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested generic types",
                               "nested_generics.asthra",
                               source,
                               "Nested generics work",
                               0);
}

void test_generic_arrays(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Vector<T> {\n"
        "    data: []T\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let v1: Vector<i32> = Vector { data: [1, 2, 3] };\n"
        "    let v2: Vector<string> = Vector { data: [\"a\", \"b\", \"c\"] };\n"
        "    log(\"Generic arrays work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Generic type with arrays",
                               "generic_arrays.asthra",
                               source,
                               "Generic arrays work",
                               0);
}

void test_result_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn divide(a: i32, b: i32) -> Result<i32, string> {\n"
        "    if b == 0 {\n"
        "        return Result::Err(\"Division by zero\");\n"
        "    }\n"
        "    return Result::Ok(a / b);\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let r1: Result<i32, string> = divide(10, 2);\n"
        "    let r2: Result<i32, string> = divide(10, 0);\n"
        "    log(\"Result type works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Result type usage",
                               "result_type.asthra",
                               source,
                               "Result type works",
                               0);
}

void test_option_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn find_value(key: string) -> Option<i32> {\n"
        "    if key == \"answer\" {\n"
        "        return Option::Some(42);\n"
        "    }\n"
        "    return Option::None;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let found: Option<i32> = find_value(\"answer\");\n"
        "    let not_found: Option<i32> = find_value(\"question\");\n"
        "    log(\"Option type works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Option type usage",
                               "option_type.asthra",
                               source,
                               "Option type works",
                               0);
}

void test_missing_type_param(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Box<T> {\n"
        "    value: T\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let b: Box = Box { value: 42 };\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Missing type parameter error",
                                 "missing_type_param.asthra",
                                 source,
                                 0,  // should fail
                                 "missing type parameter");
}

void test_wrong_type_params(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Pair<T, U> {\n"
        "    first: T,\n"
        "    second: U\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: Pair<i32> = Pair { first: 42, second: \"hello\" };\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Wrong number of type parameters",
                                 "wrong_type_params.asthra",
                                 source,
                                 0,  // should fail
                                 "wrong number of type parameters");
}

void test_type_param_conflict(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Container<T, T> {\n"
        "    value1: T,\n"
        "    value2: T\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Type parameter name conflict",
                                 "type_param_conflict.asthra",
                                 source,
                                 0,  // should fail
                                 "duplicate type parameter");
}

// Define test cases using the new framework - no @wip tags based on original file
BddTestCase generic_types_test_cases[] = {
    BDD_TEST_CASE(generic_struct_single, test_generic_struct_single),
    BDD_TEST_CASE(generic_struct_multiple, test_generic_struct_multiple),
    BDD_TEST_CASE(generic_enum, test_generic_enum),
    BDD_TEST_CASE(generic_function, test_generic_function),
    BDD_TEST_CASE(generic_methods, test_generic_methods),
    BDD_TEST_CASE(nested_generics, test_nested_generics),
    BDD_TEST_CASE(generic_arrays, test_generic_arrays),
    BDD_TEST_CASE(result_type, test_result_type),
    BDD_TEST_CASE(option_type, test_option_type),
    BDD_TEST_CASE(missing_type_param, test_missing_type_param),
    BDD_TEST_CASE(wrong_type_params, test_wrong_type_params),
    BDD_TEST_CASE(type_param_conflict, test_type_param_conflict),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Generic Types",
                              generic_types_test_cases,
                              sizeof(generic_types_test_cases) / sizeof(generic_types_test_cases[0]),
                              bdd_cleanup_temp_files);
}