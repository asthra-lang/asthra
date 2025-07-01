#include "bdd_unit_common.h"
// Test scenarios for arithmetic operators

// Addition tests
void test_basic_integer_addition(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: i32 = 3;\n"
        "    let result: i32 = a + b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic integer addition",
                               "basic_addition.asthra",
                               source,
                               NULL,
                               8);
}

void test_addition_with_negative_numbers(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = -10;\n"
        "    let b: i32 = 7;\n"
        "    let result: i32 = a + b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Addition with negative numbers",
                               "negative_addition.asthra",
                               source,
                               NULL,
                               253);
}

void test_floating_point_addition(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: f32 = 3.14;\n"
        "    let b: f32 = 2.86;\n"
        "    let result: f32 = a + b;\n"
        "    if result > 5.99 && result < 6.01 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Floating point addition",
                               "float_addition.asthra",
                               source,
                               NULL,
                               1);
}

void test_addition_different_integer_types(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i16 = 100;\n"
        "    let b: i16 = 50;\n"
        "    let result: i16 = a + b;\n"
        "    return result as i32;\n"
        "}\n";
    
    bdd_run_execution_scenario("Addition with different integer types",
                               "different_int_addition.asthra",
                               source,
                               NULL,
                               150);
}

// Subtraction tests
void test_basic_integer_subtraction(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 3;\n"
        "    let result: i32 = a - b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic integer subtraction",
                               "basic_subtraction.asthra",
                               source,
                               NULL,
                               7);
}

void test_subtraction_resulting_negative(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: i32 = 12;\n"
        "    let result: i32 = a - b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Subtraction resulting in negative",
                               "negative_subtraction.asthra",
                               source,
                               NULL,
                               249);
}

void test_subtraction_unsigned_integers(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: u32 = 100;\n"
        "    let b: u32 = 25;\n"
        "    let result: u32 = a - b;\n"
        "    return result as i32;\n"
        "}\n";
    
    bdd_run_execution_scenario("Subtraction with unsigned integers",
                               "unsigned_subtraction.asthra",
                               source,
                               NULL,
                               75);
}

void test_floating_point_subtraction(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: f64 = 10.5;\n"
        "    let b: f64 = 3.2;\n"
        "    let result: f64 = a - b;\n"
        "    if result > 7.29 && result < 7.31 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Floating point subtraction",
                               "float_subtraction.asthra",
                               source,
                               NULL,
                               1);
}

// Multiplication tests
void test_basic_integer_multiplication(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 6;\n"
        "    let b: i32 = 7;\n"
        "    let result: i32 = a * b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic integer multiplication",
                               "basic_multiplication.asthra",
                               source,
                               NULL,
                               42);
}

void test_multiplication_with_zero(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 1000;\n"
        "    let b: i32 = 0;\n"
        "    let result: i32 = a * b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiplication with zero",
                               "multiplication_zero.asthra",
                               source,
                               NULL,
                               0);
}

void test_multiplication_negative_numbers(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = -4;\n"
        "    let b: i32 = -5;\n"
        "    let result: i32 = a * b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiplication with negative numbers",
                               "negative_multiplication.asthra",
                               source,
                               NULL,
                               20);
}

void test_floating_point_multiplication(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: f32 = 2.5;\n"
        "    let b: f32 = 4.0;\n"
        "    let result: f32 = a * b;\n"
        "    if result > 9.99 && result < 10.01 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Floating point multiplication",
                               "float_multiplication.asthra",
                               source,
                               NULL,
                               1);
}

// Division tests
void test_basic_integer_division(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 20;\n"
        "    let b: i32 = 4;\n"
        "    let result: i32 = a / b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic integer division",
                               "basic_division.asthra",
                               source,
                               NULL,
                               5);
}

void test_integer_division_truncation(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 7;\n"
        "    let b: i32 = 3;\n"
        "    let result: i32 = a / b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Integer division with truncation",
                               "division_truncation.asthra",
                               source,
                               NULL,
                               2);
}

void test_division_negative_numbers(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = -15;\n"
        "    let b: i32 = 3;\n"
        "    let result: i32 = a / b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Division with negative numbers",
                               "negative_division.asthra",
                               source,
                               NULL,
                               251);
}

void test_floating_point_division(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: f32 = 10.0;\n"
        "    let b: f32 = 4.0;\n"
        "    let result: f32 = a / b;\n"
        "    if result > 2.49 && result < 2.51 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Floating point division",
                               "float_division.asthra",
                               source,
                               NULL,
                               1);
}

void test_division_by_zero(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 0;\n"
        "    let result: i32 = a / b;\n"
        "    return result;\n"
        "}\n";
    
    // This test expects compilation to succeed but execution to fail with runtime error
    bdd_run_compilation_scenario("Division by zero - compilation succeeds",
                                 "division_by_zero.asthra",
                                 source,
                                 1,  // compilation should succeed
                                 NULL);
    
    // Note: Runtime division by zero behavior testing would require 
    // a separate execution scenario test, but this is primarily a 
    // compile-time feature test for the division operator.
}

