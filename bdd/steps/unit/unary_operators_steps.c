#include "bdd_unit_common.h"
// Test scenarios for unary operators

// Logical NOT operator (!) tests
void test_basic_logical_not_with_true_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    if !a {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic logical NOT with true value",
                               "logical_not_true.asthra",
                               source,
                               NULL,
                               1);
}

void test_basic_logical_not_with_false_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = false;\n"
        "    if !a {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic logical NOT with false value",
                               "logical_not_false.asthra",
                               source,
                               NULL,
                               1);
}

void test_double_logical_not(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = !a;\n"
        "    let c: bool = !b;\n"
        "    if c {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Double logical NOT",
                               "double_logical_not.asthra",
                               source,
                               NULL,
                               1);
}

void test_logical_not_in_complex_expressions(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    if !a || !b {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical NOT in complex expressions",
                               "logical_not_complex.asthra",
                               source,
                               NULL,
                               1);
}

// Arithmetic negation operator (-) tests
void test_basic_arithmetic_negation_with_positive_integer(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let b: i32 = -a;\n"
        "    return b;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic arithmetic negation with positive integer",
                               "arithmetic_neg_positive.asthra",
                               source,
                               NULL,
                               -42);
}

void test_basic_arithmetic_negation_with_negative_integer(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = -42;\n"
        "    let b: i32 = -a;\n"
        "    return b;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic arithmetic negation with negative integer",
                               "arithmetic_neg_negative.asthra",
                               source,
                               NULL,
                               42);
}

void test_arithmetic_negation_with_zero(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 0;\n"
        "    let b: i32 = -a;\n"
        "    return b;\n"
        "}\n";
    
    bdd_run_execution_scenario("Arithmetic negation with zero",
                               "arithmetic_neg_zero.asthra",
                               source,
                               NULL,
                               0);
}

void test_arithmetic_negation_with_different_integer_types(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i8 = 10;\n"
        "    let b: i8 = -a;\n"
        "    if b == -10 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Arithmetic negation with different integer types",
                               "arithmetic_neg_i8.asthra",
                               source,
                               NULL,
                               1);
}

void test_arithmetic_negation_with_floating_point(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: f32 = 3.14;\n"
        "    let b: f32 = -a;\n"
        "    if b < 0.0 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Arithmetic negation with floating point",
                               "arithmetic_neg_float.asthra",
                               source,
                               NULL,
                               1);
}

// Bitwise complement operator (~) tests
void test_basic_bitwise_complement(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 0;\n"
        "    let b: i32 = ~a;\n"
        "    if b == -1 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic bitwise complement",
                               "bitwise_complement_basic.asthra",
                               source,
                               NULL,
                               1);
}

void test_bitwise_complement_of_minus_one(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = -1;\n"
        "    let b: i32 = ~a;\n"
        "    return b;\n"
        "}\n";
    
    bdd_run_execution_scenario("Bitwise complement of -1",
                               "bitwise_complement_minus_one.asthra",
                               source,
                               NULL,
                               0);
}

void test_bitwise_complement_with_u8(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: u8 = 0;\n"
        "    let b: u8 = ~a;\n"
        "    if b == 255 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bitwise complement with u8",
                               "bitwise_complement_u8.asthra",
                               source,
                               NULL,
                               1);
}

void test_bitwise_complement_in_expressions(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: i32 = ~a & 0xFF;\n"
        "    if b == 250 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bitwise complement in expressions",
                               "bitwise_complement_expr.asthra",
                               source,
                               NULL,
                               1);
}

// Pointer dereference operator (*) tests
void test_basic_pointer_dereference(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let p: *const i32 = &a;\n"
        "    unsafe {\n"
        "        let b: i32 = *p;\n"
        "        return b;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic pointer dereference",
                               "pointer_deref_basic.asthra",
                               source,
                               NULL,
                               42);
}

void test_mutable_pointer_dereference(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut a: i32 = 10;\n"
        "    let p: *mut i32 = &a;\n"
        "    unsafe {\n"
        "        *p = 20;\n"
        "    }\n"
        "    return a;\n"
        "}\n";
    
    bdd_run_execution_scenario("Mutable pointer dereference",
                               "pointer_deref_mut.asthra",
                               source,
                               NULL,
                               20);
}

void test_pointer_dereference_in_expressions(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 5;\n"
        "    let pa: *const i32 = &a;\n"
        "    let pb: *const i32 = &b;\n"
        "    unsafe {\n"
        "        let sum: i32 = *pa + *pb;\n"
        "        return sum;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Pointer dereference in expressions",
                               "pointer_deref_expr.asthra",
                               source,
                               NULL,
                               15);
}

