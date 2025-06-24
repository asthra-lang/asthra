// Example: Updated boolean operators test with @wip support
// Shows how to modify existing tests with minimal changes

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

// Test functions remain unchanged
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

void test_logical_not(void) {
    bdd_scenario("Logical NOT operator");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let val: bool = true;\n"
        "    let negated = !val;\n"
        "    \n"
        "    if negated {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_not.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

void test_logical_and(void) {
    bdd_scenario("Logical AND operator");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = true;\n"
        "    let c: bool = false;\n"
        "    \n"
        "    let result1 = a && b;\n"
        "    let result2 = a && c;\n"
        "    \n"
        "    if result1 && !result2 {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_and.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

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

// Main test runner with @wip support
int main(void) {
    bdd_init("Boolean operators");
    
    // Use the BDD_RUN_SCENARIO macro to handle @wip scenarios
    BDD_RUN_SCENARIO(test_boolean_literals, "Boolean literals", 0);          // Not @wip
    BDD_RUN_SCENARIO(test_logical_not, "Logical NOT operator", 1);          // @wip
    BDD_RUN_SCENARIO(test_logical_and, "Logical AND operator", 1);          // @wip
    BDD_RUN_SCENARIO(test_logical_or, "Logical OR operator", 0);            // Not @wip
    
    // Alternative: Manual approach for existing tests
    /*
    test_boolean_literals();
    
    if (bdd_should_skip_wip()) {
        bdd_skip_scenario("Logical NOT operator [@wip]");
        bdd_skip_scenario("Logical AND operator [@wip]");
    } else {
        test_logical_not();
        test_logical_and();
    }
    
    test_logical_or();
    */
    
    common_cleanup();
    
    return bdd_report();
}