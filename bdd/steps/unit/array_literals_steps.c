#include "bdd_unit_common.h"
// Test scenarios for array literals

// Empty arrays
void test_empty_array_with_none_marker(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [none];\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty array with none marker",
                               "array_empty_none.asthra",
                               source,
                               NULL,
                               42);
}

void test_empty_array_for_fixed_size(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: [0]i32 = [none];\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty array for fixed size",
                               "array_fixed_empty.asthra",
                               source,
                               NULL,
                               42);
}

// Simple array literals
void test_single_element_array(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [42];\n"
        "  return arr[0];\n"
        "}\n";
    
    bdd_run_execution_scenario("Single element array",
                               "array_single.asthra",
                               source,
                               NULL,
                               42);
}

void test_multiple_element_array(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [10, 20, 30, 40, 50];\n"
        "  return arr[1] + arr[3] - 18;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple element array",
                               "array_multiple.asthra",
                               source,
                               NULL,
                               42);
}

void test_array_of_different_numeric_types(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr_i8: []i8 = [1, 2, 3];\n"
        "  let arr_u32: []u32 = [100, 200, 300];\n"
        "  return (arr_i8[1] as i32) + 40;\n"
        "}\n";
    
    bdd_run_execution_scenario("Array of different numeric types",
                               "array_numeric_types.asthra",
                               source,
                               NULL,
                               42);
}

// Repeated element arrays
void test_repeated_element_array_syntax(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [7; 6];\n"
        "  return arr[0] * arr[5];\n"
        "}\n";
    
    bdd_run_execution_scenario("Repeated element array syntax",
                               "array_repeated.asthra",
                               source,
                               NULL,
                               42);
}

void test_repeated_zero_initialization(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [0; 100];\n"
        "  return arr[0] + arr[50] + arr[99] + 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Repeated zero initialization",
                               "array_zero_init.asthra",
                               source,
                               NULL,
                               42);
}

void test_repeated_element_with_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let value: i32 = 14;\n"
        "  let arr: []i32 = [value; 3];\n"
        "  return arr[0] + arr[1] + arr[2];\n"
        "}\n";
    
    bdd_run_execution_scenario("Repeated element with expression",
                               "array_repeated_expr.asthra",
                               source,
                               NULL,
                               42);
}

// Fixed-size arrays
void test_fixed_size_array_declaration(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: [5]i32 = [8, 8, 8, 9, 9];\n"
        "  return arr[0] + arr[1] + arr[2] + arr[3] + arr[4];\n"
        "}\n";
    
    bdd_run_execution_scenario("Fixed-size array declaration",
                               "array_fixed_size.asthra",
                               source,
                               NULL,
                               42);
}

void test_fixed_size_array_with_const_size(void) {
    const char* source = 
        "package test;\n"
        "pub const SIZE: i32 = 3;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: [SIZE]i32 = [14, 14, 14];\n"
        "  return arr[0] + arr[1] + arr[2];\n"
        "}\n";
    
    bdd_run_execution_scenario("Fixed-size array with const size",
                               "array_const_size.asthra",
                               source,
                               NULL,
                               42);
}

// Array indexing
void test_basic_array_indexing(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [10, 20, 30, 40, 50];\n"
        "  let idx: i32 = 2;\n"
        "  return arr[idx] + 12;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic array indexing",
                               "array_indexing.asthra",
                               source,
                               NULL,
                               42);
}

void test_array_indexing_with_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [5, 10, 15, 20, 25];\n"
        "  return arr[1 + 2] + 22;\n"
        "}\n";
    
    bdd_run_execution_scenario("Array indexing with expression",
                               "array_index_expr.asthra",
                               source,
                               NULL,
                               42);
}

// Mutable arrays
void test_mutable_array_element_modification(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let mut arr: []i32 = [10, 20, 30];\n"
        "  arr[1] = 12;\n"
        "  return arr[0] + arr[1] + arr[2] - 10;\n"
        "}\n";
    
    bdd_run_execution_scenario("Mutable array element modification",
                               "array_mut_element.asthra",
                               source,
                               NULL,
                               42);
}

void test_mutable_array_reassignment(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let mut arr: []i32 = [1, 2, 3];\n"
        "  arr = [14, 14, 14];\n"
        "  return arr[0] + arr[1] + arr[2];\n"
        "}\n";
    
    bdd_run_execution_scenario("Mutable array reassignment",
                               "array_mut_reassign.asthra",
                               source,
                               NULL,
                               42);
}

