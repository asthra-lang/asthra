#include "bdd_unit_common.h"

// Test scenarios using the new reusable framework

void test_boolean_literals(void) {
    const char* source = 
        BDD_CONDITIONAL_TEST(
            "let t: bool = true; let f: bool = false;",
            "t && !f",
            "return 0;",
            "return 1;"
        );
    
    bdd_run_execution_scenario("Boolean literals",
                               "bool_literals.asthra",
                               source,
                               NULL,
                               0);
}

void test_logical_not(void) {
    const char* source = 
        BDD_CONDITIONAL_TEST(
            "let val: bool = true; let negated: bool = !val;",
            "negated",
            "return 1;",
            "return 0;"
        );
    
    bdd_run_execution_scenario("Logical NOT operator",
                               "bool_not.asthra",
                               source,
                               NULL,
                               0);
}

void test_logical_and(void) {
    const char* source = 
        BDD_CONDITIONAL_TEST(
            "let a: bool = true; let b: bool = true; let c: bool = false; let result1: bool = a && b; let result2: bool = a && c;",
            "result1 && !result2",
            "return 0;",
            "return 1;"
        );
    
    bdd_run_execution_scenario("Logical AND operator",
                               "bool_and.asthra",
                               source,
                               NULL,
                               0);
}

void test_logical_or(void) {
    const char* source = 
        BDD_CONDITIONAL_TEST(
            "let a: bool = true; let b: bool = false; let c: bool = false; let result1: bool = a || b; let result2: bool = b || c;",
            "result1 && !result2",
            "return 0;",
            "return 1;"
        );
    
    bdd_run_execution_scenario("Logical OR operator",
                               "bool_or.asthra",
                               source,
                               NULL,
                               0);
}

void test_boolean_precedence(void) {
    const char* source = 
        BDD_CONDITIONAL_TEST(
            "let a: bool = true; let b: bool = false; let c: bool = true; let result1: bool = a || b && c; let result2: bool = !a || b && c; let result3: bool = a && b || c;",
            "result1 && !result2 && result3",
            "return 0;",
            "return 1;"
        );
    
    bdd_run_execution_scenario("Boolean operator precedence",
                               "bool_precedence.asthra",
                               source,
                               NULL,
                               0);
}

void test_complex_boolean(void) {
    const char* source = 
        BDD_CONDITIONAL_TEST(
            "let x: i32 = 5; let y: i32 = 10; let z: i32 = 15; let result: bool = (x < y) && (y < z) || (x == 5);",
            "result",
            "return 0;",
            "return 1;"
        );
    
    bdd_run_execution_scenario("Complex boolean expressions",
                               "bool_complex.asthra",
                               source,
                               NULL,
                               0);
}

void test_boolean_as_values(void) {
    const char* source = 
        BDD_TEST_PACKAGE_HEADER
        "pub fn get_bool(val: i32) -> bool {\n"
        "    return val > 0;\n"
        "}\n"
        "\n"
        BDD_TEST_MAIN_SIGNATURE
        "    let a: bool = get_bool(5);\n"
        "    let b: bool = get_bool(-5);\n"
        "    let c: bool = get_bool(0);\n"
        "    let result: bool = a && !b && !c;\n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        BDD_TEST_MAIN_FOOTER;
    
    bdd_run_execution_scenario("Boolean expressions as values",
                               "bool_values.asthra",
                               source,
                               NULL,
                               0);
}

void test_short_circuit_and(void) {
    const char* source = 
        BDD_TEST_PACKAGE_HEADER
        "pub fn always_false(none) -> bool {\n"
        "    return false;\n"
        "}\n"
        "\n"
        "pub fn should_not_call(none) -> bool {\n"
        "    return true;\n"
        "}\n"
        "\n"
        BDD_TEST_MAIN_SIGNATURE
        "    let result: bool = always_false() && should_not_call();\n"
        "    if !result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        BDD_TEST_MAIN_FOOTER;
    
    bdd_run_execution_scenario("Short-circuit evaluation with AND",
                               "bool_short_and.asthra",
                               source,
                               NULL,
                               0);
}

void test_short_circuit_or(void) {
    const char* source = 
        BDD_TEST_PACKAGE_HEADER
        "pub fn always_true(none) -> bool {\n"
        "    return true;\n"
        "}\n"
        "\n"
        "pub fn should_not_call(none) -> bool {\n"
        "    return false;\n"
        "}\n"
        "\n"
        BDD_TEST_MAIN_SIGNATURE
        "    let result: bool = always_true() || should_not_call();\n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        BDD_TEST_MAIN_FOOTER;
    
    bdd_run_execution_scenario("Short-circuit evaluation with OR",
                               "bool_short_or.asthra",
                               source,
                               NULL,
                               0);
}

