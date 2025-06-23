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

// Test scenario: Compile and run a simple Hello World program
void test_hello_world(void) {
    bdd_scenario("Compile and run a simple Hello World program");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Hello, World!\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("hello.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Hello, World!");
    then_exit_code_is(0);
}

// Test scenario: Compile and run a program with multiple log statements
void test_multiple_logs(void) {
    bdd_scenario("Compile and run a program with multiple log statements");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Starting program\");\n"
        "    log(\"Processing data\");\n"
        "    log(\"Program completed\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("multiple_logs.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Starting program");
    then_output_contains("Processing data");
    then_output_contains("Program completed");
    then_exit_code_is(0);
}

// Test scenario: Compile and run a program with basic arithmetic
void test_arithmetic(void) {
    bdd_scenario("Compile and run a program with basic arithmetic");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 10;\n"
        "    let y: i32 = 20;\n"
        "    let sum: i32 = x + y;\n"
        "    \n"
        "    log(\"x = 10\");\n"
        "    log(\"y = 20\");\n"
        "    log(\"x + y = 30\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("arithmetic.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("x = 10");
    then_output_contains("y = 20");
    then_output_contains("x + y = 30");
    then_exit_code_is(0);
}

// Test scenario: Handle syntax errors gracefully
void test_syntax_error(void) {
    bdd_scenario("Handle syntax errors gracefully");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Missing semicolon\")\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("syntax_error.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("expected ';'");
}

// Test scenario: Compile and run a program with function calls
void test_function_calls(void) {
    bdd_scenario("Compile and run a program with function calls");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "fn greet(none) -> void {\n"
        "    log(\"Hello from greet function!\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Main function starting\");\n"
        "    greet();\n"
        "    log(\"Main function ending\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("function_calls.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Main function starting");
    then_output_contains("Hello from greet function!");
    then_output_contains("Main function ending");
    then_exit_code_is(0);
}

// Test scenario: Compile and run a program with boolean operations
void test_boolean_operations(void) {
    bdd_scenario("Compile and run a program with boolean operations");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let is_true: bool = true;\n"
        "    let is_false: bool = false;\n"
        "    \n"
        "    if is_true {\n"
        "        log(\"is_true is true\");\n"
        "    }\n"
        "    \n"
        "    if !is_false {\n"
        "        log(\"not false is true\");\n"
        "    }\n"
        "    \n"
        "    if is_true && !is_false {\n"
        "        log(\"true AND (NOT false) is true\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("boolean_ops.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("is_true is true");
    then_output_contains("not false is true");
    then_output_contains("true AND (NOT false) is true");
    then_exit_code_is(0);
}

// Main test runner
int main(void) {
    bdd_init("Basic Compiler Functionality");
    
    // Run all scenarios from compiler_basic.feature
    test_hello_world();
    test_multiple_logs();
    test_arithmetic();
    test_syntax_error();
    // @wip - Disabled until function calls are implemented
    // test_function_calls();
    // @wip - Disabled until boolean operations are implemented
    // test_boolean_operations();
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}