// Array of arrays
void test_two_dimensional_array(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let matrix: [][]i32 = [[1, 2], [3, 4], [5, 6]];\n"
        "  return matrix[0][0] + matrix[1][1] + matrix[2][0] + 32;\n"
        "}\n";
    
    bdd_run_execution_scenario("Two-dimensional array",
                               "array_2d.asthra",
                               source,
                               NULL,
                               42);
}

void test_nested_array_indexing(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: [][]i32 = [[10, 20], [30, 40]];\n"
        "  return arr[0][1] + arr[1][0] - 8;\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested array indexing",
                               "array_nested_index.asthra",
                               source,
                               NULL,
                               42);
}

// Array slicing
void test_full_array_slice(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [10, 20, 30, 40, 50];\n"
        "  let slice: []i32 = arr[:];\n"
        "  return slice[1] + slice[3] - 18;\n"
        "}\n";
    
    bdd_run_execution_scenario("Full array slice",
                               "array_slice_full.asthra",
                               source,
                               NULL,
                               42);
}

void test_array_slice_from_start(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [5, 10, 15, 20, 25];\n"
        "  let slice: []i32 = arr[:3];\n"
        "  return slice[0] + slice[1] + slice[2] + 12;\n"
        "}\n";
    
    bdd_run_execution_scenario("Array slice from start",
                               "array_slice_start.asthra",
                               source,
                               NULL,
                               42);
}

void test_array_slice_to_end(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [5, 10, 15, 20, 25];\n"
        "  let slice: []i32 = arr[2:];\n"
        "  return slice[0] + slice[1] + slice[2] - 18;\n"
        "}\n";
    
    bdd_run_execution_scenario("Array slice to end",
                               "array_slice_end.asthra",
                               source,
                               NULL,
                               42);
}

void test_array_slice_range(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [1, 2, 3, 4, 5, 6, 7, 8, 9];\n"
        "  let slice: []i32 = arr[2:5];\n"
        "  return slice[0] + slice[1] + slice[2] + 30;\n"
        "}\n"
        "\n"
        "// Dummy function to work around parser EOF bug\n"
        "priv fn dummy(none) -> void { return (); }\n"
        "\n";
    
    bdd_run_execution_scenario("Array slice range",
                               "array_slice_range.asthra",
                               source,
                               NULL,
                               42);
}

// Array length
void test_get_array_length(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [10, 20, 30, 40, 50, 60, 70];\n"
        "  return arr.len as i32 * 6;\n"
        "}\n";
    
    bdd_run_execution_scenario("Get array length",
                               "array_length.asthra",
                               source,
                               NULL,
                               42);
}

void test_empty_array_length(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [none];\n"
        "  return arr.len as i32 + 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty array length",
                               "array_empty_len.asthra",
                               source,
                               NULL,
                               42);
}

// Arrays with other types
void test_boolean_array(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let flags: []bool = [true, false, true, true];\n"
        "  if flags[0] && !flags[1] && flags[2] {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Boolean array",
                               "array_bool.asthra",
                               source,
                               NULL,
                               42);
}

void test_string_array(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let words: []string = [\"hello\", \"world\", \"test\"];\n"
        "  if words[0] == \"hello\" && words[1] == \"world\" {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("String array",
                               "array_string.asthra",
                               source,
                               NULL,
                               42);
}

// Arrays as function parameters
void test_pass_array_to_function(void) {
    const char* source = 
        "package test;\n"
        "pub fn sum_array(arr: []i32) -> i32 {\n"
        "  return arr[0] + arr[1] + arr[2];\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let numbers: []i32 = [10, 15, 17];\n"
        "  return sum_array(numbers);\n"
        "}\n";
    
    bdd_run_execution_scenario("Pass array to function",
                               "array_func_param.asthra",
                               source,
                               NULL,
                               42);
}

void test_return_array_from_function(void) {
    const char* source = 
        "package test;\n"
        "pub fn create_array(none) -> []i32 {\n"
        "  return [14, 14, 14];\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = create_array(none);\n"
        "  return arr[0] + arr[1] + arr[2];\n"
        "}\n";
    
    bdd_run_execution_scenario("Return array from function",
                               "array_func_return.asthra",
                               source,
                               NULL,
                               42);
}

// Array operations in loops
void test_iterate_over_array(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [5, 8, 10, 7, 12];\n"
        "  let sum: i32 = 0;\n"
        "  for val in arr {\n"
        "    sum = sum + val;\n"
        "  }\n"
        "  return sum;\n"
        "}\n";
    
    bdd_run_execution_scenario("Iterate over array",
                               "array_for_in.asthra",
                               source,
                               NULL,
                               42);
}

