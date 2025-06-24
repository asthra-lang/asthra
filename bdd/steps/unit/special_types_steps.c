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

// Test scenario: Unit type as void return
void test_unit_type(void) {
    bdd_scenario("Unit type as void return");
    
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
        "    log(\"Unit type works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("unit_type.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Unit type works");
    then_exit_code_is(0);
}

// Test scenario: Unit type in expressions
void test_unit_expressions(void) {
    bdd_scenario("Unit type in expressions");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let unit: void = ();\n"
        "    let result: void = if true { () } else { () };\n"
        "    log(\"Unit in expressions works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("unit_expressions.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Unit in expressions works");
    then_exit_code_is(0);
}

// Test scenario: Never type for non-returning functions
void test_never_type(void) {
    bdd_scenario("Never type for non-returning functions");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("never_type.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Never type works");
    then_exit_code_is(0);
}

// Test scenario: Size types - usize
void test_usize_type(void) {
    bdd_scenario("Size types - usize");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("usize_type.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("usize type works");
    then_exit_code_is(0);
}

// Test scenario: Size types - isize
void test_isize_type(void) {
    bdd_scenario("Size types - isize");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("isize_type.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("isize type works");
    then_exit_code_is(0);
}

// Test scenario: Size types in array operations
void test_size_array_ops(void) {
    bdd_scenario("Size types in array operations");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("size_array_ops.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Size types in arrays work");
    then_exit_code_is(0);
}

// Test scenario: SizeOf compile-time expression
void test_sizeof_expr(void) {
    bdd_scenario("SizeOf compile-time expression");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("sizeof_expr.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("sizeof expressions work");
    then_exit_code_is(0);
}

// Test scenario: Never type in match expressions
void test_never_match(void) {
    bdd_scenario("Never type in match expressions");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("never_match.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Never in match works");
    then_exit_code_is(0);
}

// Test scenario: Unit type in struct fields
void test_unit_struct_field(void) {
    bdd_scenario("Unit type in struct fields");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("unit_struct_field.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Unit in struct works");
    then_exit_code_is(0);
}

// Test scenario: Never type cannot be instantiated
void test_never_instantiate(void) {
    bdd_scenario("Never type cannot be instantiated");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: Never = panic(\"error\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("never_instantiate.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("Never type cannot be instantiated");
}

// Test scenario: Invalid sizeof usage
void test_invalid_sizeof(void) {
    bdd_scenario("Invalid sizeof usage");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 42;\n"
        "    let size: usize = sizeof(x);\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("invalid_sizeof.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("sizeof expects a type");
}

// Test scenario: Unit type comparison
void test_unit_comparison(void) {
    bdd_scenario("Unit type comparison");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("unit_comparison.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Unit values are equal");
    then_exit_code_is(0);
}

// Test scenario: Platform-specific size types
void test_platform_sizes(void) {
    bdd_scenario("Platform-specific size types");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let ptr_size: usize = sizeof(*const i32);\n"
        "    let size_size: usize = sizeof(usize);\n"
        "    log(\"Platform size types work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("platform_sizes.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Platform size types work");
    then_exit_code_is(0);
}

// Main test runner
int main(void) {
    bdd_init("Special Types");
    
    // Check if @wip scenarios should be skipped
    if (bdd_should_skip_wip()) {
        // Skip @wip scenarios - none currently in special_types.feature
        
        // Run all scenarios as none are marked @wip
        test_unit_type();
        test_unit_expressions();
        test_never_type();
        test_usize_type();
        test_isize_type();
        test_size_array_ops();
        test_sizeof_expr();
        test_never_match();
        test_unit_struct_field();
        test_never_instantiate();
        test_invalid_sizeof();
        test_unit_comparison();
        test_platform_sizes();
    } else {
        // Run all scenarios from special_types.feature
        test_unit_type();
        test_unit_expressions();
        test_never_type();
        test_usize_type();
        test_isize_type();
        test_size_array_ops();
        test_sizeof_expr();
        test_never_match();
        test_unit_struct_field();
        test_never_instantiate();
        test_invalid_sizeof();
        test_unit_comparison();
        test_platform_sizes();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}