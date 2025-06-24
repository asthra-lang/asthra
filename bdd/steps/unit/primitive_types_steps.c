#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bdd_support.h"

// External functions from common_steps.c
extern void given_asthra_compiler_available(void);
extern void given_file_with_content(const char* filename, const char* content);
extern void when_compile_file(void);
extern void when_run_executable(void);
extern void then_compilation_should_succeed(void);
extern void then_compilation_should_fail(void);
extern void then_executable_created(void);
extern void then_output_contains(const char* expected_output);
extern void then_exit_code_is(int expected_code);
extern void then_error_contains(const char* expected_error);
extern void common_cleanup(void);

// Test scenario: Integer types - i32
void test_i32_type(void) {
    bdd_scenario("Integer types - i32");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("int32_type.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("i32 type works");
    then_exit_code_is(0);
}

// Test scenario: All signed integer types
void test_all_signed_integers(void) {
    bdd_scenario("All signed integer types");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("signed_integers.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("All signed integers work");
    then_exit_code_is(0);
}

// Test scenario: All unsigned integer types
void test_all_unsigned_integers(void) {
    bdd_scenario("All unsigned integer types");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("unsigned_integers.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("All unsigned integers work");
    then_exit_code_is(0);
}

// Test scenario: Floating point types
void test_float_types(void) {
    bdd_scenario("Floating point types");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("float_types.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Float types work");
    then_exit_code_is(0);
}

// Test scenario: Boolean type
void test_bool_type(void) {
    bdd_scenario("Boolean type");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("bool_type.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Bool type works");
    then_exit_code_is(0);
}

// Test scenario: String type
void test_string_type(void) {
    bdd_scenario("String type");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("string_type.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Hello, World!");
    then_output_contains("Asthra");
    then_exit_code_is(0);
}

// Test scenario: Void type in function return
void test_void_type(void) {
    bdd_scenario("Void type in function return");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("void_type.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Void type works");
    then_exit_code_is(0);
}

// Test scenario: Type mismatch error
void test_type_mismatch(void) {
    bdd_scenario("Type mismatch error");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = \"not a number\";\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("type_mismatch.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("type mismatch");
}

// Test scenario: Integer overflow error
void test_integer_overflow(void) {
    bdd_scenario("Integer overflow error");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i8 = 128;\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("int_overflow.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("integer overflow");
}

// Test scenario: Binary literals
void test_binary_literals(void) {
    bdd_scenario("Binary literals");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: u8 = 0b11111111;\n"
        "    let y: u16 = 0b1010101010101010;\n"
        "    log(\"Binary literals work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("binary_literals.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Binary literals work");
    then_exit_code_is(0);
}

// Test scenario: Hexadecimal literals
void test_hex_literals(void) {
    bdd_scenario("Hexadecimal literals");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: u8 = 0xFF;\n"
        "    let y: u32 = 0xDEADBEEF;\n"
        "    log(\"Hex literals work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("hex_literals.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Hex literals work");
    then_exit_code_is(0);
}

// Main test runner
int main(void) {
    bdd_init("Primitive Types");
    
    if (bdd_should_skip_wip()) {
        // Skip @wip scenarios, run only passing scenarios
        test_i32_type();
        test_float_types();
        test_bool_type();
        test_string_type();
        test_void_type();
    } else {
        // Run all major scenarios from primitive_types.feature
        test_i32_type();
        test_all_signed_integers();
        test_all_unsigned_integers();
        test_float_types();
        test_bool_type();
        test_string_type();
        test_void_type();
        test_type_mismatch();
        test_integer_overflow();
        test_binary_literals();
        test_hex_literals();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}