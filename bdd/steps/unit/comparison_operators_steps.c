#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <sys/stat.h>

// Test scenarios for comparison operators

// Equality operator (==) tests
void test_basic_equality_comparison_with_integers(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let b: i32 = 42;\n"
        "    if a == b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic equality comparison with integers",
                               "equality_basic.asthra",
                               source,
                               NULL,
                               1);
}

void test_equality_comparison_with_different_integer_values(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 20;\n"
        "    if a == b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Equality comparison with different integer values",
                               "equality_different.asthra",
                               source,
                               NULL,
                               0);
}

void test_equality_comparison_with_boolean_values(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = true;\n"
        "    if a == b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Equality comparison with boolean values",
                               "equality_bool.asthra",
                               source,
                               NULL,
                               1);
}

void test_equality_comparison_with_strings(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: string = \"hello\";\n"
        "    let b: string = \"hello\";\n"
        "    if a == b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Equality comparison with strings",
                               "equality_string.asthra",
                               source,
                               NULL,
                               1);
}

void test_equality_comparison_with_floats(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: f32 = 3.14;\n"
        "    let b: f32 = 3.14;\n"
        "    if a == b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Equality comparison with floats",
                               "equality_float.asthra",
                               source,
                               NULL,
                               1);
}

// Inequality operator (!=) tests
void test_basic_inequality_comparison(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 20;\n"
        "    if a != b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic inequality comparison",
                               "inequality_basic.asthra",
                               source,
                               NULL,
                               1);
}

void test_inequality_comparison_with_equal_values(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let b: i32 = 42;\n"
        "    if a != b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Inequality comparison with equal values",
                               "inequality_equal.asthra",
                               source,
                               NULL,
                               0);
}

void test_inequality_comparison_with_booleans(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    if a != b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Inequality comparison with booleans",
                               "inequality_bool.asthra",
                               source,
                               NULL,
                               1);
}

// Less than operator (<) tests
void test_basic_less_than_comparison(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 20;\n"
        "    if a < b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic less than comparison",
                               "less_than_basic.asthra",
                               source,
                               NULL,
                               1);
}

void test_less_than_comparison_with_equal_values(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let b: i32 = 42;\n"
        "    if a < b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Less than comparison with equal values",
                               "less_than_equal.asthra",
                               source,
                               NULL,
                               0);
}

void test_less_than_comparison_with_greater_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 20;\n"
        "    let b: i32 = 10;\n"
        "    if a < b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Less than comparison with greater value",
                               "less_than_greater.asthra",
                               source,
                               NULL,
                               0);
}

void test_less_than_comparison_with_negative_numbers(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = -20;\n"
        "    let b: i32 = -10;\n"
        "    if a < b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Less than comparison with negative numbers",
                               "less_than_negative.asthra",
                               source,
                               NULL,
                               1);
}

// Less than or equal operator (<=) tests
void test_less_than_or_equal_with_less_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 20;\n"
        "    if a <= b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Less than or equal with less value",
                               "less_equal_less.asthra",
                               source,
                               NULL,
                               1);
}

void test_less_than_or_equal_with_equal_values(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let b: i32 = 42;\n"
        "    if a <= b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Less than or equal with equal values",
                               "less_equal_equal.asthra",
                               source,
                               NULL,
                               1);
}

void test_less_than_or_equal_with_greater_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 20;\n"
        "    let b: i32 = 10;\n"
        "    if a <= b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Less than or equal with greater value",
                               "less_equal_greater.asthra",
                               source,
                               NULL,
                               0);
}

// Greater than operator (>) tests
void test_basic_greater_than_comparison(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 20;\n"
        "    let b: i32 = 10;\n"
        "    if a > b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic greater than comparison",
                               "greater_than_basic.asthra",
                               source,
                               NULL,
                               1);
}

void test_greater_than_comparison_with_equal_values(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let b: i32 = 42;\n"
        "    if a > b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Greater than comparison with equal values",
                               "greater_than_equal.asthra",
                               source,
                               NULL,
                               0);
}