// Address-of operator (&) tests
void test_basic_address_of_operator(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let p: *const i32 = &a;\n"
        "    unsafe {\n"
        "        if *p == 42 {\n"
        "            return 1;\n"
        "        } else {\n"
        "            return 0;\n"
        "        }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic address-of operator",
                               "address_of_basic.asthra",
                               source,
                               NULL,
                               1);
}

void test_address_of_mutable_variable(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut a: i32 = 10;\n"
        "    let p: *mut i32 = &a;\n"
        "    unsafe {\n"
        "        *p = 20;\n"
        "        if a == 20 {\n"
        "            return 1;\n"
        "        } else {\n"
        "            return 0;\n"
        "        }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Address-of mutable variable",
                               "address_of_mut.asthra",
                               source,
                               NULL,
                               1);
}

void test_address_of_in_function_calls(void) {
    const char* source = 
        "package test;\n"
        "pub fn modify_value(p: *mut i32) -> void {\n"
        "    unsafe {\n"
        "        *p = *p + 10;\n"
        "    }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut a: i32 = 5;\n"
        "    modify_value(&a);\n"
        "    return a;\n"
        "}\n";
    
    bdd_run_execution_scenario("Address-of in function calls",
                               "address_of_function.asthra",
                               source,
                               NULL,
                               15);
}

// Combined unary operators tests
void test_combining_arithmetic_negation_and_logical_not(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: bool = !(-a < 0);\n"
        "    if b {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Combining arithmetic negation and logical NOT",
                               "combined_neg_not.asthra",
                               source,
                               NULL,
                               1);
}

void test_combining_bitwise_complement_and_arithmetic_negation(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: i32 = -~a;\n"
        "    return b;\n"
        "}\n";
    
    bdd_run_execution_scenario("Combining bitwise complement and arithmetic negation",
                               "combined_complement_neg.asthra",
                               source,
                               NULL,
                               6);
}

void test_pointer_operations_with_arithmetic(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let p: *const i32 = &a;\n"
        "    unsafe {\n"
        "        let b: i32 = -*p;\n"
        "        return b;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Pointer operations with arithmetic",
                               "pointer_with_arithmetic.asthra",
                               source,
                               NULL,
                               -10);
}

// Edge cases
void test_negation_of_minimum_i32_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut a: i32 = -2147483647;\n"
        "    a = a - 1;  // This gives us the minimum i32 value\n"
        "    let b: i32 = -a;\n"
        "    if b == a {  // Overflow: negating minimum gives minimum\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Negation of minimum i32 value",
                               "neg_min_i32.asthra",
                               source,
                               NULL,
                               1);
}

void test_multiple_dereferences(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 42;\n"
        "    let p1: *const i32 = &a;\n"
        "    let p2: *const *const i32 = &p1;\n"
        "    unsafe {\n"
        "        let b: i32 = **p2;\n"
        "        return b;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple dereferences",
                               "multiple_deref.asthra",
                               source,
                               NULL,
                               42);
}

// Operator precedence tests
void test_unary_operator_precedence_with_multiplication(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: i32 = -a * 2;  // Should be (-5) * 2 = -10\n"
        "    return b;\n"
        "}\n";
    
    bdd_run_execution_scenario("Unary operator precedence with multiplication",
                               "unary_precedence_mult.asthra",
                               source,
                               NULL,
                               -10);
}

void test_unary_operator_precedence_with_addition(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 5;\n"
        "    let b: i32 = -a + 3;  // Should be (-5) + 3 = -2\n"
        "    if b == -2 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Unary operator precedence with addition",
                               "unary_precedence_add.asthra",
                               source,
                               NULL,
                               1);
}

// Type-specific tests
void test_logical_not_with_comparison_result(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 10;\n"
        "    let b: i32 = 20;\n"
        "    let result: bool = !(a > b);\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical NOT with comparison result",
                               "logical_not_comparison.asthra",
                               source,
                               NULL,
                               1);
}

void test_bitwise_complement_with_different_widths(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: u16 = 0;\n"
        "    let b: u16 = ~a;\n"
        "    if b == 65535 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bitwise complement with different widths",
                               "bitwise_complement_u16.asthra",
                               source,
                               NULL,
                               1);
}

// Error prevention tests
void test_single_unary_operator_per_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = !a;  // Grammar prevents !!a\n"
        "    let c: bool = !b;  // Must use separate statement\n"
        "    if c {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Single unary operator per expression",
                               "single_unary_op.asthra",
                               source,
                               NULL,
                               1);
}