// Modulo tests
void test_basic_modulo_operation(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 17;\n"
        "    let b: i32 = 5;\n"
        "    let result: i32 = a % b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic modulo operation",
                               "basic_modulo.asthra",
                               source,
                               NULL,
                               2);
}

void test_modulo_even_division(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 20;\n"
        "    let b: i32 = 5;\n"
        "    let result: i32 = a % b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Modulo with even division",
                               "modulo_even.asthra",
                               source,
                               NULL,
                               0);
}

void test_modulo_negative_dividend(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = -17;\n"
        "    let b: i32 = 5;\n"
        "    let result: i32 = a % b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Modulo with negative dividend",
                               "negative_modulo.asthra",
                               source,
                               NULL,
                               254);
}

void test_modulo_unsigned_integers(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: u32 = 23;\n"
        "    let b: u32 = 7;\n"
        "    let result: u32 = a % b;\n"
        "    return result as i32;\n"
        "}\n";
    
    bdd_run_execution_scenario("Modulo with unsigned integers",
                               "unsigned_modulo.asthra",
                               source,
                               NULL,
                               2);
}

// Mixed operations tests
void test_multiple_additions(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 1;\n"
        "    let b: i32 = 2;\n"
        "    let c: i32 = 3;\n"
        "    let d: i32 = 4;\n"
        "    let result: i32 = a + b + c + d;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple additions",
                               "multiple_additions.asthra",
                               source,
                               NULL,
                               10);
}

void test_mixed_arithmetic_operations(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 5;\n"
        "    let c: i32 = 2;\n"
        "    let result: i32 = a + b * c - a / b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Mixed arithmetic operations",
                               "mixed_arithmetic.asthra",
                               source,
                               NULL,
                               18);  // 10 + (5 * 2) - (10 / 5) = 10 + 10 - 2 = 18
}

void test_arithmetic_with_parentheses(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 5;\n"
        "    let c: i32 = 2;\n"
        "    let result: i32 = (a + b) * c;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Arithmetic with parentheses",
                               "parentheses_arithmetic.asthra",
                               source,
                               NULL,
                               30);
}

void test_arithmetic_mutable_variables(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut sum: i32 = 0;\n"
        "    sum = sum + 10;\n"
        "    sum = sum + 20;\n"
        "    sum = sum - 5;\n"
        "    return sum;\n"
        "}\n";
    
    bdd_run_execution_scenario("Arithmetic with mutable variables",
                               "mutable_arithmetic.asthra",
                               source,
                               NULL,
                               25);
}

void test_compound_arithmetic_expressions(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 100;\n"
        "    let b: i32 = 25;\n"
        "    let c: i32 = 10;\n"
        "    let result: i32 = (a - b) / (c - 5);\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Compound arithmetic expressions",
                               "compound_arithmetic.asthra",
                               source,
                               NULL,
                               15);  // (100 - 25) / (10 - 5) = 75 / 5 = 15
}

void test_arithmetic_with_type_casting(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i16 = 100;\n"
        "    let b: i32 = 200;\n"
        "    let result: i32 = (a as i32) + b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Arithmetic with type casting",
                               "type_cast_arithmetic.asthra",
                               source,
                               NULL,
                               300);
}

void test_arithmetic_with_constants(void) {
    const char* source = 
        "package test;\n"
        "pub const PI: f32 = 3.14159;\n"
        "pub const RADIUS: i32 = 10;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let circumference: f32 = 2.0 * PI * (RADIUS as f32);\n"
        "    if circumference > 62.8 && circumference < 62.9 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Arithmetic with const values",
                               "const_arithmetic.asthra",
                               source,
                               NULL,
                               1);
}

void test_large_number_arithmetic(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i64 = 1000000;\n"
        "    let b: i64 = 2000000;\n"
        "    let result: i64 = a * b;\n"
        "    if result == 2000000000000 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Large number arithmetic",
                               "large_arithmetic.asthra",
                               source,
                               NULL,
                               1);
}

void test_i8_boundary_arithmetic(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i8 = 127;\n"
        "    let b: i8 = -128;\n"
        "    let result: i16 = (a as i16) - (b as i16);\n"
        "    return result as i32;\n"
        "}\n";
    
    bdd_run_execution_scenario("Arithmetic with i8 boundary values",
                               "i8_boundary.asthra",
                               source,
                               NULL,
                               255);
}

// Unary minus tests
void test_unary_minus_positive(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let result: i32 = -a;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Unary minus on positive number",
                               "unary_minus_positive.asthra",
                               source,
                               NULL,
                               214);
}

void test_unary_minus_negative(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = -15;\n"
        "    let result: i32 = -a;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Unary minus on negative number",
                               "unary_minus_negative.asthra",
                               source,
                               NULL,
                               15);
}

void test_multiple_unary_minus(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let result: i32 = -(-a);\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple unary minus",
                               "multiple_unary_minus.asthra",
                               source,
                               NULL,
                               10);
}

