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

// Test scenario: Boolean literals
void test_boolean_literals(void) {
    bdd_scenario("Boolean literals");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let t: bool = true;\n"
        "    let f: bool = false;\n"
        "    \n"
        "    if t && !f {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_literals.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Logical OR operator
void test_logical_or(void) {
    bdd_scenario("Logical OR operator");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = false;\n"
        "    \n"
        "    let result1 = a || b;\n"
        "    let result2 = b || c;\n"
        "    \n"
        "    if result1 && !result2 {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_or.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Boolean operator precedence
void test_boolean_precedence(void) {
    bdd_scenario("Boolean operator precedence");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    \n"
        "    // AND has higher precedence than OR\n"
        "    let result1 = a || b && c;  // Should be a || (b && c) = true\n"
        "    let result2 = (a || b) && c; // Should be true && true = true\n"
        "    \n"
        "    if result1 && result2 {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_precedence.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Complex boolean expressions
void test_complex_boolean(void) {
    bdd_scenario("Complex boolean expressions");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    let d: bool = false;\n"
        "    \n"
        "    let complex = (a && b) || (c && !d);\n"
        "    \n"
        "    if complex {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_complex.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Error test scenarios
void test_type_mismatch_not(void) {
    bdd_scenario("Error - Type mismatch in boolean operation");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 42;\n"
        "    let result = !x;  // Error: NOT on non-boolean\n"
        "    return 0;\n"
        "}\n";
    
    given_file_with_content("bool_error_not.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("type mismatch");
}

void test_type_mismatch_and(void) {
    bdd_scenario("Error - Non-boolean in logical AND");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 42;\n"
        "    let y: bool = true;\n"
        "    let result = x && y;  // Error: AND with non-boolean\n"
        "    return 0;\n"
        "}\n";
    
    given_file_with_content("bool_error_and.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("type mismatch");
}

void test_type_mismatch_if(void) {
    bdd_scenario("Error - Non-boolean condition in if");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 42;\n"
        "    if x {  // Error: non-boolean condition\n"
        "        return 0;\n"
        "    }\n"
        "    return 1;\n"
        "}\n";
    
    given_file_with_content("bool_error_if.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("condition");
}

// Main test runner
int main(void) {
    bdd_init("Boolean operators");
    
    // Run scenarios that are NOT marked with @wip
    test_boolean_literals();
    test_logical_or();
    test_boolean_precedence();
    test_complex_boolean();
    test_type_mismatch_not();
    test_type_mismatch_and();
    test_type_mismatch_if();
    
    // @wip scenarios - automatically skip if BDD_SKIP_WIP != 0
    if (bdd_should_skip_wip()) {
        // The following scenarios are marked as @wip in the feature file
        bdd_skip_scenario("Logical NOT operator [@wip]");
        bdd_skip_scenario("Logical AND operator [@wip]");
        bdd_skip_scenario("Boolean as values [@wip]");
        bdd_skip_scenario("Short-circuit evaluation - AND [@wip]");
        bdd_skip_scenario("Short-circuit evaluation - OR [@wip]");
        bdd_skip_scenario("Nested boolean expressions [@wip]");
        bdd_skip_scenario("Boolean type inference [@wip]");
        bdd_skip_scenario("Mutable boolean variables [@wip]");
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}