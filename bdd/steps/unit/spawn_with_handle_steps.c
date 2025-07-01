#include "bdd_unit_common.h"

void test_basic_spawn_with_handle(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn compute_task(none) -> i32 {\n"
        "    return 42;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    spawn_with_handle handle = compute_task();\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic spawn with handle",
                               "spawn_handle_basic.asthra",
                               source,
                               NULL,
                               0);
}

void test_spawn_handle_with_parameters(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn multiply_task(value: i32) -> i32 {\n"
        "    return value * 2;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    spawn_with_handle result_handle = multiply_task(21);\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Spawn handle with parameters",
                               "spawn_handle_params.asthra",
                               source,
                               NULL,
                               0);
}

void test_spawn_handle_custom_type(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct TaskResult {\n"
        "    value: i32,\n"
        "    success: bool\n"
        "}\n"
        "\n"
        "pub fn create_result(val: i32) -> TaskResult {\n"
        "    return TaskResult { value: val, success: true };\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    spawn_with_handle handle = create_result(100);\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Spawn handle with custom type",
                               "spawn_handle_custom.asthra",
                               source,
                               NULL,
                               0);
}

void test_spawn_handle_error_missing_name(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn task(none) -> i32 {\n"
        "    return 42;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    spawn_with_handle = task();\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_syntax_error_scenario("Spawn handle missing name error",
                                  "spawn_handle_error_name.asthra",
                                  source);
}

void test_spawn_handle_error_missing_assignment(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn task(none) -> i32 {\n"
        "    return 42;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    spawn_with_handle handle task();\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_syntax_error_scenario("Spawn handle missing assignment error",
                                  "spawn_handle_error_assign.asthra",
                                  source);
}

BDD_DECLARE_TEST_CASES(spawn_with_handle)
    BDD_TEST_CASE("basic spawn with handle", test_basic_spawn_with_handle),
    BDD_TEST_CASE("spawn handle with parameters", test_spawn_handle_with_parameters),
    BDD_TEST_CASE("spawn handle custom type", test_spawn_handle_custom_type),
    BDD_TEST_CASE("spawn handle error missing name", test_spawn_handle_error_missing_name),
    BDD_TEST_CASE("spawn handle error missing assignment", test_spawn_handle_error_missing_assignment)
BDD_END_TEST_CASES()

BDD_UNIT_TEST_MAIN("Spawn with handle", spawn_with_handle_test_cases)