void test_nested_boolean(void) {
    const char* source = 
        BDD_CONDITIONAL_TEST(
            "let a: bool = true; let b: bool = false; let c: bool = true; let d: bool = false; let result: bool = (a && (b || c)) && !(d || !c);",
            "result",
            "return 0;",
            "return 1;"
        );
    
    bdd_run_execution_scenario("Nested boolean expressions",
                               "bool_nested.asthra",
                               source,
                               NULL,
                               0);
}

void test_boolean_type_inference(void) {
    const char* source = 
        BDD_CONDITIONAL_TEST(
            "let inferred: bool = 5 > 3; let also_inferred: bool = true && false;",
            "inferred && !also_inferred",
            "return 0;",
            "return 1;"
        );
    
    bdd_run_execution_scenario("Boolean type inference",
                               "bool_inference.asthra",
                               source,
                               NULL,
                               0);
}

void test_mutable_boolean(void) {
    const char* source = 
        BDD_VARIABLE_TEST(
            "let mut flag: bool = true; flag = !flag; flag = flag || true; flag = flag && false;",
            "if !flag { return 0; } else { return 1; }"
        );
    
    bdd_run_execution_scenario("Boolean assignment and mutation",
                               "bool_mutation.asthra",
                               source,
                               NULL,
                               0);
}

// Error test scenarios
void test_type_mismatch_not(void) {
    const char* source = 
        BDD_VARIABLE_TEST(
            "let num: i32 = 42; let result: bool = !num;",
            "return 0;"
        );
    
    bdd_run_type_mismatch_scenario("Error - Type mismatch in boolean operation",
                                   "bool_error_not.asthra",
                                   source);
}

void test_type_mismatch_and(void) {
    const char* source = 
        BDD_VARIABLE_TEST(
            "let a: bool = true; let b: i32 = 1; let result: bool = a && b;",
            "return 0;"
        );
    
    bdd_run_type_mismatch_scenario("Error - Non-boolean in logical AND",
                                   "bool_error_and.asthra",
                                   source);
}

void test_type_mismatch_or(void) {
    const char* source = 
        BDD_VARIABLE_TEST(
            "let a: bool = true; let b: i32 = 1; let result = a || b;",
            "return 0;"
        );
    
    bdd_run_type_mismatch_scenario("Error - Non-boolean in logical OR",
                                   "bool_error_or.asthra",
                                   source);
}

void test_type_mismatch_if(void) {
    const char* source = 
        BDD_TEST_PACKAGE_HEADER
        BDD_TEST_MAIN_SIGNATURE
        "    let num: i32 = 42;\n"
        "    if num {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        BDD_TEST_MAIN_FOOTER;
    
    bdd_run_compilation_scenario("Error - Non-boolean condition in if",
                                 "bool_error_if.asthra",
                                 source,
                                 0,
                                 "condition");
}

// Define test cases using the new framework
BDD_DECLARE_TEST_CASES(boolean_operators)
    BDD_TEST_CASE(boolean_literals, test_boolean_literals),
    BDD_TEST_CASE(logical_not, test_logical_not),
    BDD_TEST_CASE(logical_and, test_logical_and),
    BDD_TEST_CASE(logical_or, test_logical_or),
    BDD_TEST_CASE(boolean_precedence, test_boolean_precedence),
    BDD_TEST_CASE(complex_boolean, test_complex_boolean),
    BDD_TEST_CASE(boolean_as_values, test_boolean_as_values),
    BDD_TEST_CASE(short_circuit_and, test_short_circuit_and),
    BDD_TEST_CASE(short_circuit_or, test_short_circuit_or),
    BDD_TEST_CASE(nested_boolean, test_nested_boolean),
    BDD_TEST_CASE(boolean_type_inference, test_boolean_type_inference),
    BDD_TEST_CASE(mutable_boolean, test_mutable_boolean),
    BDD_TEST_CASE(type_mismatch_not, test_type_mismatch_not),
    BDD_TEST_CASE(type_mismatch_and, test_type_mismatch_and),
    BDD_TEST_CASE(type_mismatch_or, test_type_mismatch_or),
    BDD_TEST_CASE(type_mismatch_if, test_type_mismatch_if),
BDD_END_TEST_CASES()

// Main test runner using the new framework - ELIMINATED DUPLICATION!
BDD_UNIT_TEST_MAIN("Boolean operators", boolean_operators_test_cases)