void test_iterate_with_range(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [10, 20, 30];\n"
        "  let mut sum: i32 = 0;\n"
        "  for i in range(3) {\n"
        "    sum = sum + arr[i];\n"
        "  }\n"
        "  return sum - 18;\n"
        "}\n";
    
    bdd_run_execution_scenario("Iterate with range",
                               "array_range_loop.asthra",
                               source,
                               NULL,
                               42);
}

// Edge cases
void test_large_repeated_array(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let arr: []i32 = [1; 42];\n"
        "  return arr[0] + arr[20] + arr[41] + 39;\n"
        "}\n";
    
    bdd_run_execution_scenario("Large repeated array",
                               "array_large_repeated.asthra",
                               source,
                               NULL,
                               42);
}

void test_array_of_tuples(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let pairs: [](i32, i32) = [(10, 20), (5, 7), (15, 25)];\n"
        "  let p: (i32, i32) = pairs[1];\n"
        "  return p.0 + p.1 + 30;\n"
        "}\n";
    
    bdd_run_execution_scenario("Array of tuples",
                               "array_tuples.asthra",
                               source,
                               NULL,
                               42);
}

// Define test cases
BDD_DECLARE_TEST_CASES(array_literals)
    // Empty arrays
    BDD_WIP_TEST_CASE(empty_array_with_none_marker, test_empty_array_with_none_marker),
    BDD_WIP_TEST_CASE(empty_array_for_fixed_size, test_empty_array_for_fixed_size),
    
    // Simple array literals
    BDD_WIP_TEST_CASE(single_element_array, test_single_element_array),
    BDD_WIP_TEST_CASE(multiple_element_array, test_multiple_element_array),
    BDD_WIP_TEST_CASE(array_of_different_numeric_types, test_array_of_different_numeric_types),
    
    // Repeated element arrays
    BDD_WIP_TEST_CASE(repeated_element_array_syntax, test_repeated_element_array_syntax),
    BDD_WIP_TEST_CASE(repeated_zero_initialization, test_repeated_zero_initialization),
    BDD_WIP_TEST_CASE(repeated_element_with_expression, test_repeated_element_with_expression),
    
    // Fixed-size arrays
    BDD_WIP_TEST_CASE(fixed_size_array_declaration, test_fixed_size_array_declaration),
    BDD_WIP_TEST_CASE(fixed_size_array_with_const_size, test_fixed_size_array_with_const_size),
    
    // Array indexing
    BDD_WIP_TEST_CASE(basic_array_indexing, test_basic_array_indexing),
    BDD_WIP_TEST_CASE(array_indexing_with_expression, test_array_indexing_with_expression),
    
    // Mutable arrays
    BDD_WIP_TEST_CASE(mutable_array_element_modification, test_mutable_array_element_modification),
    BDD_WIP_TEST_CASE(mutable_array_reassignment, test_mutable_array_reassignment),
    
    // Array of arrays
    BDD_WIP_TEST_CASE(two_dimensional_array, test_two_dimensional_array),
    BDD_WIP_TEST_CASE(nested_array_indexing, test_nested_array_indexing),
    
    // Array slicing
    BDD_WIP_TEST_CASE(full_array_slice, test_full_array_slice),
    BDD_WIP_TEST_CASE(array_slice_from_start, test_array_slice_from_start),
    BDD_WIP_TEST_CASE(array_slice_to_end, test_array_slice_to_end),
    BDD_WIP_TEST_CASE(array_slice_range, test_array_slice_range),
    
    // Array length
    BDD_WIP_TEST_CASE(get_array_length, test_get_array_length),
    BDD_WIP_TEST_CASE(empty_array_length, test_empty_array_length),
    
    // Arrays with other types
    BDD_WIP_TEST_CASE(boolean_array, test_boolean_array),
    BDD_WIP_TEST_CASE(string_array, test_string_array),
    
    // Arrays as function parameters
    BDD_WIP_TEST_CASE(pass_array_to_function, test_pass_array_to_function),
    BDD_WIP_TEST_CASE(return_array_from_function, test_return_array_from_function),
    
    // Array operations in loops
    BDD_WIP_TEST_CASE(iterate_over_array, test_iterate_over_array),
    BDD_WIP_TEST_CASE(iterate_with_range, test_iterate_with_range),
    
    // Edge cases
    BDD_WIP_TEST_CASE(large_repeated_array, test_large_repeated_array),
    BDD_WIP_TEST_CASE(array_of_tuples, test_array_of_tuples),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Array Literals", array_literals_test_cases)