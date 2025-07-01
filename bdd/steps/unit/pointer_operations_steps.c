#include "bdd_unit_common.h"

void test_basic_address_of_operator(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let value: i32 = 42;\n"
        "    let ptr: *const i32 = &value;\n"
        "    \n"
        "    unsafe {\n"
        "        let result: i32 = *ptr;\n"
        "        return result;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic address-of operator",
                               "ptr_address_of.asthra",
                               source,
                               NULL,
                               42);
}

void test_mutable_pointer_operations(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut value: i32 = 10;\n"
        "    let ptr: *mut i32 = &value;\n"
        "    \n"
        "    unsafe {\n"
        "        *ptr = 25;\n"
        "    }\n"
        "    \n"
        "    return value;\n"
        "}\n";
    
    bdd_run_execution_scenario("Mutable pointer operations",
                               "ptr_mutable.asthra",
                               source,
                               NULL,
                               25);
}

void test_pointer_to_struct(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let point: Point = Point { x: 5, y: 10 };\n"
        "    let ptr: *const Point = &point;\n"
        "    \n"
        "    unsafe {\n"
        "        let accessed_point: Point = *ptr;\n"
        "        return accessed_point.x + accessed_point.y;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Pointer to struct",
                               "ptr_struct.asthra",
                               source,
                               NULL,
                               15);
}

void test_pointer_function_parameter(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn modify_through_pointer(ptr: *mut i32, new_value: i32) -> void {\n"
        "    unsafe {\n"
        "        *ptr = new_value;\n"
        "    }\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut value: i32 = 5;\n"
        "    modify_through_pointer(&value, 50);\n"
        "    return value;\n"
        "}\n";
    
    bdd_run_execution_scenario("Pointer function parameter",
                               "ptr_function_param.asthra",
                               source,
                               NULL,
                               50);
}

void test_pointer_error_dereference_unsafe(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let value: i32 = 42;\n"
        "    let ptr: *const i32 = &value;\n"
        "    let result: i32 = *ptr;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Pointer dereference without unsafe error",
                                 "ptr_error_unsafe.asthra",
                                 source,
                                 0,
                                 "unsafe operation");
}

BDD_DECLARE_TEST_CASES(pointer_operations)
    BDD_TEST_CASE("basic address-of operator", test_basic_address_of_operator),
    BDD_TEST_CASE("mutable pointer operations", test_mutable_pointer_operations),
    BDD_TEST_CASE("pointer to struct", test_pointer_to_struct),
    BDD_TEST_CASE("pointer function parameter", test_pointer_function_parameter),
    BDD_TEST_CASE("pointer error dereference unsafe", test_pointer_error_dereference_unsafe)
BDD_END_TEST_CASES()

BDD_UNIT_TEST_MAIN("Pointer operations", pointer_operations_test_cases)