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

// Test scenario: Call a simple function with no parameters
void test_simple_function(void) {
    bdd_scenario("Call a simple function with no parameters");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn greet(none) -> void {\n"
        "    log(\"Hello from greet function!\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    greet();\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("simple_function.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Hello from greet function!");
    then_exit_code_is(0);
}

// Test scenario: Call multiple functions in sequence
void test_multiple_functions(void) {
    bdd_scenario("Call multiple functions in sequence");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn first(none) -> void {\n"
        "    log(\"First function called\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "priv fn second(none) -> void {\n"
        "    log(\"Second function called\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "priv fn third(none) -> void {\n"
        "    log(\"Third function called\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    first();\n"
        "    second();\n"
        "    third();\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("multiple_functions.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("First function called");
    then_output_contains("Second function called");
    then_output_contains("Third function called");
    then_exit_code_is(0);
}

// Test scenario: Call a function with integer parameters
void test_function_with_params(void) {
    bdd_scenario("Call a function with integer parameters");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn add(a: i32, b: i32) -> i32 {\n"
        "    return a + b;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = add(5, 3);\n"
        "    if result == 8 {\n"
        "        log(\"Addition result is correct: 8\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("function_with_params.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Addition result is correct: 8");
    then_exit_code_is(0);
}

// Test scenario: Call a function that returns a value
void test_function_return(void) {
    bdd_scenario("Call a function that returns a value");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn get_answer(none) -> i32 {\n"
        "    return 42;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let answer: i32 = get_answer();\n"
        "    if answer == 42 {\n"
        "        log(\"The answer is 42\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("function_return.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("The answer is 42");
    then_exit_code_is(0);
}

// Test scenario: Nested function calls
void test_nested_calls(void) {
    bdd_scenario("Nested function calls");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn inner(x: i32) -> i32 {\n"
        "    log(\"Inner function called\");\n"
        "    return x * 2;\n"
        "}\n"
        "\n"
        "priv fn middle(y: i32) -> i32 {\n"
        "    log(\"Middle function called\");\n"
        "    let result: i32 = inner(y);\n"
        "    return result + 1;\n"
        "}\n"
        "\n"
        "priv fn outer(z: i32) -> i32 {\n"
        "    log(\"Outer function called\");\n"
        "    let result: i32 = middle(z);\n"
        "    return result + 10;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let final_result: i32 = outer(5);\n"
        "    // outer(5) -> middle(5) -> inner(5) -> 10 -> 11 -> 21\n"
        "    if final_result == 21 {\n"
        "        log(\"Nested calls result: 21\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("nested_calls.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Outer function called");
    then_output_contains("Middle function called");
    then_output_contains("Inner function called");
    then_output_contains("Nested calls result: 21");
    then_exit_code_is(0);
}

// Test scenario: Function with multiple parameters of different types
void test_mixed_params(void) {
    bdd_scenario("Function with multiple parameters of different types");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn process_data(count: i32, flag: bool, value: f32) -> void {\n"
        "    if flag {\n"
        "        log(\"Processing with flag enabled\");\n"
        "        if count > 0 {\n"
        "            log(\"Count is positive\");\n"
        "        }\n"
        "        if value > 0.0 {\n"
        "            log(\"Value is positive\");\n"
        "        }\n"
        "    } else {\n"
        "        log(\"Processing with flag disabled\");\n"
        "    }\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    process_data(5, true, 3.14);\n"
        "    process_data(0, false, -1.0);\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("mixed_params.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Processing with flag enabled");
    then_output_contains("Count is positive");
    then_output_contains("Value is positive");
    then_output_contains("Processing with flag disabled");
    then_exit_code_is(0);
}

// Test scenario: Recursive function calls
void test_recursive(void) {
    bdd_scenario("Recursive function calls");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn factorial(n: i32) -> i32 {\n"
        "    if n <= 1 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return n * factorial(n - 1);\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = factorial(5);\n"
        "    // 5! = 5 * 4 * 3 * 2 * 1 = 120\n"
        "    if result == 120 {\n"
        "        log(\"Factorial of 5 is 120\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("recursive.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Factorial of 5 is 120");
    then_exit_code_is(0);
}

// Test scenario: Function call in expression context
void test_function_in_expression(void) {
    bdd_scenario("Function call in expression context");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn double(x: i32) -> i32 {\n"
        "    return x * 2;\n"
        "}\n"
        "\n"
        "priv fn triple(x: i32) -> i32 {\n"
        "    return x * 3;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = double(5) + triple(3);\n"
        "    // double(5) = 10, triple(3) = 9, total = 19\n"
        "    if result == 19 {\n"
        "        log(\"Expression result is 19\");\n"
        "    }\n"
        "    \n"
        "    // Function calls in condition\n"
        "    if double(2) == 4 {\n"
        "        log(\"Double of 2 is 4\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("function_in_expression.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Expression result is 19");
    then_output_contains("Double of 2 is 4");
    then_exit_code_is(0);
}

// Test scenario: Forward function declaration
void test_forward_declaration(void) {
    bdd_scenario("Forward function declaration");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "// main calls helper before it's defined\n"
        "pub fn main(none) -> void {\n"
        "    helper();\n"
        "    return ();\n"
        "}\n"
        "\n"
        "priv fn helper(none) -> void {\n"
        "    log(\"Helper function called\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("forward_declaration.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Helper function called");
    then_exit_code_is(0);
}

// Test scenario: Error - calling undefined function
void test_undefined_function_error(void) {
    bdd_scenario("Error - calling undefined function");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    undefined_function();\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("undefined_function.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("undefined function");
}

// Test scenario: Error - incorrect number of arguments
void test_wrong_arg_count_error(void) {
    bdd_scenario("Error - incorrect number of arguments");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn add(a: i32, b: i32) -> i32 {\n"
        "    return a + b;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = add(5);  // Missing second argument\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("wrong_arg_count.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("incorrect number of arguments");
}

// Test scenario: Error - type mismatch in function arguments
void test_type_mismatch_error(void) {
    bdd_scenario("Error - type mismatch in function arguments");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn process_int(x: i32) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    process_int(3.14);  // Passing float to int parameter\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("type_mismatch.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("type mismatch");
}

// Main test runner
int main(void) {
    bdd_init("Function Call Functionality");
    
    // @wip scenarios - skip all function call tests until compiler is available
    if (bdd_should_skip_wip()) {
        bdd_skip_scenario("Call a simple function with no parameters [@wip]");
        bdd_skip_scenario("Call multiple functions in sequence [@wip]");
        bdd_skip_scenario("Call a function with integer parameters [@wip]");
        bdd_skip_scenario("Call a function that returns a value [@wip]");
        bdd_skip_scenario("Nested function calls [@wip]");
        bdd_skip_scenario("Function with multiple parameters of different types [@wip]");
        bdd_skip_scenario("Recursive function calls [@wip]");
        bdd_skip_scenario("Function call in expression context [@wip]");
        bdd_skip_scenario("Forward function declaration [@wip]");
        bdd_skip_scenario("Error - calling undefined function [@wip]");
        bdd_skip_scenario("Error - incorrect number of arguments [@wip]");
        bdd_skip_scenario("Error - type mismatch in function arguments [@wip]");
    } else {
        test_simple_function();
        // test_multiple_functions();
        // test_function_with_params();
        // test_function_return();
        // test_nested_calls();
        // test_mixed_params();
        // test_recursive();
        // test_function_in_expression();
        // test_forward_declaration();
        // test_undefined_function_error();
        // test_wrong_arg_count_error();
        // test_type_mismatch_error();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}