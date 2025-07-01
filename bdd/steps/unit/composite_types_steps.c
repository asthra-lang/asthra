#include "bdd_unit_common.h"
// Test scenarios using the new reusable framework

void test_fixed_size_array(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let numbers: [5]i32 = [1, 2, 3, 4, 5];\n"
        "    log(\"Fixed arrays work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Fixed-size array declaration",
                               "fixed_array.asthra",
                               source,
                               "Fixed arrays work",
                               0);
}

void test_array_const_size(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub const BUFFER_SIZE: i32 = 256;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let buffer: [BUFFER_SIZE]i32 = [0; BUFFER_SIZE];\n"
        "    log(\"Array with const size works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Array with constant size expression",
                               "array_const_size.asthra",
                               source,
                               "Array with const size works",
                               0);
}

void test_tuple_two_elements(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let pair: (i32, string) = (42, \"answer\");\n"
        "    let coords: (f64, f64) = (3.14, 2.71);\n"
        "    log(\"Tuples work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple type with two elements",
                               "tuple_two.asthra",
                               source,
                               "Tuples work",
                               0);
}

void test_dynamic_slice_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn process_slice(data: []i32) -> void {\n"
        "    log(\"Processing slice\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let numbers: []i32 = [1, 2, 3, 4, 5];\n"
        "    process_slice(numbers);\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Dynamic slice type",
                               "slice_type.asthra",
                               source,
                               "Processing slice",
                               0);
}

void test_array_size_mismatch(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let numbers: [3]i32 = [1, 2, 3, 4, 5];\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Array size mismatch error",
                                 "array_size_mismatch.asthra",
                                 source,
                                 0,  // should fail
                                 "array size mismatch");
}

void test_invalid_single_tuple(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let single: (i32) = (42);\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Invalid tuple with one element",
                                 "single_tuple.asthra",
                                 source,
                                 0,  // should fail
                                 "tuple must have at least 2 elements");
}

void test_mutable_pointer_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut x: i32 = 42;\n"
        "    let ptr: *mut i32 = &x;\n"
        "    log(\"Mutable pointer works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Mutable pointer type",
                               "mut_pointer.asthra",
                               source,
                               "Mutable pointer works",
                               0);
}

void test_slice_of_slices(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn process_matrix(data: [][]i32) -> void {\n"
        "    log(\"Processing matrix\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let matrix: [][]i32 = [[1, 2], [3, 4]];\n"
        "    process_matrix(matrix);\n"
        "    return ();\n"
        "}\n";
    
    // Currently, Asthra doesn't support implicit conversion from fixed arrays to slices
    // This test expects compilation to fail with a type mismatch error
    bdd_run_compilation_scenario("Slice of slices",
                                 "slice_of_slices.asthra",
                                 source,
                                 0,  // should fail
                                 "Type mismatch");
}

// Define test cases using the new framework with @wip tags matching the feature file
BDD_DECLARE_TEST_CASES(composite_types)
    BDD_TEST_CASE(fixed_size_array, test_fixed_size_array),
    BDD_TEST_CASE(array_const_size, test_array_const_size),
    BDD_TEST_CASE(tuple_two_elements, test_tuple_two_elements),
    BDD_TEST_CASE(dynamic_slice_type, test_dynamic_slice_type),  // This one passes
    BDD_TEST_CASE(slice_of_slices, test_slice_of_slices),  // New test case for slice of slices
    BDD_WIP_TEST_CASE(array_size_mismatch, test_array_size_mismatch),
    BDD_WIP_TEST_CASE(invalid_single_tuple, test_invalid_single_tuple),
    BDD_TEST_CASE(mutable_pointer_type, test_mutable_pointer_type),
BDD_END_TEST_CASES()

// Main test runner using the new framework
BDD_UNIT_TEST_MAIN("Composite Types", composite_types_test_cases)