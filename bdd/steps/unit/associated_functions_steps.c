#include "bdd_unit_common.h"

void test_basic_associated_function(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Math {\n"
        "    none\n"
        "}\n"
        "\n"
        "impl Math {\n"
        "    pub fn add(a: i32, b: i32) -> i32 {\n"
        "        return a + b;\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: i32 = Math::add(5, 3);\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic associated function",
                               "assoc_basic.asthra",
                               source,
                               NULL,
                               8);
}

void test_associated_function_multiple_params(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Calculator {\n"
        "    none\n"
        "}\n"
        "\n"
        "impl Calculator {\n"
        "    pub fn multiply_and_add(a: i32, b: i32, c: i32) -> i32 {\n"
        "        return (a * b) + c;\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: i32 = Calculator::multiply_and_add(3, 4, 2);\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Associated function multiple params",
                               "assoc_multi_params.asthra",
                               source,
                               NULL,
                               14);
}

void test_associated_function_custom_return(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "impl Point {\n"
        "    pub fn new(x_val: i32, y_val: i32) -> Point {\n"
        "        return Point { x: x_val, y: y_val };\n"
        "    }\n"
        "    \n"
        "    pub fn origin(none) -> Point {\n"
        "        return Point { x: 0, y: 0 };\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let p1: Point = Point::new(3, 4);\n"
        "    let p2: Point = Point::origin();\n"
        "    return p1.x + p1.y + p2.x + p2.y;\n"
        "}\n";
    
    bdd_run_execution_scenario("Associated function custom return",
                               "assoc_custom_return.asthra",
                               source,
                               NULL,
                               7);
}

void test_multiple_associated_functions(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Utils {\n"
        "    none\n"
        "}\n"
        "\n"
        "impl Utils {\n"
        "    pub fn add(a: i32, b: i32) -> i32 {\n"
        "        return a + b;\n"
        "    }\n"
        "    \n"
        "    pub fn subtract(a: i32, b: i32) -> i32 {\n"
        "        return a - b;\n"
        "    }\n"
        "    \n"
        "    pub fn multiply(a: i32, b: i32) -> i32 {\n"
        "        return a * b;\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let sum: i32 = Utils::add(10, 5);\n"
        "    let diff: i32 = Utils::subtract(20, 8);\n"
        "    let product: i32 = Utils::multiply(3, 4);\n"
        "    return sum + diff + product;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple associated functions",
                               "assoc_multiple.asthra",
                               source,
                               NULL,
                               39);
}

void test_associated_function_in_expressions(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Math {\n"
        "    none\n"
        "}\n"
        "\n"
        "impl Math {\n"
        "    pub fn square(x: i32) -> i32 {\n"
        "        return x * x;\n"
        "    }\n"
        "    \n"
        "    pub fn double(x: i32) -> i32 {\n"
        "        return x * 2;\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: i32 = Math::square(3) + Math::double(4);\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Associated function in expressions",
                               "assoc_expressions.asthra",
                               source,
                               NULL,
                               17);
}

void test_associated_function_error_undefined(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Empty {\n"
        "    none\n"
        "}\n"
        "\n"
        "impl Empty {\n"
        "    pub fn existing_function(none) -> i32 {\n"
        "        return 42;\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: i32 = Empty::non_existent();\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_undefined_symbol_scenario("Associated function undefined error",
                                      "assoc_error_undefined.asthra",
                                      source);
}

BDD_DECLARE_TEST_CASES(associated_functions)
    BDD_TEST_CASE("basic associated function", test_basic_associated_function),
    BDD_TEST_CASE("associated function multiple params", test_associated_function_multiple_params),
    BDD_TEST_CASE("associated function custom return", test_associated_function_custom_return),
    BDD_TEST_CASE("multiple associated functions", test_multiple_associated_functions),
    BDD_TEST_CASE("associated function in expressions", test_associated_function_in_expressions),
    BDD_TEST_CASE("associated function error undefined", test_associated_function_error_undefined)
BDD_END_TEST_CASES()

BDD_UNIT_TEST_MAIN("Associated functions", associated_functions_test_cases)