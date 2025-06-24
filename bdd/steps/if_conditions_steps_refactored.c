#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"

// Test scenarios using the new reusable framework

void test_simple_if_true(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    if true {\n"
        "        log(\"Condition is true\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple if condition with true branch",
                               "if_true.asthra",
                               source,
                               "Condition is true",
                               0);
}

void test_simple_if_false(void) {
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
    
    // For negative output checks, we'll use the detailed scenario pattern
    bdd_scenario("Simple if condition with false branch");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"if_false.asthra\" with content");
    bdd_create_temp_source_file("if_false.asthra", source);
    
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
    
    bdd_then("the output should not contain \"This should not print\"");
    BDD_ASSERT_TRUE(strstr(execution_output, "This should not print") == NULL);
    
    bdd_then("the output should contain \"Program completed\"");
    bdd_assert_output_contains(execution_output, "Program completed");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_if_else_condition(void) {
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
    
    // For multiple output checks with negatives, we'll use the detailed scenario pattern
    bdd_scenario("If-else condition");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"if_else.asthra\" with content");
    bdd_create_temp_source_file("if_else.asthra", source);
    
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
    
    bdd_then("the output should contain \"x is greater than 5\"");
    bdd_assert_output_contains(execution_output, "x is greater than 5");
    
    bdd_then("the output should not contain \"x is not greater than 5\"");
    BDD_ASSERT_TRUE(strstr(execution_output, "x is not greater than 5") == NULL);
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_nested_if_conditions(void) {
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
    
    // For multiple output checks, we'll use the detailed scenario pattern
    bdd_scenario("Nested if conditions");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"nested_if.asthra\" with content");
    bdd_create_temp_source_file("nested_if.asthra", source);
    
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
    
    bdd_then("the output should contain \"a is greater than 5\"");
    bdd_assert_output_contains(execution_output, "a is greater than 5");
    
    bdd_then("the output should contain \"b is also greater than 15\"");
    bdd_assert_output_contains(execution_output, "b is also greater than 15");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_if_expression_result(void) {
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
    
    bdd_run_execution_scenario("If condition with expression result",
                               "if_expression.asthra",
                               source,
                               "Result is 42",
                               0);
}

void test_complex_boolean_expression(void) {
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
    
    // For multiple output checks, we'll use the detailed scenario pattern
    bdd_scenario("If condition with complex boolean expression");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"complex_condition.asthra\" with content");
    bdd_create_temp_source_file("complex_condition.asthra", source);
    
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
    
    bdd_then("the output should contain \"x < y < z is true\"");
    bdd_assert_output_contains(execution_output, "x < y < z is true");
    
    bdd_then("the output should contain \"At least one condition is true\"");
    bdd_assert_output_contains(execution_output, "At least one condition is true");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

// Define test cases using the new framework - all marked @wip based on original file
BddTestCase if_conditions_test_cases[] = {
    BDD_WIP_TEST_CASE(simple_if_true, test_simple_if_true),
    BDD_WIP_TEST_CASE(simple_if_false, test_simple_if_false),
    BDD_WIP_TEST_CASE(if_else_condition, test_if_else_condition),
    BDD_WIP_TEST_CASE(nested_if_conditions, test_nested_if_conditions),
    BDD_WIP_TEST_CASE(if_expression_result, test_if_expression_result),
    BDD_WIP_TEST_CASE(complex_boolean_expression, test_complex_boolean_expression),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("If Condition Functionality",
                              if_conditions_test_cases,
                              sizeof(if_conditions_test_cases) / sizeof(if_conditions_test_cases[0]),
                              bdd_cleanup_temp_files);
}