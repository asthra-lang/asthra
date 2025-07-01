#include "bdd_unit_common.h"
// Test scenarios for const declarations

void test_const_basic_integer(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const MAX_SIZE: i32 = 100;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = MAX_SIZE;\n"
        "    log(\"Basic const integer works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic const declaration with integer",
                               "const_basic_integer.asthra",
                               source,
                               "Basic const integer works",
                               0);
}

void test_const_float(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const PI: f64 = 3.14159;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let area: f64 = PI * 2.0 * 2.0;\n"
        "    log(\"Const float works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const declaration with float",
                               "const_float.asthra",
                               source,
                               "Const float works",
                               0);
}

void test_const_string(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const VERSION: string = \"1.0.0\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Version: \" + VERSION);\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const declaration with string",
                               "const_string.asthra",
                               source,
                               "Version: 1.0.0",
                               0);
}

void test_const_boolean(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const DEBUG_MODE: bool = true;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    if DEBUG_MODE {\n"
        "        log(\"Debug mode is enabled\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const declaration with boolean",
                               "const_boolean.asthra",
                               source,
                               "Debug mode is enabled",
                               0);
}

void test_const_binary_expr(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const SIZE: i32 = 10;\n"
        "pub const DOUBLE_SIZE: i32 = SIZE * 2;\n"
        "pub const TOTAL: i32 = DOUBLE_SIZE + 5;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = TOTAL;\n"
        "    log(\"Const binary expression works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const with binary arithmetic expression",
                               "const_binary_expr.asthra",
                               source,
                               "Const binary expression works",
                               0);
}

void test_const_unary_expr(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const POSITIVE: i32 = 42;\n"
        "pub const NEGATIVE: i32 = -POSITIVE;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let neg: i32 = NEGATIVE;\n"
        "    log(\"Const unary expression works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const with unary expression",
                               "const_unary_expr.asthra",
                               source,
                               "Const unary expression works",
                               0);
}

void test_const_sizeof(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const INT_SIZE: usize = sizeof(i32);\n"
        "pub const FLOAT_SIZE: usize = sizeof(f64);\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Const sizeof works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const with sizeof expression",
                               "const_sizeof.asthra",
                               source,
                               "Const sizeof works",
                               0);
}

void test_const_array_size(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const BUFFER_SIZE: usize = 10;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let buffer: [BUFFER_SIZE]i32 = [0; BUFFER_SIZE];\n"
        "    log(\"Const array size works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const used as array size",
                               "const_array_size.asthra",
                               source,
                               "Const array size works",
                               0);
}

void test_const_private(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "priv const SECRET_KEY: i32 = 12345;\n"
        "\n"
        "pub fn get_key(none) -> i32 {\n"
        "    return SECRET_KEY;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let key: i32 = get_key();\n"
        "    log(\"Private const works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Private const declaration",
                               "const_private.asthra",
                               source,
                               "Private const works",
                               0);
}

void test_const_hex_literal(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const HEX_VALUE: i32 = 0xFF;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let hex: i32 = HEX_VALUE;\n"
        "    log(\"Hex literal const works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const with hex literal",
                               "const_hex_literal.asthra",
                               source,
                               "Hex literal const works",
                               0);
}

void test_const_binary_literal(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const BINARY_VALUE: i32 = 0b11111111;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let bin: i32 = BINARY_VALUE;\n"
        "    log(\"Binary literal const works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const with binary literal",
                               "const_binary_literal.asthra",
                               source,
                               "Binary literal const works",
                               0);
}

void test_const_octal_literal(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const OCTAL_VALUE: i32 = 0o777;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let oct: i32 = OCTAL_VALUE;\n"
        "    log(\"Octal literal const works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const with octal literal",
                               "const_octal_literal.asthra",
                               source,
                               "Octal literal const works",
                               0);
}

void test_const_no_type_annotation(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const VALUE = 42;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Const without type annotation",
                                 "const_no_type_annotation.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "type annotation");
}

void test_const_non_const_expr(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn get_value(none) -> i32 {\n"
        "    return 42;\n"
        "}\n"
        "\n"
        "pub const VALUE: i32 = get_value();\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Const with non-const expression",
                                 "const_non_const_expr.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "constant expression");
}

void test_const_duplicate(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const VALUE: i32 = 42;\n"
        "pub const VALUE: i32 = 100;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Duplicate const declaration",
                                 "const_duplicate.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "already defined");
}

void test_const_reassignment(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const VALUE: i32 = 42;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    VALUE = 100;\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Const reassignment attempt",
                                 "const_reassignment.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "cannot assign");
}

void test_const_complex_arithmetic(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const BASE: i32 = 10;\n"
        "pub const MULTIPLIER: i32 = 3;\n"
        "pub const OFFSET: i32 = 5;\n"
        "pub const RESULT: i32 = BASE * MULTIPLIER + OFFSET;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let val: i32 = RESULT;\n"
        "    log(\"Complex const arithmetic works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const with multiple arithmetic operations",
                               "const_complex_arithmetic.asthra",
                               source,
                               "Complex const arithmetic works",
                               0);
}

void test_const_char_literal(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub const NEWLINE: char = '\\n';\n"
        "pub const TAB: char = '\\t';\n"
        "pub const LETTER_A: char = 'A';\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Char literal const works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Const with character literal",
                               "const_char_literal.asthra",
                               source,
                               "Char literal const works",
                               0);
}

// Define test cases - all active since this is a new feature
BDD_DECLARE_TEST_CASES(const_declarations)
    BDD_TEST_CASE(const_basic_integer, test_const_basic_integer),
    BDD_TEST_CASE(const_float, test_const_float),
    BDD_WIP_TEST_CASE(const_string, test_const_string),
    BDD_WIP_TEST_CASE(const_boolean, test_const_boolean),
    BDD_TEST_CASE(const_binary_expr, test_const_binary_expr),
    BDD_TEST_CASE(const_unary_expr, test_const_unary_expr),
    BDD_TEST_CASE(const_sizeof, test_const_sizeof),
    BDD_TEST_CASE(const_array_size, test_const_array_size),
    BDD_TEST_CASE(const_private, test_const_private),
    BDD_TEST_CASE(const_hex_literal, test_const_hex_literal),
    BDD_TEST_CASE(const_binary_literal, test_const_binary_literal),
    BDD_TEST_CASE(const_octal_literal, test_const_octal_literal),
    BDD_TEST_CASE(const_no_type_annotation, test_const_no_type_annotation),
    BDD_TEST_CASE(const_non_const_expr, test_const_non_const_expr),
    BDD_TEST_CASE(const_duplicate, test_const_duplicate),
    BDD_TEST_CASE(const_reassignment, test_const_reassignment),
    BDD_TEST_CASE(const_complex_arithmetic, test_const_complex_arithmetic),
    BDD_WIP_TEST_CASE(const_char_literal, test_const_char_literal),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Const Declarations", const_declarations_test_cases)