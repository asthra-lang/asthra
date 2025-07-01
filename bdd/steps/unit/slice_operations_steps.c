#include "bdd_unit_common.h"

void test_basic_slice_start_end(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let array: [5]i32 = [10, 20, 30, 40, 50];\n"
        "    let slice: []i32 = array[1:4];\n"
        "    // Should contain [20, 30, 40], so return middle element\n"
        "    return slice[1];\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic slice start and end",
                               "slice_start_end.asthra",
                               source,
                               NULL,
                               30);
}

void test_slice_start_only(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let array: [4]i32 = [1, 2, 3, 4];\n"
        "    let slice: []i32 = array[2:];\n"
        "    // Should contain [3, 4], return last element\n"
        "    return slice[1];\n"
        "}\n";
    
    bdd_run_execution_scenario("Slice start only",
                               "slice_start.asthra",
                               source,
                               NULL,
                               4);
}

void test_slice_end_only(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let array: [6]i32 = [1, 2, 3, 4, 5, 6];\n"
        "    let slice: []i32 = array[:3];\n"
        "    // Should contain [1, 2, 3], return sum of first two\n"
        "    return slice[0] + slice[1];\n"
        "}\n";
    
    bdd_run_execution_scenario("Slice end only",
                               "slice_end.asthra",
                               source,
                               NULL,
                               3);
}

void test_full_slice_copy(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let array: [3]i32 = [100, 200, 300];\n"
        "    let slice: []i32 = array[:];\n"
        "    // Should contain all elements, return first element / 100\n"
        "    return slice[0] / 100;\n"
        "}\n";
    
    bdd_run_execution_scenario("Full slice copy",
                               "slice_full.asthra",
                               source,
                               NULL,
                               1);
}

void test_slice_with_variables(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let array: [7]i32 = [1, 2, 3, 4, 5, 6, 7];\n"
        "    let start: i32 = 2;\n"
        "    let end: i32 = 5;\n"
        "    let slice: []i32 = array[start:end];\n"
        "    // Should contain [3, 4, 5], return middle element\n"
        "    return slice[1];\n"
        "}\n";
    
    bdd_run_execution_scenario("Slice with variables",
                               "slice_variables.asthra",
                               source,
                               NULL,
                               4);
}

void test_slice_access_indexing(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let array: [5]i32 = [10, 20, 30, 40, 50];\n"
        "    let slice: []i32 = array[1:4];\n"
        "    return slice[1];\n"
        "}\n";
    
    bdd_run_execution_scenario("Slice access indexing",
                               "slice_indexing.asthra",
                               source,
                               NULL,
                               30);
}

void test_empty_slice(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let array: [5]i32 = [1, 2, 3, 4, 5];\n"
        "    let empty_slice: []i32 = array[2:2];\n"
        "    // Empty slice test - just return 0 to indicate success\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty slice",
                               "slice_empty.asthra",
                               source,
                               NULL,
                               0);
}

void test_slice_function_parameter(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn process_slice(data: []i32) -> i32 {\n"
        "    let mut sum: i32 = 0;\n"
        "    for i in range(len(data)) {\n"
        "        sum = sum + data[i];\n"
        "    }\n"
        "    return sum;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let array: [4]i32 = [10, 20, 30, 40];\n"
        "    let slice: []i32 = array[1:3];\n"
        "    return process_slice(slice);\n"
        "}\n";
    
    bdd_run_execution_scenario("Slice function parameter",
                               "slice_function_param.asthra",
                               source,
                               NULL,
                               50);
}

BDD_DECLARE_TEST_CASES(slice_operations)
    BDD_TEST_CASE("basic slice start end", test_basic_slice_start_end),
    BDD_TEST_CASE("slice start only", test_slice_start_only),
    BDD_TEST_CASE("slice end only", test_slice_end_only),
    BDD_TEST_CASE("full slice copy", test_full_slice_copy),
    BDD_TEST_CASE("slice with variables", test_slice_with_variables),
    BDD_TEST_CASE("slice access indexing", test_slice_access_indexing),
    BDD_TEST_CASE("empty slice", test_empty_slice),
    BDD_TEST_CASE("slice function parameter", test_slice_function_parameter)
BDD_END_TEST_CASES()

BDD_UNIT_TEST_MAIN("Slice operations", slice_operations_test_cases)