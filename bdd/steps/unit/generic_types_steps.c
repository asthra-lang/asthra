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

// Test scenario: Generic struct with single type parameter
void test_generic_struct_single(void) {
    bdd_scenario("Generic struct with single type parameter");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Box<T> {\n"
        "    value: T\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let int_box: Box<i32> = Box { value: 42 };\n"
        "    let str_box: Box<string> = Box { value: \"hello\" };\n"
        "    log(\"Generic struct works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("generic_struct_single.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Generic struct works");
    then_exit_code_is(0);
}

// Test scenario: Generic struct with multiple type parameters
void test_generic_struct_multiple(void) {
    bdd_scenario("Generic struct with multiple type parameters");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Pair<T, U> {\n"
        "    first: T,\n"
        "    second: U\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p1: Pair<i32, string> = Pair { first: 42, second: \"answer\" };\n"
        "    let p2: Pair<bool, f64> = Pair { first: true, second: 3.14 };\n"
        "    log(\"Multiple type parameters work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("generic_struct_multiple.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Multiple type parameters work");
    then_exit_code_is(0);
}

// Test scenario: Generic enum
void test_generic_enum(void) {
    bdd_scenario("Generic enum");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub enum Option<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let some_int: Option<i32> = Option::Some(42);\n"
        "    let some_str: Option<string> = Option::Some(\"hello\");\n"
        "    let none_int: Option<i32> = Option::None;\n"
        "    log(\"Generic enum works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("generic_enum.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Generic enum works");
    then_exit_code_is(0);
}

// Test scenario: Generic function
void test_generic_function(void) {
    bdd_scenario("Generic function");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn identity<T>(value: T) -> T {\n"
        "    return value;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = identity(42);\n"
        "    let s: string = identity(\"hello\");\n"
        "    log(\"Generic function works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("generic_function.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Generic function works");
    then_exit_code_is(0);
}

// Test scenario: Generic methods
void test_generic_methods(void) {
    bdd_scenario("Generic methods");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Container<T> {\n"
        "    value: T\n"
        "}\n"
        "\n"
        "impl<T> Container<T> {\n"
        "    pub fn new(val: T) -> Container<T> {\n"
        "        return Container { value: val };\n"
        "    }\n"
        "    \n"
        "    pub fn get(self) -> T {\n"
        "        return self.value;\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let c1: Container<i32> = Container::new(42);\n"
        "    let c2: Container<string> = Container::new(\"hello\");\n"
        "    log(\"Generic methods work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("generic_methods.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Generic methods work");
    then_exit_code_is(0);
}

// Test scenario: Nested generic types
void test_nested_generics(void) {
    bdd_scenario("Nested generic types");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Box<T> {\n"
        "    value: T\n"
        "}\n"
        "\n"
        "pub enum Option<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let nested: Box<Option<i32>> = Box { \n"
        "        value: Option::Some(42) \n"
        "    };\n"
        "    log(\"Nested generics work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("nested_generics.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Nested generics work");
    then_exit_code_is(0);
}

// Test scenario: Generic type with arrays
void test_generic_arrays(void) {
    bdd_scenario("Generic type with arrays");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Vector<T> {\n"
        "    data: []T\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let v1: Vector<i32> = Vector { data: [1, 2, 3] };\n"
        "    let v2: Vector<string> = Vector { data: [\"a\", \"b\", \"c\"] };\n"
        "    log(\"Generic arrays work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("generic_arrays.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Generic arrays work");
    then_exit_code_is(0);
}

// Test scenario: Result type usage
void test_result_type(void) {
    bdd_scenario("Result type usage");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn divide(a: i32, b: i32) -> Result<i32, string> {\n"
        "    if b == 0 {\n"
        "        return Result::Err(\"Division by zero\");\n"
        "    }\n"
        "    return Result::Ok(a / b);\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let r1: Result<i32, string> = divide(10, 2);\n"
        "    let r2: Result<i32, string> = divide(10, 0);\n"
        "    log(\"Result type works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("result_type.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Result type works");
    then_exit_code_is(0);
}

// Test scenario: Option type usage
void test_option_type(void) {
    bdd_scenario("Option type usage");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn find_value(key: string) -> Option<i32> {\n"
        "    if key == \"answer\" {\n"
        "        return Option::Some(42);\n"
        "    }\n"
        "    return Option::None;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let found: Option<i32> = find_value(\"answer\");\n"
        "    let not_found: Option<i32> = find_value(\"question\");\n"
        "    log(\"Option type works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("option_type.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Option type works");
    then_exit_code_is(0);
}

// Test scenario: Missing type parameter error
void test_missing_type_param(void) {
    bdd_scenario("Missing type parameter error");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Box<T> {\n"
        "    value: T\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let b: Box = Box { value: 42 };\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("missing_type_param.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("missing type parameter");
}

// Test scenario: Wrong number of type parameters
void test_wrong_type_params(void) {
    bdd_scenario("Wrong number of type parameters");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Pair<T, U> {\n"
        "    first: T,\n"
        "    second: U\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: Pair<i32> = Pair { first: 42, second: \"hello\" };\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("wrong_type_params.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("wrong number of type parameters");
}

// Test scenario: Type parameter name conflict
void test_type_param_conflict(void) {
    bdd_scenario("Type parameter name conflict");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Container<T, T> {\n"
        "    value1: T,\n"
        "    value2: T\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("type_param_conflict.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("duplicate type parameter");
}

// Main test runner
int main(void) {
    bdd_init("Generic Types");
    
    // Check if @wip scenarios should be skipped
    if (bdd_should_skip_wip()) {
        // Skip @wip scenarios - none currently in generic_types.feature
        
        // Run all scenarios as none are marked @wip
        test_generic_struct_single();
        test_generic_struct_multiple();
        test_generic_enum();
        test_generic_function();
        test_generic_methods();
        test_nested_generics();
        test_generic_arrays();
        test_result_type();
        test_option_type();
        test_missing_type_param();
        test_wrong_type_params();
        test_type_param_conflict();
    } else {
        // Run all scenarios from generic_types.feature
        test_generic_struct_single();
        test_generic_struct_multiple();
        test_generic_enum();
        test_generic_function();
        test_generic_methods();
        test_nested_generics();
        test_generic_arrays();
        test_result_type();
        test_option_type();
        test_missing_type_param();
        test_wrong_type_params();
        test_type_param_conflict();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}