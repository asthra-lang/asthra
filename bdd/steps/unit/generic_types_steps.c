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
        "    let int_box: Box<i32> = Box<i32> { value: 42 };\n"
        "    log(\"Generic struct works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Generic struct with single type parameter",
                               "generic_struct_single.asthra",
                               source,
                               "Generic struct works",
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
        "    let box_val: Box = Box { value: 42 };\n"  // Missing type parameter
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Missing type parameter",
                                 "missing_type_param.asthra",
                                 source,
                                 0,  // should fail
                                 "requires type arguments");
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
        "    let p: Pair<i32> = Pair<i32> { first: 1, second: 2 };\n"  // Wrong number of type params
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Wrong number of type parameters",
                                 "wrong_type_params.asthra",
                                 source,
                                 0,  // should fail
                                 "Type argument count mismatch");
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
                                 "Duplicate type parameter");
}

// Define test cases using the new framework - no @wip tags based on original file
BddTestCase generic_types_test_cases[] = {
    BDD_TEST_CASE(generic_struct_single, test_generic_struct_single),
    BDD_TEST_CASE(missing_type_param, test_missing_type_param),
    BDD_TEST_CASE(wrong_type_params, test_wrong_type_params),
    BDD_TEST_CASE(type_param_conflict, test_type_param_conflict),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Generic Types",
                              generic_types_test_cases,
                              sizeof(generic_types_test_cases) / sizeof(generic_types_test_cases[0]),
                              NULL);
}