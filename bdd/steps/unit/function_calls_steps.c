#include "bdd_unit_common.h"
// Test scenarios using the new reusable framework

void test_simple_function(void) {
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
    
    bdd_run_execution_scenario("Call a simple function with no parameters",
                               "simple_function.asthra",
                               source,
                               "Hello from greet function!",
                               0);
}

void test_multiple_functions(void) {
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
    
    // For multiple output checks, we'll use the detailed scenario pattern
    bdd_scenario("Call multiple functions in sequence");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"multiple_functions.asthra\" with content");
    bdd_create_temp_source_file("multiple_functions.asthra", source);
    
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
    
    bdd_then("the output should contain \"First function called\"");
    bdd_assert_output_contains(execution_output, "First function called");
    
    bdd_then("the output should contain \"Second function called\"");
    bdd_assert_output_contains(execution_output, "Second function called");
    
    bdd_then("the output should contain \"Third function called\"");
    bdd_assert_output_contains(execution_output, "Third function called");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_function_with_params(void) {
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
    
    bdd_run_execution_scenario("Call a function with integer parameters",
                               "function_with_params.asthra",
                               source,
                               "Addition result is correct: 8",
                               0);
}

void test_function_return(void) {
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
    
    bdd_run_execution_scenario("Call a function that returns a value",
                               "function_return.asthra",
                               source,
                               "The answer is 42",
                               0);
}

void test_nested_calls(void) {
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
        "    if final_result == 21 {\n"
        "        log(\"Nested calls result: 21\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    // For multiple output checks, we'll use the detailed scenario pattern
    bdd_scenario("Nested function calls");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"nested_calls.asthra\" with content");
    bdd_create_temp_source_file("nested_calls.asthra", source);
    
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
    
    bdd_then("the output should contain \"Outer function called\"");
    bdd_assert_output_contains(execution_output, "Outer function called");
    
    bdd_then("the output should contain \"Middle function called\"");
    bdd_assert_output_contains(execution_output, "Middle function called");
    
    bdd_then("the output should contain \"Inner function called\"");
    bdd_assert_output_contains(execution_output, "Inner function called");
    
    bdd_then("the output should contain \"Nested calls result: 21\"");
    bdd_assert_output_contains(execution_output, "Nested calls result: 21");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_undefined_function_error(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    undefined_function();\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Error - calling undefined function",
                                 "undefined_function.asthra",
                                 source,
                                 0,  // should fail
                                 "undefined function");
}

void test_wrong_arg_count_error(void) {
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
    
    bdd_run_compilation_scenario("Error - incorrect number of arguments",
                                 "wrong_arg_count.asthra",
                                 source,
                                 0,  // should fail
                                 "incorrect number of arguments");
}

void test_type_mismatch_error(void) {
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
    
    bdd_run_compilation_scenario("Error - type mismatch in function arguments",
                                 "type_mismatch.asthra",
                                 source,
                                 0,  // should fail
                                 "type mismatch");
}

// Define test cases using the new framework - all marked @wip based on original file
BDD_DECLARE_TEST_CASES(function_calls)
    BDD_WIP_TEST_CASE(simple_function, test_simple_function),
    BDD_WIP_TEST_CASE(multiple_functions, test_multiple_functions),
    BDD_WIP_TEST_CASE(function_with_params, test_function_with_params),
    BDD_WIP_TEST_CASE(function_return, test_function_return),
    BDD_TEST_CASE(nested_calls, test_nested_calls),
    BDD_WIP_TEST_CASE(undefined_function_error, test_undefined_function_error),
    BDD_WIP_TEST_CASE(wrong_arg_count_error, test_wrong_arg_count_error),
    BDD_WIP_TEST_CASE(type_mismatch_error, test_type_mismatch_error),
BDD_END_TEST_CASES()

// Main test runner using the new framework
BDD_UNIT_TEST_MAIN("Function Call Functionality", function_calls_test_cases)