#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"

// Test scenarios using the new reusable framework

void test_unit_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn do_nothing(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    do_nothing();\n"
        "    log(\"Unit type works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Unit type as void return",
                               "unit_type.asthra",
                               source,
                               "Unit type works",
                               0);
}

void test_unit_expressions(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let unit: void = ();\n"
        "    let result: void = if true { () } else { () };\n"
        "    log(\"Unit in expressions works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Unit type in expressions",
                               "unit_expressions.asthra",
                               source,
                               "Unit in expressions works",
                               0);
}

void test_never_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn panic(msg: string) -> Never {\n"
        "    log(msg);\n"
        "    exit(1);\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = if false {\n"
        "        panic(\"This won't happen\");\n"
        "    } else {\n"
        "        42\n"
        "    };\n"
        "    log(\"Never type works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Never type for non-returning functions",
                               "never_type.asthra",
                               source,
                               "Never type works",
                               0);
}

void test_usize_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let array_size: usize = 100;\n"
        "    let index: usize = 0;\n"
        "    let count: usize = array_size;\n"
        "    log(\"usize type works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Size types - usize",
                               "usize_type.asthra",
                               source,
                               "usize type works",
                               0);
}

void test_isize_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let offset: isize = -10;\n"
        "    let position: isize = 50;\n"
        "    let delta: isize = offset + position;\n"
        "    log(\"isize type works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Size types - isize",
                               "isize_type.asthra",
                               source,
                               "isize type works",
                               0);
}

void test_size_array_ops(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn get_array_length<T>(arr: []T) -> usize {\n"
        "    return arr.len();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let numbers: []i32 = [1, 2, 3, 4, 5];\n"
        "    let len: usize = get_array_length(numbers);\n"
        "    log(\"Size types in arrays work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Size types in array operations",
                               "size_array_ops.asthra",
                               source,
                               "Size types in arrays work",
                               0);
}

void test_sizeof_expr(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub const I32_SIZE: usize = sizeof(i32);\n"
        "pub const BOOL_SIZE: usize = sizeof(bool);\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub const POINT_SIZE: usize = sizeof(Point);\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"sizeof expressions work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("SizeOf compile-time expression",
                               "sizeof_expr.asthra",
                               source,
                               "sizeof expressions work",
                               0);
}

void test_never_match(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub enum Status {\n"
        "    Ok,\n"
        "    Error\n"
        "}\n"
        "\n"
        "pub fn handle_error(none) -> Never {\n"
        "    log(\"Fatal error\");\n"
        "    exit(1);\n"
        "}\n"
        "\n"
        "pub fn process(s: Status) -> i32 {\n"
        "    return match s {\n"
        "        Status::Ok => 42,\n"
        "        Status::Error => handle_error()\n"
        "    };\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = process(Status::Ok);\n"
        "    log(\"Never in match works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Never type in match expressions",
                               "never_match.asthra",
                               source,
                               "Never in match works",
                               0);
}

void test_unit_struct_field(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Marker {\n"
        "    tag: string,\n"
        "    placeholder: void\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let m: Marker = Marker { tag: \"test\", placeholder: () };\n"
        "    log(\"Unit in struct works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Unit type in struct fields",
                               "unit_struct_field.asthra",
                               source,
                               "Unit in struct works",
                               0);
}

void test_never_instantiate(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: Never = panic(\"error\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Never type cannot be instantiated",
                                 "never_instantiate.asthra",
                                 source,
                                 0,  // should fail
                                 "Never type cannot be instantiated");
}

void test_invalid_sizeof(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 42;\n"
        "    let size: usize = sizeof(x);\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Invalid sizeof usage",
                                 "invalid_sizeof.asthra",
                                 source,
                                 0,  // should fail
                                 "sizeof expects a type");
}

void test_unit_comparison(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let u1: void = ();\n"
        "    let u2: void = ();\n"
        "    if u1 == u2 {\n"
        "        log(\"Unit values are equal\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Unit type comparison",
                               "unit_comparison.asthra",
                               source,
                               "Unit values are equal",
                               0);
}

void test_platform_sizes(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let ptr_size: usize = sizeof(*const i32);\n"
        "    let size_size: usize = sizeof(usize);\n"
        "    log(\"Platform size types work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Platform-specific size types",
                               "platform_sizes.asthra",
                               source,
                               "Platform size types work",
                               0);
}

// Define test cases using the new framework - no @wip tags based on original file
BddTestCase special_types_test_cases[] = {
    BDD_TEST_CASE(unit_type, test_unit_type),
    BDD_TEST_CASE(unit_expressions, test_unit_expressions),
    BDD_TEST_CASE(never_type, test_never_type),
    BDD_TEST_CASE(usize_type, test_usize_type),
    BDD_TEST_CASE(isize_type, test_isize_type),
    BDD_TEST_CASE(size_array_ops, test_size_array_ops),
    BDD_TEST_CASE(sizeof_expr, test_sizeof_expr),
    BDD_TEST_CASE(never_match, test_never_match),
    BDD_TEST_CASE(unit_struct_field, test_unit_struct_field),
    BDD_TEST_CASE(never_instantiate, test_never_instantiate),
    BDD_TEST_CASE(invalid_sizeof, test_invalid_sizeof),
    BDD_TEST_CASE(unit_comparison, test_unit_comparison),
    BDD_TEST_CASE(platform_sizes, test_platform_sizes),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Special Types",
                              special_types_test_cases,
                              sizeof(special_types_test_cases) / sizeof(special_types_test_cases[0]),
                              bdd_cleanup_temp_files);
}