void test_pointer_operations_require_unsafe_blocks(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 100;\n"
        "    let p: *const i32 = &a;\n"
        "    let result: i32 = 0;\n"
        "    unsafe {\n"
        "        result = *p;\n"
        "    }\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Pointer operations require unsafe blocks",
                               "pointer_unsafe_required.asthra",
                               source,
                               NULL,
                               100);
}

// Define test cases
BDD_DECLARE_TEST_CASES(unary_operators)
    // Logical NOT operator tests
    BDD_WIP_TEST_CASE(basic_logical_not_with_true_value, test_basic_logical_not_with_true_value),
    BDD_WIP_TEST_CASE(basic_logical_not_with_false_value, test_basic_logical_not_with_false_value),
    BDD_WIP_TEST_CASE(double_logical_not, test_double_logical_not),
    BDD_WIP_TEST_CASE(logical_not_in_complex_expressions, test_logical_not_in_complex_expressions),
    
    // Arithmetic negation operator tests
    BDD_WIP_TEST_CASE(basic_arithmetic_negation_with_positive_integer, test_basic_arithmetic_negation_with_positive_integer),
    BDD_WIP_TEST_CASE(basic_arithmetic_negation_with_negative_integer, test_basic_arithmetic_negation_with_negative_integer),
    BDD_TEST_CASE(arithmetic_negation_with_zero, test_arithmetic_negation_with_zero),
    BDD_WIP_TEST_CASE(arithmetic_negation_with_different_integer_types, test_arithmetic_negation_with_different_integer_types),
    BDD_WIP_TEST_CASE(arithmetic_negation_with_floating_point, test_arithmetic_negation_with_floating_point),
    
    // Bitwise complement operator tests
    BDD_WIP_TEST_CASE(basic_bitwise_complement, test_basic_bitwise_complement),
    BDD_TEST_CASE(bitwise_complement_of_minus_one, test_bitwise_complement_of_minus_one),
    BDD_WIP_TEST_CASE(bitwise_complement_with_u8, test_bitwise_complement_with_u8),
    BDD_WIP_TEST_CASE(bitwise_complement_in_expressions, test_bitwise_complement_in_expressions),
    
    // Pointer dereference operator tests
    BDD_WIP_TEST_CASE(basic_pointer_dereference, test_basic_pointer_dereference),
    BDD_WIP_TEST_CASE(mutable_pointer_dereference, test_mutable_pointer_dereference),
    BDD_WIP_TEST_CASE(pointer_dereference_in_expressions, test_pointer_dereference_in_expressions),
    
    // Address-of operator tests
    BDD_WIP_TEST_CASE(basic_address_of_operator, test_basic_address_of_operator),
    BDD_WIP_TEST_CASE(address_of_mutable_variable, test_address_of_mutable_variable),
    BDD_WIP_TEST_CASE(address_of_in_function_calls, test_address_of_in_function_calls),
    
    // Combined unary operators tests
    BDD_WIP_TEST_CASE(combining_arithmetic_negation_and_logical_not, test_combining_arithmetic_negation_and_logical_not),
    BDD_WIP_TEST_CASE(combining_bitwise_complement_and_arithmetic_negation, test_combining_bitwise_complement_and_arithmetic_negation),
    BDD_WIP_TEST_CASE(pointer_operations_with_arithmetic, test_pointer_operations_with_arithmetic),
    
    // Edge cases
    BDD_WIP_TEST_CASE(negation_of_minimum_i32_value, test_negation_of_minimum_i32_value),
    BDD_WIP_TEST_CASE(multiple_dereferences, test_multiple_dereferences),
    
    // Operator precedence tests
    BDD_WIP_TEST_CASE(unary_operator_precedence_with_multiplication, test_unary_operator_precedence_with_multiplication),
    BDD_WIP_TEST_CASE(unary_operator_precedence_with_addition, test_unary_operator_precedence_with_addition),
    
    // Type-specific tests
    BDD_WIP_TEST_CASE(logical_not_with_comparison_result, test_logical_not_with_comparison_result),
    BDD_WIP_TEST_CASE(bitwise_complement_with_different_widths, test_bitwise_complement_with_different_widths),
    
    // Error prevention tests
    BDD_WIP_TEST_CASE(single_unary_operator_per_expression, test_single_unary_operator_per_expression),
    BDD_WIP_TEST_CASE(pointer_operations_require_unsafe_blocks, test_pointer_operations_require_unsafe_blocks),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Unary Operators", unary_operators_test_cases)