void test_greater_than_comparison_with_smaller_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 20;\n"
        "    if a > b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Greater than comparison with smaller value",
                               "greater_than_smaller.asthra",
                               source,
                               NULL,
                               0);
}

// Greater than or equal operator (>=) tests
void test_greater_than_or_equal_with_greater_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 20;\n"
        "    let b: i32 = 10;\n"
        "    if a >= b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Greater than or equal with greater value",
                               "greater_equal_greater.asthra",
                               source,
                               NULL,
                               1);
}

void test_greater_than_or_equal_with_equal_values(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let b: i32 = 42;\n"
        "    if a >= b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Greater than or equal with equal values",
                               "greater_equal_equal.asthra",
                               source,
                               NULL,
                               1);
}

void test_greater_than_or_equal_with_smaller_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 20;\n"
        "    if a >= b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Greater than or equal with smaller value",
                               "greater_equal_smaller.asthra",
                               source,
                               NULL,
                               0);
}

// Chained comparisons
void test_multiple_comparisons_in_one_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 20;\n"
        "    let c: i32 = 30;\n"
        "    if a < b && b < c {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple comparisons in one expression",
                               "chained_comparisons.asthra",
                               source,
                               NULL,
                               1);
}

void test_complex_comparison_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 15;\n"
        "    let y: i32 = 10;\n"
        "    let z: i32 = 20;\n"
        "    if x > y && x < z {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Complex comparison expression",
                               "complex_comparison.asthra",
                               source,
                               NULL,
                               1);
}

// Different numeric types
void test_comparison_with_u8_types(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: u8 = 100;\n"
        "    let b: u8 = 200;\n"
        "    if a < b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison with u8 types",
                               "comparison_u8.asthra",
                               source,
                               NULL,
                               1);
}

void test_comparison_with_i64_types(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i64 = 1000000000000;\n"
        "    let b: i64 = 2000000000000;\n"
        "    if a < b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison with i64 types",
                               "comparison_i64.asthra",
                               source,
                               NULL,
                               1);
}

void test_comparison_with_f64_types(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: f64 = 3.14159265359;\n"
        "    let b: f64 = 2.71828182846;\n"
        "    if a > b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison with f64 types",
                               "comparison_f64.asthra",
                               source,
                               NULL,
                               1);
}

// Edge cases
void test_comparison_with_zero(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 0;\n"
        "    let b: i32 = 0;\n"
        "    if a == b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison with zero",
                               "comparison_zero.asthra",
                               source,
                               NULL,
                               1);
}

void test_comparison_with_maximum_i32_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 2147483647;\n"
        "    let b: i32 = 2147483647;\n"
        "    if a == b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison with maximum i32 value",
                               "comparison_max_i32.asthra",
                               source,
                               NULL,
                               1);
}

void test_comparison_with_minimum_i32_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut a: i32 = -2147483647;\n"
        "    let mut b: i32 = -2147483647;\n"
        "    a = a - 1;  // This gives us the minimum i32 value\n"
        "    b = b - 1;\n"
        "    if a == b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison with minimum i32 value",
                               "comparison_min_i32.asthra",
                               source,
                               NULL,
                               1);
}

// Mixed operators
void test_equality_and_inequality_in_same_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 10;\n"
        "    let c: i32 = 20;\n"
        "    if a == b && b != c {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Equality and inequality in same expression",
                               "mixed_eq_neq.asthra",
                               source,
                               NULL,
                               1);
}

void test_all_comparison_operators_in_complex_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: i32 = 10;\n"
        "    let c: i32 = 10;\n"
        "    let d: i32 = 15;\n"
        "    if a < b && b <= c && c >= b && d > c {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("All comparison operators in complex expression",
                               "all_comparisons.asthra",
                               source,
                               NULL,
                               1);
}

// Use in control flow
void test_comparison_in_nested_if_statements(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 25;\n"
        "    if x > 20 {\n"
        "        if x < 30 {\n"
        "            return 1;\n"
        "        }\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison in nested if statements",
                               "nested_if_comparison.asthra",
                               source,
                               NULL,
                               1);
}