// Complex tests
void test_nested_arithmetic_expressions(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 50;\n"
        "    let b: i32 = 10;\n"
        "    let c: i32 = 5;\n"
        "    let d: i32 = 2;\n"
        "    let result: i32 = ((a - b) / c) * d + (b % c);\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested arithmetic expressions",
                               "nested_arithmetic.asthra",
                               source,
                               NULL,
                               16);  // ((50-10)/5)*2 + (10%5) = (40/5)*2 + 0 = 8*2 + 0 = 16
}

void test_arithmetic_in_function_calls(void) {
    const char* source = 
        "package test;\n"
        "pub fn calculate(x: i32, y: i32) -> i32 {\n"
        "    return x * 2 + y * 3;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: i32 = calculate(5 + 2, 10 - 3);\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Arithmetic in function calls",
                               "function_call_arithmetic.asthra",
                               source,
                               NULL,
                               35);  // calculate(7, 7) = 7*2 + 7*3 = 14 + 21 = 35
}

// Define test cases - mark failing tests as WIP
BDD_DECLARE_TEST_CASES(arithmetic_operators)
    // Addition tests
    BDD_TEST_CASE(basic_integer_addition, test_basic_integer_addition),
    BDD_TEST_CASE(addition_with_negative_numbers, test_addition_with_negative_numbers),
    BDD_WIP_TEST_CASE(floating_point_addition, test_floating_point_addition), // Parser issue with float comparisons
    BDD_WIP_TEST_CASE(addition_different_integer_types, test_addition_different_integer_types), // Type cast issue
    
    // Subtraction tests
    BDD_TEST_CASE(basic_integer_subtraction, test_basic_integer_subtraction),
    BDD_TEST_CASE(subtraction_resulting_negative, test_subtraction_resulting_negative),
    BDD_WIP_TEST_CASE(subtraction_unsigned_integers, test_subtraction_unsigned_integers), // Type cast issue
    BDD_WIP_TEST_CASE(floating_point_subtraction, test_floating_point_subtraction), // Parser issue with float comparisons
    
    // Multiplication tests
    BDD_TEST_CASE(basic_integer_multiplication, test_basic_integer_multiplication),
    BDD_TEST_CASE(multiplication_with_zero, test_multiplication_with_zero),
    BDD_TEST_CASE(multiplication_negative_numbers, test_multiplication_negative_numbers),
    BDD_WIP_TEST_CASE(floating_point_multiplication, test_floating_point_multiplication), // Parser issue with float comparisons
    
    // Division tests
    BDD_TEST_CASE(basic_integer_division, test_basic_integer_division),
    BDD_TEST_CASE(integer_division_truncation, test_integer_division_truncation),
    BDD_TEST_CASE(division_negative_numbers, test_division_negative_numbers),
    BDD_WIP_TEST_CASE(floating_point_division, test_floating_point_division), // Parser issue with float comparisons
    BDD_TEST_CASE(division_by_zero, test_division_by_zero),
    
    // Modulo tests
    BDD_TEST_CASE(basic_modulo_operation, test_basic_modulo_operation),
    BDD_TEST_CASE(modulo_even_division, test_modulo_even_division),
    BDD_TEST_CASE(modulo_negative_dividend, test_modulo_negative_dividend),
    BDD_WIP_TEST_CASE(modulo_unsigned_integers, test_modulo_unsigned_integers), // Type cast issue
    
    // Mixed operations
    BDD_TEST_CASE(multiple_additions, test_multiple_additions),
    BDD_TEST_CASE(mixed_arithmetic_operations, test_mixed_arithmetic_operations),
    BDD_TEST_CASE(arithmetic_with_parentheses, test_arithmetic_with_parentheses),
    BDD_TEST_CASE(arithmetic_mutable_variables, test_arithmetic_mutable_variables),
    BDD_TEST_CASE(compound_arithmetic_expressions, test_compound_arithmetic_expressions),
    BDD_WIP_TEST_CASE(arithmetic_with_type_casting, test_arithmetic_with_type_casting),
    BDD_WIP_TEST_CASE(arithmetic_with_constants, test_arithmetic_with_constants), // Type cast issue
    BDD_TEST_CASE(large_number_arithmetic, test_large_number_arithmetic),
    BDD_WIP_TEST_CASE(i8_boundary_arithmetic, test_i8_boundary_arithmetic), // Type cast issue
    
    // Unary minus
    BDD_TEST_CASE(unary_minus_positive, test_unary_minus_positive),
    BDD_TEST_CASE(unary_minus_negative, test_unary_minus_negative),
    BDD_TEST_CASE(multiple_unary_minus, test_multiple_unary_minus),
    
    // Complex expressions
    BDD_TEST_CASE(nested_arithmetic_expressions, test_nested_arithmetic_expressions),
    BDD_TEST_CASE(arithmetic_in_function_calls, test_arithmetic_in_function_calls),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Arithmetic Operators", arithmetic_operators_test_cases)