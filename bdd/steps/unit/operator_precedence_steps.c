#include "bdd_unit_common.h"
// Test scenarios for operator precedence

void test_logical_or_lowest_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    let result: bool = a && b || c;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical OR has lowest precedence",
                               "logical_or_precedence.asthra",
                               source,
                               NULL,  // No log output expected
                               1);    // Exit code 1 expected
}

void test_logical_and_higher_than_or(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: bool = false || true && false;\n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical AND has higher precedence than OR",
                               "logical_and_precedence.asthra",
                               source,
                               NULL,
                               1);
}

void test_bitwise_or_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 0b1100;\n"
        "    let b: i32 = 0b1010;\n"
        "    let c: i32 = 0b0011;\n"
        "    let result: i32 = a & b | c;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Bitwise OR precedence",
                               "bitwise_or_precedence.asthra",
                               source,
                               NULL,
                               11);  // (12 & 10) | 3 = 8 | 3 = 11
}

void test_bitwise_xor_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 0b1111;\n"
        "    let b: i32 = 0b1010;\n"
        "    let c: i32 = 0b0101;\n"
        "    let result: i32 = a & b ^ c;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Bitwise XOR precedence",
                               "bitwise_xor_precedence.asthra",
                               source,
                               NULL,
                               15);  // (15 & 10) ^ 5 = 10 ^ 5 = 15
}

void test_equality_operators_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: i32 = 3;\n"
        "    let c: i32 = 2;\n"
        "    let result: bool = a > b == c < a;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Equality operators precedence",
                               "equality_precedence.asthra",
                               source,
                               NULL,
                               1);  // (5 > 3) == (2 < 5) = true == true = true
}

void test_relational_over_bitwise(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 8;\n"
        "    let b: i32 = 4;\n"
        "    let c: i32 = 2;\n"
        "    let result: bool = a & b > c;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Relational operators precedence over bitwise",
                               "relational_over_bitwise.asthra",
                               source,
                               NULL,
                               0);  // 8 & (4 > 2) = 8 & 1 = 0
}

void test_shift_operators_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 16;\n"
        "    let b: i32 = 2;\n"
        "    let c: i32 = 1;\n"
        "    let result: i32 = a >> b + c;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Shift operators precedence",
                               "shift_precedence.asthra",
                               source,
                               NULL,
                               2);  // 16 >> (2 + 1) = 16 >> 3 = 2
}

void test_shift_between_relational_arithmetic(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 32;\n"
        "    let b: i32 = 2;\n"
        "    let c: i32 = 3;\n"
        "    let result: bool = a >> b < c;\n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Shift operators between relational and arithmetic",
                               "shift_relational.asthra",
                               source,
                               NULL,
                               1);  // (32 >> 2) < 3 = 8 < 3 = false
}

void test_multiplication_over_addition(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 2;\n"
        "    let b: i32 = 3;\n"
        "    let c: i32 = 4;\n"
        "    let result: i32 = a + b * c;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiplication has higher precedence than addition",
                               "mult_over_add.asthra",
                               source,
                               NULL,
                               14);  // 2 + (3 * 4) = 2 + 12 = 14
}

void test_addition_left_associative(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 5;\n"
        "    let c: i32 = 3;\n"
        "    let result: i32 = a - b + c;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Addition and subtraction are left-associative",
                               "add_left_assoc.asthra",
                               source,
                               NULL,
                               8);  // (10 - 5) + 3 = 5 + 3 = 8
}

void test_division_modulo_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 20;\n"
        "    let b: i32 = 4;\n"
        "    let c: i32 = 3;\n"
        "    let result: i32 = a / b % c;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Division and modulo same precedence as multiplication",
                               "div_mod_precedence.asthra",
                               source,
                               NULL,
                               2);  // (20 / 4) % 3 = 5 % 3 = 2
}

void test_unary_highest_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: i32 = 3;\n"
        "    let result: i32 = -a + b;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Unary operators have highest precedence",
                               "unary_precedence.asthra",
                               source,
                               NULL,
                               254);  // (-5) + 3 = -2, which becomes 254 as unsigned 8-bit exit code
}

void test_logical_not_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let result: bool = !a && b;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical NOT precedence",
                               "logical_not_precedence.asthra",
                               source,
                               NULL,
                               0);  // (!true) && false = false && false = false
}

void test_bitwise_not_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 0b1010;\n"
        "    let b: i32 = 0b1100;\n"
        "    let result: i32 = ~a & b;\n"
        "    return result & 0xFF;\n"
        "}\n";
    
    bdd_run_execution_scenario("Bitwise NOT precedence",
                               "bitwise_not_precedence.asthra",
                               source,
                               NULL,
                               4);  // (~10) & 12 = ...11110101 & 12 = 4 (after masking)
}

void test_complex_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 5;\n"
        "    let c: i32 = 2;\n"
        "    let d: i32 = 3;\n"
        "    let result: bool = a + b * c > d << 2 && a - b < c + d;\n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Complex expression with multiple operators",
                               "complex_expression.asthra",
                               source,
                               NULL,
                               1);  // ((10 + (5 * 2)) > (3 << 2)) && ((10 - 5) < (2 + 3))
                                    // = (20 > 12) && (5 < 5) = true && false = false
}

void test_parentheses_override(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 2;\n"
        "    let b: i32 = 3;\n"
        "    let c: i32 = 4;\n"
        "    let result1: i32 = a + b * c;\n"
        "    let result2: i32 = (a + b) * c;\n"
        "    return result2 - result1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Parentheses override precedence",
                               "parentheses_override.asthra",
                               source,
                               NULL,
                               6);  // ((2 + 3) * 4) - (2 + (3 * 4)) = 20 - 14 = 6
}

