#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <sys/stat.h>

// Test scenarios using the new reusable framework

void test_i32_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 42;\n"
        "    let y: i32 = -100;\n"
        "    let sum: i32 = x + y;\n"
        "    log(\"i32 type works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Integer types - i32",
                               "int32_type.asthra",
                               source,
                               "i32 type works",
                               0);
}

void test_all_signed_integers(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: i8 = 127;\n"
        "    let b: i16 = 32767;\n"
        "    let c: i32 = 2147483647;\n"
        "    let d: i64 = 9223372036854775807;\n"
        "    let e: i128 = 170141183460469231731687303715884105727;\n"
        "    let f: isize = 1000;\n"
        "    log(\"All signed integers work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("All signed integer types",
                               "signed_integers.asthra",
                               source,
                               "All signed integers work",
                               0);
}

void test_all_unsigned_integers(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: u8 = 255;\n"
        "    let b: u16 = 65535;\n"
        "    let c: u32 = 4294967295;\n"
        "    let d: u64 = 18446744073709551615;\n"
        "    let e: u128 = 340282366920938463463374607431768211455;\n"
        "    let f: usize = 1000;\n"
        "    log(\"All unsigned integers work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("All unsigned integer types",
                               "unsigned_integers.asthra",
                               source,
                               "All unsigned integers work",
                               0);
}

void test_float_types(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: f32 = 3.14;\n"
        "    let y: f64 = 2.718281828;\n"
        "    let sum: f64 = y + 1.0;\n"
        "    log(\"Float types work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Floating point types",
                               "float_types.asthra",
                               source,
                               "Float types work",
                               0);
}

void test_bool_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let is_true: bool = true;\n"
        "    let is_false: bool = false;\n"
        "    let result: bool = is_true && !is_false;\n"
        "    log(\"Bool type works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Boolean type",
                               "bool_type.asthra",
                               source,
                               "Bool type works",
                               0);
}

void test_string_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let greeting: string = \"Hello, World!\";\n"
        "    let name: string = \"Asthra\";\n"
        "    log(greeting);\n"
        "    log(name);\n"
        "    return ();\n"
        "}\n";
    
    // For multiple output checks, we'll use the detailed scenario pattern
    bdd_scenario("String type");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"string_type.asthra\" with content");
    bdd_create_temp_source_file("string_type.asthra", source);
    
    bdd_when("I compile the file");
    char* executable = strdup(bdd_get_temp_source_file());
    char* dot = strrchr(executable, '.');
    if (dot) *dot = '\0';
    
    int exit_code = bdd_compile_source_file(bdd_get_temp_source_file(), executable, NULL);
    
    bdd_then("the compilation should succeed");
    BDD_ASSERT_EQ(exit_code, 0);
    
    bdd_then("an executable should be created");
    struct stat st;
    int exists = (stat(executable, &st) == 0);
    BDD_ASSERT_TRUE(exists);
    
    bdd_when("I run the executable");
    char command[512];
    snprintf(command, sizeof(command), "./%s 2>&1", executable);
    
    int execution_exit_code;
    char* execution_output = bdd_execute_command(command, &execution_exit_code);
    
    bdd_then("the output should contain \"Hello, World!\"");
    bdd_assert_output_contains(execution_output, "Hello, World!");
    
    bdd_then("the output should contain \"Asthra\"");
    bdd_assert_output_contains(execution_output, "Asthra");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_void_type(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn do_nothing(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    do_nothing();\n"
        "    log(\"Void type works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Void type in function return",
                               "void_type.asthra",
                               source,
                               "Void type works",
                               0);
}

void test_type_mismatch(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = \"not a number\";\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Type mismatch error",
                                 "type_mismatch.asthra",
                                 source,
                                 0,  // should fail
                                 "type mismatch");
}

void test_integer_overflow(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i8 = 128;\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Integer overflow error",
                                 "int_overflow.asthra",
                                 source,
                                 0,  // should fail
                                 "integer overflow");
}

void test_binary_literals(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: u8 = 0b11111111;\n"
        "    let y: u16 = 0b1010101010101010;\n"
        "    log(\"Binary literals work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Binary literals",
                               "binary_literals.asthra",
                               source,
                               "Binary literals work",
                               0);
}

void test_hex_literals(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: u8 = 0xFF;\n"
        "    let y: u32 = 0xDEADBEEF;\n"
        "    log(\"Hex literals work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Hexadecimal literals",
                               "hex_literals.asthra",
                               source,
                               "Hex literals work",
                               0);
}

// Define test cases using the new framework - @wip tags based on original file
BddTestCase primitive_types_test_cases[] = {
    BDD_TEST_CASE(i32_type, test_i32_type),
    BDD_WIP_TEST_CASE(all_signed_integers, test_all_signed_integers),
    BDD_WIP_TEST_CASE(all_unsigned_integers, test_all_unsigned_integers),
    BDD_TEST_CASE(float_types, test_float_types),
    BDD_TEST_CASE(bool_type, test_bool_type),
    BDD_TEST_CASE(string_type, test_string_type),
    BDD_TEST_CASE(void_type, test_void_type),
    BDD_WIP_TEST_CASE(type_mismatch, test_type_mismatch),
    BDD_WIP_TEST_CASE(integer_overflow, test_integer_overflow),
    BDD_WIP_TEST_CASE(binary_literals, test_binary_literals),
    BDD_WIP_TEST_CASE(hex_literals, test_hex_literals),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Primitive Types",
                              primitive_types_test_cases,
                              sizeof(primitive_types_test_cases) / sizeof(primitive_types_test_cases[0]),
                              bdd_cleanup_temp_files);
}