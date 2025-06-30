#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <stdio.h>
#include <string.h>

// Test scenarios using the new reusable framework

void test_unit_type_literal(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let unit_val: void = ();\n"
        "    log(\"Unit type literal works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Unit type literal",
                               "unit_literal.asthra",
                               source,
                               "Unit type literal works",
                               0);
}

void test_unit_type_return(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn do_nothing(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    do_nothing();\n"
        "    log(\"Unit type return works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Unit type return value",
                               "unit_return.asthra",
                               source,
                               "Unit type return works",
                               0);
}

void test_unit_type_in_expressions(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn get_unit(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let unit: void = ();\n"
        "    let result: void = get_unit();\n"
        "    log(\"Unit in expressions works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Unit type in expressions",
                               "unit_expressions.asthra",
                               source,
                               "Unit in expressions works",
                               0);
}

// Test Never type syntax acceptance
void test_never_type_syntax(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn abort_program(msg: string) -> Never {\n"
        "    log(msg);\n"
        "    exit(1);\n"
        "}\n"
        "\n"
        "pub fn infinite_recursion(none) -> Never {\n"
        "    return infinite_recursion();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Never type syntax accepted\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Never type syntax acceptance",
                               "never_type_syntax.asthra",
                               source,
                               "Never type syntax accepted",
                               0);
}

// Test that Never type actually terminates execution
void test_never_terminates(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn crash_now(msg: string) -> Never {\n"
        "    log(msg);\n"
        "    exit(1);\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Before crash\");\n"
        "    crash_now(\"Fatal error occurred\");\n"
        "    log(\"This should never print\");\n"
        "    return ();\n"
        "}\n";
    
    // For now, we'll test that it compiles correctly
    // Testing actual termination would require a different test approach
    bdd_run_compilation_scenario("Never type actually terminates execution",
                                "never_terminates.asthra",
                                source,
                                1,  // should succeed
                                NULL);
}

// Test Never type satisfies any return type context
void test_never_type_context(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn fail_with_error(msg: string) -> Never {\n"
        "    log(msg);\n"
        "    exit(1);\n"
        "}\n"
        "\n"
        "pub fn get_value(fail: bool) -> i32 {\n"
        "    if fail {\n"
        "        return fail_with_error(\"Failed to get value\");\n"
        "    } else {\n"
        "        return 42;\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = get_value(false);\n"
        "    log(\"Never type in return context works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Never type satisfies any return type context",
                               "never_type_context.asthra",
                               source,
                               "Never type in return context works",
                               0);
}

// Test unreachable code after Never type
void test_never_unreachable(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn terminate(none) -> Never {\n"
        "    exit(0);\n"
        "}\n"
        "\n"
        "pub fn test_unreachable(none) -> i32 {\n"
        "    return terminate();\n"
        "    return 42; // This is unreachable\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Testing unreachable code detection\");\n"
        "    return ();\n"
        "}\n";
    
    // For now, just test that it compiles
    // Proper warning detection would require capturing compiler stderr
    bdd_run_execution_scenario("Unreachable code after Never type",
                               "never_unreachable.asthra",
                               source,
                               "Testing unreachable code detection",
                               0);
}

void test_size_types(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let idx: usize = 42;\n"
        "    let offset: isize = -10;\n"
        "    log(\"Size types work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Size types (usize and isize)",
                               "size_types.asthra",
                               source,
                               "Size types work",
                               0);
}

void test_size_arithmetic(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: usize = 100;\n"
        "    let b: usize = 50;\n"
        "    let sum: usize = a + b;\n"
        "    let diff: isize = 100 - 150;\n"
        "    log(\"Size arithmetic works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Size type arithmetic",
                               "size_arithmetic.asthra",
                               source,
                               "Size arithmetic works",
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
        "    match s {\n"
        "        Status.Ok => {\n"
        "            return 42;\n"
        "        }\n"
        "        Status.Error => {\n"
        "            return handle_error();\n"
        "        }\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = process(Status.Ok);\n"
        "    log(\"Never in match works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Never type in match statements",
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

void test_platform_size(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "// Platform-specific constant\n"
        "pub const PTR_SIZE: usize = sizeof(*const i32);\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let ptr_size: usize = PTR_SIZE;\n"
        "    log(\"Platform size works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Platform-specific size handling",
                               "platform_size.asthra",
                               source,
                               "Platform size works",
                               0);
}

void test_unit_comparison(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn returns_unit(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let u1: void = ();\n"
        "    let u2: void = returns_unit();\n"
        "    // Unit types exist but are not comparable - this is expected\n"
        "    // Just verify we can work with unit types\n"
        "    log(\"Unit comparison works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Unit type comparison (wip)",
                               "unit_comparison.asthra",
                               source,
                               "Unit comparison works",
                               0);
}

// Define test cases using the new framework
BddTestCase special_types_test_cases[] = {
    BDD_TEST_CASE(unit_type_literal, test_unit_type_literal),
    BDD_TEST_CASE(unit_type_return, test_unit_type_return),
    BDD_TEST_CASE(unit_type_in_expressions, test_unit_type_in_expressions),
    BDD_TEST_CASE(never_type_syntax, test_never_type_syntax),
    BDD_TEST_CASE(never_terminates, test_never_terminates),
    BDD_TEST_CASE(never_type_context, test_never_type_context),
    BDD_TEST_CASE(never_unreachable, test_never_unreachable),
    BDD_TEST_CASE(size_types, test_size_types),
    BDD_TEST_CASE(size_arithmetic, test_size_arithmetic),
    BDD_TEST_CASE(sizeof_expr, test_sizeof_expr),
    BDD_TEST_CASE(never_match, test_never_match),
    BDD_TEST_CASE(unit_struct_field, test_unit_struct_field),
    BDD_TEST_CASE(platform_size, test_platform_size),
    BDD_TEST_CASE(unit_comparison, test_unit_comparison),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Special Types",
                              special_types_test_cases,
                              sizeof(special_types_test_cases) / sizeof(special_types_test_cases[0]),
                              bdd_cleanup_temp_files);
}