void test_function_call_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn square(x: i32) -> i32 {\n"
        "    return x * x;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 3;\n"
        "    let result: i32 = square(a) + 1;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Function call precedence",
                               "function_call_precedence.asthra",
                               source,
                               NULL,
                               10);  // square(3) + 1 = 9 + 1 = 10
}

void test_field_access_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let p: Point = Point { x: 5, y: 3 };\n"
        "    let result: i32 = p.x + p.y * 2;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Field access precedence",
                               "field_access_precedence.asthra",
                               source,
                               NULL,
                               11);  // 5 + (3 * 2) = 5 + 6 = 11
}

void test_array_indexing_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: [3]i32 = [10, 20, 30];\n"
        "    let i: i32 = 1;\n"
        "    let result: i32 = arr[i] + arr[i + 1];\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Array indexing precedence",
                               "array_indexing_precedence.asthra",
                               source,
                               NULL,
                               50);  // arr[1] + arr[2] = 20 + 30 = 50
}

void test_logical_bitwise_mixed(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 0b1100;\n"
        "    let b: i32 = 0b1010;\n"
        "    let c: bool = true;\n"
        "    let result: bool = a & b != 0 && c;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical operators have lower precedence than bitwise",
                               "logical_bitwise_mixed.asthra",
                               source,
                               NULL,
                               1);  // ((12 & 10) != 0) && true = (8 != 0) && true = true
}

void test_sizeof_precedence(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let size: usize = sizeof(i32) * 2;\n"
        "    return size as i32;\n"
        "}\n";
    
    bdd_run_execution_scenario("Sizeof operator precedence",
                               "sizeof_precedence.asthra",
                               source,
                               NULL,
                               8);  // sizeof(i32) * 2 = 4 * 2 = 8
}

void test_full_precedence_hierarchy(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 100;\n"
        "    let b: i32 = 10;\n"
        "    let c: i32 = 5;\n"
        "    let d: i32 = 2;\n"
        "    let e: bool = true;\n"
        "    let f: bool = false;\n"
        "    \n"
        "    let result: bool = a / b - c << d == 20 >> 1 && e || !f;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Full precedence hierarchy validation",
                               "full_precedence.asthra",
                               source,
                               NULL,
                               1);  // Complex expression evaluates to true
}

void test_chained_unary_operators(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let result: i32 = -~a;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Chained unary operators are left-to-right",
                               "chained_unary.asthra",
                               source,
                               NULL,
                               6);  // -(~5) = -(-6) = 6
}

void test_left_associative_chain(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 100;\n"
        "    let b: i32 = 50;\n"
        "    let c: i32 = 20;\n"
        "    let d: i32 = 10;\n"
        "    let result: i32 = a - b - c - d;\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Left-associative operators chain correctly",
                               "left_associative.asthra",
                               source,
                               NULL,
                               20);  // ((100 - 50) - 20) - 10 = 20
}

// Define test cases - mark failing tests as WIP
BDD_DECLARE_TEST_CASES(operator_precedence)
    BDD_TEST_CASE(logical_or_lowest_precedence, test_logical_or_lowest_precedence),
    BDD_TEST_CASE(logical_and_higher_than_or, test_logical_and_higher_than_or),
    BDD_TEST_CASE(bitwise_or_precedence, test_bitwise_or_precedence),
    BDD_TEST_CASE(bitwise_xor_precedence, test_bitwise_xor_precedence),
    BDD_WIP_TEST_CASE(equality_operators_precedence, test_equality_operators_precedence),  // Type checking issue
    BDD_WIP_TEST_CASE(relational_over_bitwise, test_relational_over_bitwise),  // Type checking issue
    BDD_TEST_CASE(shift_operators_precedence, test_shift_operators_precedence),
    BDD_TEST_CASE(shift_between_relational_arithmetic, test_shift_between_relational_arithmetic),
    BDD_TEST_CASE(multiplication_over_addition, test_multiplication_over_addition),
    BDD_TEST_CASE(addition_left_associative, test_addition_left_associative),
    BDD_TEST_CASE(division_modulo_precedence, test_division_modulo_precedence),
    BDD_TEST_CASE(unary_highest_precedence, test_unary_highest_precedence),
    BDD_TEST_CASE(logical_not_precedence, test_logical_not_precedence),
    BDD_TEST_CASE(bitwise_not_precedence, test_bitwise_not_precedence),
    BDD_TEST_CASE(complex_expression, test_complex_expression),
    BDD_TEST_CASE(parentheses_override, test_parentheses_override),
    BDD_TEST_CASE(function_call_precedence, test_function_call_precedence),
    BDD_WIP_TEST_CASE(field_access_precedence, test_field_access_precedence),  // Segfault
    BDD_WIP_TEST_CASE(array_indexing_precedence, test_array_indexing_precedence),  // Segfault
    BDD_WIP_TEST_CASE(logical_bitwise_mixed, test_logical_bitwise_mixed),  // Type checking issue
    BDD_WIP_TEST_CASE(sizeof_precedence, test_sizeof_precedence),  // sizeof not implemented
    BDD_TEST_CASE(full_precedence_hierarchy, test_full_precedence_hierarchy),
    BDD_WIP_TEST_CASE(chained_unary_operators, test_chained_unary_operators),  // Unary chain issue
    BDD_TEST_CASE(left_associative_chain, test_left_associative_chain),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Operator Precedence", operator_precedence_test_cases)