void test_comparison_in_for_loop(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut count: i32 = 0;\n"
        "    for i in range(10) {\n"
        "        if i >= 5 {\n"
        "            count = count + 1;\n"
        "        }\n"
        "    }\n"
        "    return count;\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison in for loop",
                               "comparison_for_loop.asthra",
                               source,
                               NULL,
                               5);
}

// Assignment and comparison
void test_comparison_result_assignment(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 20;\n"
        "    let result: bool = a < b;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison result assignment",
                               "comparison_assignment.asthra",
                               source,
                               NULL,
                               1);
}

void test_multiple_comparison_results(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 15;\n"
        "    let zero: i32 = 0;\n"
        "    let twenty: i32 = 20;\n"
        "    if x > zero {\n"
        "        if x < twenty {\n"
        "            return 1;\n"
        "        }\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple comparison results",
                               "multiple_comparison_results.asthra",
                               source,
                               NULL,
                               1);
}

// Operator precedence
void test_comparison_precedence_with_arithmetic(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: i32 = 3;\n"
        "    if a + b > 7 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison precedence with arithmetic",
                               "comparison_arithmetic_precedence.asthra",
                               source,
                               NULL,
                               1);
}

void test_comparison_precedence_with_parentheses(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 5;\n"
        "    let c: i32 = 3;\n"
        "    if (a > b) == (b > c) {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison precedence with parentheses",
                               "comparison_parentheses.asthra",
                               source,
                               NULL,
                               1);
}

// Function calls in comparisons
void test_comparison_with_function_results(void) {
    const char* source = 
        "package test;\n"
        "pub fn double(x: i32) -> i32 {\n"
        "    return x * 2;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    if double(a) == 10 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comparison with function results",
                               "comparison_function.asthra",
                               source,
                               NULL,
                               1);
}

// Floating point special cases (marked as WIP)
void test_floating_point_nan_comparison(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let zero: f32 = 0.0;\n"
        "    let nan: f32 = zero / zero;\n"
        "    if nan == nan {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;  // NaN is not equal to itself\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Floating point NaN comparison",
                               "float_nan.asthra",
                               source,
                               NULL,
                               1);
}

void test_floating_point_infinity_comparison(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let one: f32 = 1.0;\n"
        "    let zero: f32 = 0.0;\n"
        "    let inf: f32 = one / zero;\n"
        "    let large: f32 = 1000000.0;\n"
        "    if inf > large {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Floating point infinity comparison",
                               "float_infinity.asthra",
                               source,
                               NULL,
                               1);
}

