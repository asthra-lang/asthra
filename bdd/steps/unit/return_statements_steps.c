#include "bdd_unit_common.h"

void test_basic_return_integer(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn get_value(none) -> i32 {\n"
        "    return 42;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: i32 = get_value();\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic return integer",
                               "return_basic.asthra",
                               source,
                               NULL,
                               42);
}

void test_return_arithmetic_expression(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn calculate(a: i32, b: i32) -> i32 {\n"
        "    return a * b + 10;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    return calculate(3, 4);\n"
        "}\n";
    
    bdd_run_execution_scenario("Return arithmetic expression",
                               "return_arithmetic.asthra",
                               source,
                               NULL,
                               22);
}

void test_return_boolean_expression(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn is_positive(value: i32) -> bool {\n"
        "    return value > 0;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: bool = is_positive(5);\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Return boolean expression",
                               "return_boolean.asthra",
                               source,
                               NULL,
                               1);
}

void test_return_void_unit(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn do_nothing(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    do_nothing();\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Return void unit",
                               "return_void.asthra",
                               source,
                               NULL,
                               0);
}

void test_early_return_conditional(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn check_value(value: i32) -> i32 {\n"
        "    if value < 0 {\n"
        "        return -1;\n"
        "    }\n"
        "    \n"
        "    if value == 0 {\n"
        "        return 0;\n"
        "    }\n"
        "    \n"
        "    return 1;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    return check_value(-5);\n"
        "}\n";
    
    bdd_run_execution_scenario("Early return conditional",
                               "return_early.asthra",
                               source,
                               NULL,
                               255);  // -1 as unsigned byte
}

void test_return_function_call(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn multiply(a: i32, b: i32) -> i32 {\n"
        "    return a * b;\n"
        "}\n"
        "\n"
        "pub fn calculate_area(width: i32, height: i32) -> i32 {\n"
        "    return multiply(width, height);\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    return calculate_area(6, 7);\n"
        "}\n";
    
    bdd_run_execution_scenario("Return function call",
                               "return_function_call.asthra",
                               source,
                               NULL,
                               42);
}

void test_return_struct_construction(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub fn create_point(x_val: i32, y_val: i32) -> Point {\n"
        "    return Point { x: x_val, y: y_val };\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let point: Point = create_point(3, 4);\n"
        "    return point.x + point.y;\n"
        "}\n";
    
    bdd_run_execution_scenario("Return struct construction",
                               "return_struct.asthra",
                               source,
                               NULL,
                               7);
}

void test_return_complex_expression(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn complex_calculation(x: i32, y: i32, z: i32) -> i32 {\n"
        "    return (x + y) * z - (x - y) / 2;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    return complex_calculation(10, 6, 3);\n"
        "}\n";
    
    bdd_run_execution_scenario("Return complex expression",
                               "return_complex.asthra",
                               source,
                               NULL,
                               46);
}

void test_return_error_missing_expression(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn incomplete_return(none) -> i32 {\n"
        "    return;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_syntax_error_scenario("Return missing expression error",
                                  "return_error_missing.asthra",
                                  source);
}

void test_return_error_type_mismatch(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn wrong_return(none) -> i32 {\n"
        "    return \"string\";\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_type_mismatch_scenario("Return type mismatch error",
                                   "return_error_type.asthra",
                                   source);
}

BDD_DECLARE_TEST_CASES(return_statements)
    BDD_TEST_CASE("basic return integer", test_basic_return_integer),
    BDD_TEST_CASE("return arithmetic expression", test_return_arithmetic_expression),
    BDD_TEST_CASE("return boolean expression", test_return_boolean_expression),
    BDD_TEST_CASE("return void unit", test_return_void_unit),
    BDD_TEST_CASE("early return conditional", test_early_return_conditional),
    BDD_TEST_CASE("return function call", test_return_function_call),
    BDD_TEST_CASE("return struct construction", test_return_struct_construction),
    BDD_TEST_CASE("return complex expression", test_return_complex_expression),
    BDD_TEST_CASE("return error missing expression", test_return_error_missing_expression),
    BDD_TEST_CASE("return error type mismatch", test_return_error_type_mismatch)
BDD_END_TEST_CASES()

BDD_UNIT_TEST_MAIN("Return statements", return_statements_test_cases)