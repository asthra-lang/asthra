#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <sys/stat.h>

// Test scenarios for logical operators

// Logical AND tests
void test_basic_logical_and_true(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = true;\n"
        "    let result: bool = a && b;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic logical AND with true values",
                               "logical_and_true.asthra",
                               source,
                               NULL,
                               1);
}

void test_logical_and_with_false(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let result: bool = a && b;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical AND with false values",
                               "logical_and_false.asthra",
                               source,
                               NULL,
                               0);
}

void test_logical_and_all_false(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = false;\n"
        "    let b: bool = false;\n"
        "    let result: bool = a && b;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical AND with all false",
                               "logical_and_all_false.asthra",
                               source,
                               NULL,
                               0);
}

void test_multiple_logical_and(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = true;\n"
        "    let c: bool = true;\n"
        "    let d: bool = false;\n"
        "    let result: bool = a && b && c && d;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple logical AND operations",
                               "multiple_and.asthra",
                               source,
                               NULL,
                               0);
}

// Logical OR tests
void test_basic_logical_or(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let result: bool = a || b;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic logical OR with true values",
                               "logical_or_basic.asthra",
                               source,
                               NULL,
                               1);
}

void test_logical_or_all_false(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = false;\n"
        "    let b: bool = false;\n"
        "    let result: bool = a || b;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical OR with all false",
                               "logical_or_all_false.asthra",
                               source,
                               NULL,
                               0);
}

void test_logical_or_all_true(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = true;\n"
        "    let result: bool = a || b;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical OR with all true",
                               "logical_or_all_true.asthra",
                               source,
                               NULL,
                               1);
}

void test_multiple_logical_or(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = false;\n"
        "    let b: bool = false;\n"
        "    let c: bool = false;\n"
        "    let d: bool = true;\n"
        "    let result: bool = a || b || c || d;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple logical OR operations",
                               "multiple_or.asthra",
                               source,
                               NULL,
                               1);
}

// Logical NOT tests
void test_logical_not_on_true(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let result: bool = !a;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic logical NOT on true",
                               "logical_not_true.asthra",
                               source,
                               NULL,
                               0);
}

void test_logical_not_on_false(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = false;\n"
        "    let result: bool = !a;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical NOT on false",
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
        "    let result: bool = !(!a);\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Double logical NOT",
                               "double_not.asthra",
                               source,
                               NULL,
                               1);
}

// Mixed operators tests
void test_and_precedence_over_or(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    let result: bool = a || b && c;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("AND has higher precedence than OR",
                               "and_precedence.asthra",
                               source,
                               NULL,
                               1);
}

void test_complex_with_parentheses(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    let result: bool = (a || b) && c;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Complex logical expression with parentheses",
                               "complex_parentheses.asthra",
                               source,
                               NULL,
                               1);
}

void test_not_with_and_or(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    let result: bool = !a || b && c;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("NOT with AND and OR",
                               "not_with_and_or.asthra",
                               source,
                               NULL,
                               0);
}

// Logical operators with comparisons
void test_logical_with_comparisons(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let num1: i32 = 5;\n"
        "    let num2: i32 = 10;\n"
        "    let num3: i32 = 15;\n"
        "    let result: bool = (num2 > num1) && (num3 > num2);\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical operators with comparison expressions",
                               "logical_comparisons.asthra",
                               source,
                               NULL,
                               1);
}

void test_complex_comparison_logical(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let val1: i32 = 5;\n"
        "    let val2: i32 = 10;\n"
        "    let result: bool = (val1 > 0) && (10 > val1) || (val2 == 10);\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Complex comparison with logical operators",
                               "complex_comparison.asthra",
                               source,
                               NULL,
                               1);
}

// Short-circuit evaluation tests
void test_and_short_circuit(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 0;\n"
        "    let y: i32 = 10;\n"
        "    // Division by zero would crash if evaluated\n"
        "    if false && (y / x > 0) {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical AND short-circuit evaluation",
                               "and_short_circuit.asthra",
                               source,
                               NULL,
                               1);
}

void test_or_short_circuit(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 0;\n"
        "    let y: i32 = 10;\n"
        "    // Division by zero would crash if evaluated\n"
        "    if true || (y / x > 0) {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical OR short-circuit evaluation",
                               "or_short_circuit.asthra",
                               source,
                               NULL,
                               1);
}

// Control flow tests
void test_logical_in_if_conditions(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 5;\n"
        "    let y: i32 = 10;\n"
        "    \n"
        "    if x > 0 && y > 0 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical operators in if conditions",
                               "logical_if.asthra",
                               source,
                               NULL,
                               1);
}

void test_nested_if_logical(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    \n"
        "    if a || b {\n"
        "        if !b && c {\n"
        "            return 1;\n"
        "        }\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested if with logical operators",
                               "nested_if_logical.asthra",
                               source,
                               NULL,
                               1);
}

// Mutable variable tests
void test_logical_with_mutable(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut flag: bool = false;\n"
        "    flag = true;\n"
        "    \n"
        "    if flag && true {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical operators with mutable boolean",
                               "logical_mutable.asthra",
                               source,
                               NULL,
                               1);
}

void test_complex_mutable_logic(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut a: bool = true;\n"
        "    let mut b: bool = false;\n"
        "    let mut c: bool = true;\n"
        "    \n"
        "    b = a && c;\n"
        "    a = !b || c;\n"
        "    c = a && !b;\n"
        "    \n"
        "    if a && !b && !c {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Complex mutable logic",
                               "complex_mutable.asthra",
                               source,
                               NULL,
                               0);
}

