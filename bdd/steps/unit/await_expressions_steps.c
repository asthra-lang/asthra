#include "bdd_unit_common.h"

void test_basic_await_expression(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn compute_value(none) -> i32 {\n"
        "    return 42;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    spawn_with_handle handle = compute_value();\n"
        "    let result: i32 = await handle;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic await expression",
                               "await_basic.asthra",
                               source,
                               NULL,
                               42);
}

void test_await_with_arithmetic(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn multiply(value: i32) -> i32 {\n"
        "    return value * 3;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    spawn_with_handle handle = multiply(7);\n"
        "    let result: i32 = await handle;\n"
        "    let final_result: i32 = result + 10;\n"
        "    return final_result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Await with arithmetic",
                               "await_arithmetic.asthra",
                               source,
                               NULL,
                               31);
}

void test_multiple_await_expressions(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn add_five(value: i32) -> i32 {\n"
        "    return value + 5;\n"
        "}\n"
        "\n"
        "pub fn multiply_two(value: i32) -> i32 {\n"
        "    return value * 2;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    spawn_with_handle handle1 = add_five(10);\n"
        "    spawn_with_handle handle2 = multiply_two(6);\n"
        "    \n"
        "    let result1: i32 = await handle1;\n"
        "    let result2: i32 = await handle2;\n"
        "    \n"
        "    return result1 + result2;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple await expressions",
                               "await_multiple.asthra",
                               source,
                               NULL,
                               27);
}

void test_await_void_function(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn side_effect_task(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    spawn_with_handle handle = side_effect_task();\n"
        "    await handle;\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Await void function",
                               "await_void.asthra",
                               source,
                               NULL,
                               0);
}

void test_await_in_expressions(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn get_number(none) -> i32 {\n"
        "    return 15;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    spawn_with_handle handle = get_number();\n"
        "    let result: i32 = (await handle) * 2 + 5;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Await in expressions",
                               "await_expr.asthra",
                               source,
                               NULL,
                               35);
}

void test_await_error_non_handle(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let value: i32 = 42;\n"
        "    let result: i32 = await value;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Await non-handle error",
                                 "await_error_non_handle.asthra",
                                 source,
                                 0,
                                 "expected handle");
}

BDD_DECLARE_TEST_CASES(await_expressions)
    BDD_TEST_CASE("basic await expression", test_basic_await_expression),
    BDD_TEST_CASE("await with arithmetic", test_await_with_arithmetic),
    BDD_TEST_CASE("multiple await expressions", test_multiple_await_expressions),
    BDD_TEST_CASE("await void function", test_await_void_function),
    BDD_TEST_CASE("await in expressions", test_await_in_expressions),
    BDD_TEST_CASE("await error non-handle", test_await_error_non_handle)
BDD_END_TEST_CASES()

BDD_UNIT_TEST_MAIN("Await expressions", await_expressions_test_cases)