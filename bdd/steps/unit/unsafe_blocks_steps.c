#include "bdd_unit_common.h"

void test_basic_unsafe_block(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    unsafe {\n"
        "        let value: i32 = 42;\n"
        "        return value;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic unsafe block",
                               "unsafe_basic.asthra",
                               source,
                               NULL,
                               42);
}

void test_unsafe_pointer_operations(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let value: i32 = 100;\n"
        "    let ptr: *const i32 = &value;\n"
        "    \n"
        "    unsafe {\n"
        "        let dereferenced: i32 = *ptr;\n"
        "        return dereferenced;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Unsafe pointer operations",
                               "unsafe_pointer.asthra",
                               source,
                               NULL,
                               100);
}

void test_unsafe_mutable_pointer(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut value: i32 = 50;\n"
        "    let ptr: *mut i32 = &value;\n"
        "    \n"
        "    unsafe {\n"
        "        *ptr = 75;\n"
        "    }\n"
        "    \n"
        "    return value;\n"
        "}\n";
    
    bdd_run_execution_scenario("Unsafe mutable pointer",
                               "unsafe_mut_ptr.asthra",
                               source,
                               NULL,
                               75);
}

void test_nested_unsafe_blocks(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let value: i32 = 10;\n"
        "    let ptr: *const i32 = &value;\n"
        "    \n"
        "    unsafe {\n"
        "        let intermediate: i32 = *ptr;\n"
        "        unsafe {\n"
        "            let final_value: i32 = intermediate * 2;\n"
        "            return final_value;\n"
        "        }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested unsafe blocks",
                               "unsafe_nested.asthra",
                               source,
                               NULL,
                               20);
}

void test_unsafe_with_struct(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let point: Point = Point { x: 10, y: 20 };\n"
        "    let ptr: *const Point = &point;\n"
        "    \n"
        "    unsafe {\n"
        "        let accessed_point: Point = *ptr;\n"
        "        return accessed_point.x + accessed_point.y;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Unsafe with struct",
                               "unsafe_struct.asthra",
                               source,
                               NULL,
                               30);
}

void test_unsafe_error_dereference_outside(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let value: i32 = 42;\n"
        "    let ptr: *const i32 = &value;\n"
        "    let result: i32 = *ptr;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Dereference outside unsafe error",
                                 "unsafe_error_deref.asthra",
                                 source,
                                 0,
                                 "unsafe operation");
}

BDD_DECLARE_TEST_CASES(unsafe_blocks)
    BDD_TEST_CASE("basic unsafe block", test_basic_unsafe_block),
    BDD_TEST_CASE("unsafe pointer operations", test_unsafe_pointer_operations),
    BDD_TEST_CASE("unsafe mutable pointer", test_unsafe_mutable_pointer),
    BDD_TEST_CASE("nested unsafe blocks", test_nested_unsafe_blocks),
    BDD_TEST_CASE("unsafe with struct", test_unsafe_with_struct),
    BDD_TEST_CASE("unsafe error dereference outside", test_unsafe_error_dereference_outside)
BDD_END_TEST_CASES()

BDD_UNIT_TEST_MAIN("Unsafe blocks", unsafe_blocks_test_cases)