// Edge cases
void test_all_operators_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    let d: bool = false;\n"
        "    \n"
        "    let result: bool = !a || b && c || !d;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("All logical operators in one expression",
                               "all_operators.asthra",
                               source,
                               NULL,
                               1);
}

void test_logical_with_functions(void) {
    const char* source = 
        "package test;\n"
        "pub fn is_positive(x: i32) -> bool {\n"
        "    return x > 0;\n"
        "}\n"
        "\n"
        "pub fn is_even(x: i32) -> bool {\n"
        "    return x % 2 == 0;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let num: i32 = 10;\n"
        "    \n"
        "    if is_positive(num) && is_even(num) {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical operators with function calls",
                               "logical_functions.asthra",
                               source,
                               NULL,
                               1);
}

// Boolean literal tests
void test_direct_boolean_literals(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: bool = true && false || true;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Direct boolean literals in logical expressions",
                               "boolean_literals.asthra",
                               source,
                               NULL,
                               1);
}

void test_not_with_literals(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: bool = !false && !false;\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("NOT operator with boolean literals",
                               "not_literals.asthra",
                               source,
                               NULL,
                               1);
}

// Complex nested expressions
void test_deeply_nested_logical(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    let d: bool = true;\n"
        "    \n"
        "    let result: bool = ((a && b) || (c && d)) && (!(a && !b) || (c || !d));\n"
        "    if result {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Deeply nested logical expressions",
                               "deeply_nested.asthra",
                               source,
                               NULL,
                               1);
}

// Logical operators in loops
void test_logical_in_loops(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut count: i32 = 0;\n"
        "    let mut i: i32 = 0;\n"
        "    \n"
        "    for j in range(10) {\n"
        "        if (j > 2) && (7 > j) {\n"
        "            count = count + 1;\n"
        "        }\n"
        "    }\n"
        "    \n"
        "    return count;\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical operators in for loop condition",
                               "logical_loop.asthra",
                               source,
                               NULL,
                               4);
}

// Assignment patterns
void test_assigning_logical_results(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let val: i32 = 5;\n"
        "    let limit: i32 = 10;\n"
        "    \n"
        "    let is_valid: bool = (val > 0) && (10 > val) && (limit >= 10);\n"
        "    let is_invalid: bool = !is_valid;\n"
        "    \n"
        "    if is_valid && !is_invalid {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Assigning logical expression results",
                               "assign_logical.asthra",
                               source,
                               NULL,
                               1);
}

// Define test cases
BddTestCase logical_operators_test_cases[] = {
    // Basic AND tests
    BDD_TEST_CASE(basic_logical_and_true, test_basic_logical_and_true),
    BDD_TEST_CASE(logical_and_with_false, test_logical_and_with_false),
    BDD_TEST_CASE(logical_and_all_false, test_logical_and_all_false),
    BDD_TEST_CASE(multiple_logical_and, test_multiple_logical_and),
    
    // Basic OR tests
    BDD_TEST_CASE(basic_logical_or, test_basic_logical_or),
    BDD_TEST_CASE(logical_or_all_false, test_logical_or_all_false),
    BDD_TEST_CASE(logical_or_all_true, test_logical_or_all_true),
    BDD_TEST_CASE(multiple_logical_or, test_multiple_logical_or),
    
    // NOT tests
    BDD_TEST_CASE(logical_not_on_true, test_logical_not_on_true),
    BDD_TEST_CASE(logical_not_on_false, test_logical_not_on_false),
    BDD_TEST_CASE(double_logical_not, test_double_logical_not),
    
    // Mixed operators
    BDD_TEST_CASE(and_precedence_over_or, test_and_precedence_over_or),
    BDD_TEST_CASE(complex_with_parentheses, test_complex_with_parentheses),
    BDD_TEST_CASE(not_with_and_or, test_not_with_and_or),
    
    // With comparisons
    BDD_TEST_CASE(logical_with_comparisons, test_logical_with_comparisons),
    BDD_TEST_CASE(complex_comparison_logical, test_complex_comparison_logical),
    
    // Short-circuit evaluation
    BDD_TEST_CASE(and_short_circuit, test_and_short_circuit),
    BDD_TEST_CASE(or_short_circuit, test_or_short_circuit),
    
    // Control flow
    BDD_TEST_CASE(logical_in_if_conditions, test_logical_in_if_conditions),
    BDD_TEST_CASE(nested_if_logical, test_nested_if_logical),
    
    // Mutable variables
    BDD_TEST_CASE(logical_with_mutable, test_logical_with_mutable),
    BDD_TEST_CASE(complex_mutable_logic, test_complex_mutable_logic),
    
    // Edge cases
    BDD_TEST_CASE(all_operators_expression, test_all_operators_expression),
    BDD_TEST_CASE(logical_with_functions, test_logical_with_functions),
    
    // Boolean literals
    BDD_TEST_CASE(direct_boolean_literals, test_direct_boolean_literals),
    BDD_TEST_CASE(not_with_literals, test_not_with_literals),
    
    // Complex nested
    BDD_TEST_CASE(deeply_nested_logical, test_deeply_nested_logical),
    
    // Loops
    BDD_TEST_CASE(logical_in_loops, test_logical_in_loops),
    
    // Assignment patterns
    BDD_TEST_CASE(assigning_logical_results, test_assigning_logical_results),
};

// Main test runner
int main(void) {
    return bdd_run_test_suite("Logical Operators",
                              logical_operators_test_cases,
                              sizeof(logical_operators_test_cases) / sizeof(logical_operators_test_cases[0]),
                              bdd_cleanup_temp_files);
}