// Define test cases
BddTestCase comparison_operators_test_cases[] = {
    // Equality operator tests
    BDD_WIP_TEST_CASE(basic_equality_comparison_with_integers, test_basic_equality_comparison_with_integers),
    BDD_TEST_CASE(equality_comparison_with_different_integer_values, test_equality_comparison_with_different_integer_values),
    BDD_WIP_TEST_CASE(equality_comparison_with_boolean_values, test_equality_comparison_with_boolean_values),
    BDD_WIP_TEST_CASE(equality_comparison_with_strings, test_equality_comparison_with_strings),
    BDD_WIP_TEST_CASE(equality_comparison_with_floats, test_equality_comparison_with_floats),
    
    // Inequality operator tests
    BDD_WIP_TEST_CASE(basic_inequality_comparison, test_basic_inequality_comparison),
    BDD_TEST_CASE(inequality_comparison_with_equal_values, test_inequality_comparison_with_equal_values),
    BDD_WIP_TEST_CASE(inequality_comparison_with_booleans, test_inequality_comparison_with_booleans),
    
    // Less than operator tests
    BDD_WIP_TEST_CASE(basic_less_than_comparison, test_basic_less_than_comparison),
    BDD_TEST_CASE(less_than_comparison_with_equal_values, test_less_than_comparison_with_equal_values),
    BDD_TEST_CASE(less_than_comparison_with_greater_value, test_less_than_comparison_with_greater_value),
    BDD_WIP_TEST_CASE(less_than_comparison_with_negative_numbers, test_less_than_comparison_with_negative_numbers),
    
    // Less than or equal operator tests
    BDD_WIP_TEST_CASE(less_than_or_equal_with_less_value, test_less_than_or_equal_with_less_value),
    BDD_WIP_TEST_CASE(less_than_or_equal_with_equal_values, test_less_than_or_equal_with_equal_values),
    BDD_TEST_CASE(less_than_or_equal_with_greater_value, test_less_than_or_equal_with_greater_value),
    
    // Greater than operator tests
    BDD_WIP_TEST_CASE(basic_greater_than_comparison, test_basic_greater_than_comparison),
    BDD_TEST_CASE(greater_than_comparison_with_equal_values, test_greater_than_comparison_with_equal_values),
    BDD_TEST_CASE(greater_than_comparison_with_smaller_value, test_greater_than_comparison_with_smaller_value),
    
    // Greater than or equal operator tests
    BDD_WIP_TEST_CASE(greater_than_or_equal_with_greater_value, test_greater_than_or_equal_with_greater_value),
    BDD_WIP_TEST_CASE(greater_than_or_equal_with_equal_values, test_greater_than_or_equal_with_equal_values),
    BDD_TEST_CASE(greater_than_or_equal_with_smaller_value, test_greater_than_or_equal_with_smaller_value),
    
    // Chained comparisons
    BDD_WIP_TEST_CASE(multiple_comparisons_in_one_expression, test_multiple_comparisons_in_one_expression),
    BDD_WIP_TEST_CASE(complex_comparison_expression, test_complex_comparison_expression),
    
    // Different numeric types
    BDD_WIP_TEST_CASE(comparison_with_u8_types, test_comparison_with_u8_types),
    BDD_WIP_TEST_CASE(comparison_with_i64_types, test_comparison_with_i64_types),
    BDD_WIP_TEST_CASE(comparison_with_f64_types, test_comparison_with_f64_types),
    
    // Edge cases
    BDD_WIP_TEST_CASE(comparison_with_zero, test_comparison_with_zero),
    BDD_WIP_TEST_CASE(comparison_with_maximum_i32_value, test_comparison_with_maximum_i32_value),
    BDD_WIP_TEST_CASE(comparison_with_minimum_i32_value, test_comparison_with_minimum_i32_value),
    
    // Mixed operators
    BDD_WIP_TEST_CASE(equality_and_inequality_in_same_expression, test_equality_and_inequality_in_same_expression),
    BDD_WIP_TEST_CASE(all_comparison_operators_in_complex_expression, test_all_comparison_operators_in_complex_expression),
    
    // Use in control flow
    BDD_WIP_TEST_CASE(comparison_in_nested_if_statements, test_comparison_in_nested_if_statements),
    BDD_WIP_TEST_CASE(comparison_in_for_loop, test_comparison_in_for_loop),
    
    // Assignment and comparison
    BDD_WIP_TEST_CASE(comparison_result_assignment, test_comparison_result_assignment),
    BDD_WIP_TEST_CASE(multiple_comparison_results, test_multiple_comparison_results),
    
    // Operator precedence
    BDD_WIP_TEST_CASE(comparison_precedence_with_arithmetic, test_comparison_precedence_with_arithmetic),
    BDD_WIP_TEST_CASE(comparison_precedence_with_parentheses, test_comparison_precedence_with_parentheses),
    
    // Function calls in comparisons
    BDD_WIP_TEST_CASE(comparison_with_function_results, test_comparison_with_function_results),
    
    // Floating point special cases (marked as WIP)
    BDD_WIP_TEST_CASE(floating_point_nan_comparison, test_floating_point_nan_comparison),
    BDD_WIP_TEST_CASE(floating_point_infinity_comparison, test_floating_point_infinity_comparison),
};

// Main test runner
int main(void) {
    return bdd_run_test_suite("Comparison Operators",
                              comparison_operators_test_cases,
                              sizeof(comparison_operators_test_cases) / sizeof(comparison_operators_test_cases[0]),
                              bdd_cleanup_temp_files);
}