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
extern void then_executable_created(void);
extern void then_output_contains(const char* expected_output);
extern void then_exit_code_is(int expected_code);
extern void common_cleanup(void);
extern const char* get_execution_output(void);

// New step definition for negative output check
void then_output_should_not_contain(const char* unexpected_output) {
    char desc[256];
    snprintf(desc, sizeof(desc), "the output should not contain \"%s\"", unexpected_output);
    bdd_then(desc);
    
    const char* execution_output = get_execution_output();
    BDD_ASSERT_NOT_NULL(execution_output);
    if (execution_output) {
        BDD_ASSERT_TRUE(strstr(execution_output, unexpected_output) == NULL);
    }
}

// Test implementations for if_conditions.feature scenarios
void test_simple_if_true(void) {
    bdd_scenario("Simple if condition with true branch");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    if true {\n"
        "        log(\"Condition is true\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("if_true.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Condition is true");
    then_exit_code_is(0);
}

void test_simple_if_false(void) {
    bdd_scenario("Simple if condition with false branch");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    if false {\n"
        "        log(\"This should not print\");\n"
        "    }\n"
        "    log(\"Program completed\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("if_false.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_should_not_contain("This should not print");
    then_output_contains("Program completed");
    then_exit_code_is(0);
}

void test_if_else_condition(void) {
    bdd_scenario("If-else condition");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 10;\n"
        "    if x > 5 {\n"
        "        log(\"x is greater than 5\");\n"
        "    } else {\n"
        "        log(\"x is not greater than 5\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("if_else.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("x is greater than 5");
    then_output_should_not_contain("x is not greater than 5");
    then_exit_code_is(0);
}

void test_nested_if_conditions(void) {
    bdd_scenario("Nested if conditions");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 20;\n"
        "    \n"
        "    if a > 5 {\n"
        "        log(\"a is greater than 5\");\n"
        "        if b > 15 {\n"
        "            log(\"b is also greater than 15\");\n"
        "        }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("nested_if.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("a is greater than 5");
    then_output_contains("b is also greater than 15");
    then_exit_code_is(0);
}

void test_if_expression_result(void) {
    bdd_scenario("If condition with expression result");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = if true { 42 } else { 0 };\n"
        "    if result == 42 {\n"
        "        log(\"Result is 42\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("if_expression.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Result is 42");
    then_exit_code_is(0);
}

void test_complex_boolean_expression(void) {
    bdd_scenario("If condition with complex boolean expression");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 10;\n"
        "    let y: i32 = 20;\n"
        "    let z: i32 = 30;\n"
        "    \n"
        "    if x < y && y < z {\n"
        "        log(\"x < y < z is true\");\n"
        "    }\n"
        "    \n"
        "    if x > 5 || y < 10 {\n"
        "        log(\"At least one condition is true\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("complex_condition.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("x < y < z is true");
    then_output_contains("At least one condition is true");
    then_exit_code_is(0);
}

// Main test runner
int main(void) {
    bdd_init("If Condition Functionality");
    
    // Run all scenarios
    test_simple_if_true();
    test_simple_if_false();
    test_if_else_condition();
    test_nested_if_conditions();
    test_complex_boolean_expression(); // Test the @wip scenario
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}