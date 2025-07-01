#include "bdd_unit_common.h"
// Test scenarios for numeric literals

// Decimal integer literals
void test_simple_decimal_integer(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple decimal integer",
                               "decimal_simple.asthra",
                               source,
                               NULL,
                               42);
}

void test_zero_decimal_integer(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Zero decimal integer",
                               "decimal_zero.asthra",
                               source,
                               NULL,
                               0);
}

void test_large_decimal_integer(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let x: i32 = 123456;\n"
        "  return x % 100;\n"
        "}\n";
    
    bdd_run_execution_scenario("Large decimal integer",
                               "decimal_large.asthra",
                               source,
                               NULL,
                               56);
}

// Hexadecimal literals
void test_lowercase_hex_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0x2a;\n"
        "}\n";
    
    bdd_run_execution_scenario("Lowercase hex literal",
                               "hex_lowercase.asthra",
                               source,
                               NULL,
                               42);
}

void test_uppercase_hex_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0X2A;\n"
        "}\n";
    
    bdd_run_execution_scenario("Uppercase hex literal",
                               "hex_uppercase.asthra",
                               source,
                               NULL,
                               42);
}

void test_mixed_case_hex_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0xFf;\n"
        "}\n";
    
    bdd_run_execution_scenario("Mixed case hex literal",
                               "hex_mixed_case.asthra",
                               source,
                               NULL,
                               255);
}

void test_multi_digit_hex_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let x: i32 = 0xDEADBEEF;\n"
        "  return x & 0xFF;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multi-digit hex literal",
                               "hex_multi_digit.asthra",
                               source,
                               NULL,
                               239);
}

// Binary literals
void test_lowercase_binary_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0b101010;\n"
        "}\n";
    
    bdd_run_execution_scenario("Lowercase binary literal",
                               "binary_lowercase.asthra",
                               source,
                               NULL,
                               42);
}

void test_uppercase_binary_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0B101010;\n"
        "}\n";
    
    bdd_run_execution_scenario("Uppercase binary literal",
                               "binary_uppercase.asthra",
                               source,
                               NULL,
                               42);
}

void test_single_bit_binary_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0b1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Single bit binary literal",
                               "binary_single_bit.asthra",
                               source,
                               NULL,
                               1);
}

void test_multi_bit_binary_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0b11111111;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multi-bit binary literal",
                               "binary_multi_bit.asthra",
                               source,
                               NULL,
                               255);
}

// Octal literals
void test_simple_octal_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0o52;\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple octal literal",
                               "octal_simple.asthra",
                               source,
                               NULL,
                               42);
}

void test_single_digit_octal_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0o7;\n"
        "}\n";
    
    bdd_run_execution_scenario("Single digit octal literal",
                               "octal_single_digit.asthra",
                               source,
                               NULL,
                               7);
}

void test_multi_digit_octal_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return 0o377;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multi-digit octal literal",
                               "octal_multi_digit.asthra",
                               source,
                               NULL,
                               255);
}

// Float literals
void test_simple_float_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let f: f32 = 42.0;\n"
        "  return f as i32;\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple float literal",
                               "float_simple.asthra",
                               source,
                               NULL,
                               42);
}

void test_float_with_fractional_part(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let f: f32 = 42.5;\n"
        "  return f as i32;\n"
        "}\n";
    
    bdd_run_execution_scenario("Float with fractional part",
                               "float_fractional.asthra",
                               source,
                               NULL,
                               42);
}

void test_float_without_trailing_digits(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let f: f32 = 42.;\n"
        "  return f as i32;\n"
        "}\n";
    
    bdd_run_execution_scenario("Float without trailing digits",
                               "float_no_trailing.asthra",
                               source,
                               NULL,
                               42);
}

void test_float_starting_with_decimal_point(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let f: f32 = .42;\n"
        "  let result: i32 = (f * 100.0) as i32;\n"
        "  return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Float starting with decimal point",
                               "float_leading_dot.asthra",
                               source,
                               NULL,
                               42);
}

// Mixed numeric operations
void test_arithmetic_with_different_literal_formats(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let dec: i32 = 10;\n"
        "  let hex: i32 = 0x10;\n"
        "  let bin: i32 = 0b10000;\n"
        "  let oct: i32 = 0o6;\n"
        "  return dec + hex + bin + oct;\n"
        "}\n";
    
    bdd_run_execution_scenario("Arithmetic with different literal formats",
                               "mixed_arithmetic.asthra",
                               source,
                               NULL,
                               42);
}

// Type inference with literals
void test_integer_literal_type_inference(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let x: i8 = 42;\n"
        "  let y: i16 = 42;\n"
        "  let z: i32 = 42;\n"
        "  return (x as i32) + (y as i32) + z - 84;\n"
        "}\n";
    
    bdd_run_execution_scenario("Integer literal type inference",
                               "int_type_inference.asthra",
                               source,
                               NULL,
                               42);
}

void test_float_literal_type_inference(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let x: f32 = 21.0;\n"
        "  let y: f64 = 21.0;\n"
        "  return (x as i32) + (y as i32);\n"
        "}\n";
    
    bdd_run_execution_scenario("Float literal type inference",
                               "float_type_inference.asthra",
                               source,
                               NULL,
                               42);
}

// Edge cases
void test_maximum_u8_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let max: u8 = 255;\n"
        "  return (max as i32) - 213;\n"
        "}\n";
    
    bdd_run_execution_scenario("Maximum u8 value",
                               "max_u8.asthra",
                               source,
                               NULL,
                               42);
}

void test_negative_decimal_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let neg: i32 = -42;\n"
        "  return -neg;\n"
        "}\n";
    
    bdd_run_execution_scenario("Negative decimal literal",
                               "negative_decimal.asthra",
                               source,
                               NULL,
                               42);
}

void test_negative_hex_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let neg: i32 = -0x2A;\n"
        "  return -neg;\n"
        "}\n";
    
    bdd_run_execution_scenario("Negative hex literal",
                               "negative_hex.asthra",
                               source,
                               NULL,
                               42);
}

// Literal comparisons
void test_compare_different_literal_formats(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  if 42 == 0x2A && 0x2A == 0o52 && 0o52 == 0b101010 {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Compare different literal formats",
                               "compare_literals.asthra",
                               source,
                               NULL,
                               42);
}

// Literals in expressions
void test_complex_expression_with_literals(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  return (0xFF & 0b11111111) - (0o377 - 0x2A);\n"
        "}\n";
    
    bdd_run_execution_scenario("Complex expression with literals",
                               "complex_expr.asthra",
                               source,
                               NULL,
                               42);
}

// Const expressions with literals
void test_const_with_decimal_literal(void) {
    const char* source = 
        "package test;\n"
        "pub const ANSWER: i32 = 42;\n"
        "pub fn main(none) -> i32 {\n"
        "  return ANSWER;\n"
        "}\n";
    
    bdd_run_execution_scenario("Const with decimal literal",
                               "const_decimal.asthra",
                               source,
                               NULL,
                               42);
}

void test_const_with_hex_literal(void) {
    const char* source = 
        "package test;\n"
        "pub const HEX_ANSWER: i32 = 0x2A;\n"
        "pub fn main(none) -> i32 {\n"
        "  return HEX_ANSWER;\n"
        "}\n";
    
    bdd_run_execution_scenario("Const with hex literal",
                               "const_hex.asthra",
                               source,
                               NULL,
                               42);
}

// Zero literals in different bases
void test_zero_in_all_bases(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let dec_zero: i32 = 0;\n"
        "  let hex_zero: i32 = 0x0;\n"
        "  let bin_zero: i32 = 0b0;\n"
        "  let oct_zero: i32 = 0o0;\n"
        "  if dec_zero == hex_zero && hex_zero == bin_zero && bin_zero == oct_zero {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 1;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Zero in all bases",
                               "zero_all_bases.asthra",
                               source,
                               NULL,
                               42);
}

// Define test cases
BDD_DECLARE_TEST_CASES(numeric_literals)
    // Decimal integer literals
    BDD_WIP_TEST_CASE(simple_decimal_integer, test_simple_decimal_integer),
    BDD_TEST_CASE(zero_decimal_integer, test_zero_decimal_integer),
    BDD_WIP_TEST_CASE(large_decimal_integer, test_large_decimal_integer),
    
    // Hexadecimal literals
    BDD_WIP_TEST_CASE(lowercase_hex_literal, test_lowercase_hex_literal),
    BDD_WIP_TEST_CASE(uppercase_hex_literal, test_uppercase_hex_literal),
    BDD_WIP_TEST_CASE(mixed_case_hex_literal, test_mixed_case_hex_literal),
    BDD_WIP_TEST_CASE(multi_digit_hex_literal, test_multi_digit_hex_literal),
    
    // Binary literals
    BDD_WIP_TEST_CASE(lowercase_binary_literal, test_lowercase_binary_literal),
    BDD_WIP_TEST_CASE(uppercase_binary_literal, test_uppercase_binary_literal),
    BDD_WIP_TEST_CASE(single_bit_binary_literal, test_single_bit_binary_literal),
    BDD_WIP_TEST_CASE(multi_bit_binary_literal, test_multi_bit_binary_literal),
    
    // Octal literals
    BDD_WIP_TEST_CASE(simple_octal_literal, test_simple_octal_literal),
    BDD_WIP_TEST_CASE(single_digit_octal_literal, test_single_digit_octal_literal),
    BDD_WIP_TEST_CASE(multi_digit_octal_literal, test_multi_digit_octal_literal),
    
    // Float literals
    BDD_WIP_TEST_CASE(simple_float_literal, test_simple_float_literal),
    BDD_WIP_TEST_CASE(float_with_fractional_part, test_float_with_fractional_part),
    BDD_WIP_TEST_CASE(float_without_trailing_digits, test_float_without_trailing_digits),
    BDD_WIP_TEST_CASE(float_starting_with_decimal_point, test_float_starting_with_decimal_point),
    
    // Mixed numeric operations
    BDD_WIP_TEST_CASE(arithmetic_with_different_literal_formats, test_arithmetic_with_different_literal_formats),
    
    // Type inference with literals
    BDD_WIP_TEST_CASE(integer_literal_type_inference, test_integer_literal_type_inference),
    BDD_WIP_TEST_CASE(float_literal_type_inference, test_float_literal_type_inference),
    
    // Edge cases
    BDD_WIP_TEST_CASE(maximum_u8_value, test_maximum_u8_value),
    BDD_WIP_TEST_CASE(negative_decimal_literal, test_negative_decimal_literal),
    BDD_WIP_TEST_CASE(negative_hex_literal, test_negative_hex_literal),
    
    // Literal comparisons
    BDD_WIP_TEST_CASE(compare_different_literal_formats, test_compare_different_literal_formats),
    
    // Literals in expressions
    BDD_WIP_TEST_CASE(complex_expression_with_literals, test_complex_expression_with_literals),
    
    // Const expressions with literals
    BDD_WIP_TEST_CASE(const_with_decimal_literal, test_const_with_decimal_literal),
    BDD_WIP_TEST_CASE(const_with_hex_literal, test_const_with_hex_literal),
    
    // Zero literals in different bases
    BDD_WIP_TEST_CASE(zero_in_all_bases, test_zero_in_all_bases),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Numeric Literals", numeric_literals_test_cases)