#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"

// Test scenarios using the new reusable framework

void test_boolean_literals(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let t: bool = true;\n"
        "    let f: bool = false;\n"
        "    \n"
        "    if t && !f {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Boolean literals",
                               "bool_literals.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_logical_not(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let val: bool = true;\n"
        "    let negated = !val;\n"
        "    \n"
        "    if negated {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical NOT operator",
                               "bool_not.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_logical_and(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = true;\n"
        "    let c: bool = false;\n"
        "    \n"
        "    let result1 = a && b;\n"
        "    let result2 = a && c;\n"
        "    \n"
        "    if result1 && !result2 {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical AND operator",
                               "bool_and.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_logical_or(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = false;\n"
        "    \n"
        "    let result1 = a || b;\n"
        "    let result2 = b || c;\n"
        "    \n"
        "    if result1 && !result2 {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Logical OR operator",
                               "bool_or.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_boolean_precedence(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    \n"
        "    // NOT has highest precedence, then AND, then OR\n"
        "    let result1 = a || b && c;    // true || (false && true) = true\n"
        "    let result2 = !a || b && c;   // (!true) || (false && true) = false\n"
        "    let result3 = a && b || c;    // (true && false) || true = true\n"
        "    \n"
        "    if result1 && !result2 && result3 {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Boolean operator precedence",
                               "bool_precedence.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_complex_boolean(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 5;\n"
        "    let y: i32 = 10;\n"
        "    let z: i32 = 15;\n"
        "    \n"
        "    let result = (x < y) && (y < z) || (x == 5);\n"
        "    \n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Complex boolean expressions",
                               "bool_complex.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_boolean_as_values(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn get_bool(val: i32) -> bool {\n"
        "    return val > 0;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = get_bool(5);\n"
        "    let b: bool = get_bool(-5);\n"
        "    let c: bool = get_bool(0);\n"
        "    \n"
        "    let result = a && !b && !c;\n"
        "    \n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Boolean expressions as values",
                               "bool_values.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_short_circuit_and(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn always_false() -> bool {\n"
        "    return false;\n"
        "}\n"
        "\n"
        "pub fn should_not_call() -> bool {\n"
        "    // This should not be called due to short-circuit\n"
        "    return true;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let result = always_false() && should_not_call();\n"
        "    \n"
        "    if !result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Short-circuit evaluation with AND",
                               "bool_short_and.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_short_circuit_or(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn always_true() -> bool {\n"
        "    return true;\n"
        "}\n"
        "\n"
        "pub fn should_not_call() -> bool {\n"
        "    // This should not be called due to short-circuit\n"
        "    return false;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let result = always_true() || should_not_call();\n"
        "    \n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Short-circuit evaluation with OR",
                               "bool_short_or.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_nested_boolean(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    let d: bool = false;\n"
        "    \n"
        "    // Complex nested expression\n"
        "    let result = (a && (b || c)) && !(d || !c);\n"
        "    \n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested boolean expressions",
                               "bool_nested.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_boolean_type_inference(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    // Type inference for boolean expressions\n"
        "    let inferred = 5 > 3;\n"
        "    let also_inferred = true && false;\n"
        "    \n"
        "    if inferred && !also_inferred {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Boolean type inference",
                               "bool_inference.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

void test_mutable_boolean(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut flag: bool = true;\n"
        "    \n"
        "    flag = !flag;  // Now false\n"
        "    flag = flag || true;  // Now true\n"
        "    flag = flag && false;  // Now false\n"
        "    \n"
        "    if !flag {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Boolean assignment and mutation",
                               "bool_mutation.asthra",
                               source,
                               NULL,  // No specific output expected
                               0);
}

// Error test scenarios
void test_type_mismatch_not(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let num: i32 = 42;\n"
        "    let result = !num;  // Error: NOT operator expects bool\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Error - Type mismatch in boolean operation",
                                 "bool_error_not.asthra",
                                 source,
                                 0,  // should fail
                                 "type mismatch");
}

void test_type_mismatch_and(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: i32 = 1;\n"
        "    let result = a && b;  // Error: AND expects both operands to be bool\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Error - Non-boolean in logical AND",
                                 "bool_error_and.asthra",
                                 source,
                                 0,  // should fail
                                 "type mismatch");
}

void test_type_mismatch_or(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: i32 = 1;\n"
        "    let result = a || b;  // Error: OR expects both operands to be bool\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Error - Non-boolean in logical OR",
                                 "bool_error_or.asthra",
                                 source,
                                 0,  // should fail
                                 "type mismatch");
}

void test_type_mismatch_if(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let num: i32 = 42;\n"
        "    \n"
        "    if num {  // Error: if condition must be bool\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    bdd_run_compilation_scenario("Error - Non-boolean condition in if",
                                 "bool_error_if.asthra",
                                 source,
                                 0,  // should fail
                                 "condition");
}

// Define test cases using the new framework - @wip tags based on original file
BddTestCase boolean_operators_test_cases[] = {
    BDD_TEST_CASE(boolean_literals, test_boolean_literals),
    BDD_WIP_TEST_CASE(logical_not, test_logical_not),
    BDD_WIP_TEST_CASE(logical_and, test_logical_and),
    BDD_TEST_CASE(logical_or, test_logical_or),
    BDD_TEST_CASE(boolean_precedence, test_boolean_precedence),
    BDD_TEST_CASE(complex_boolean, test_complex_boolean),
    BDD_WIP_TEST_CASE(boolean_as_values, test_boolean_as_values),
    BDD_WIP_TEST_CASE(short_circuit_and, test_short_circuit_and),
    BDD_WIP_TEST_CASE(short_circuit_or, test_short_circuit_or),
    BDD_WIP_TEST_CASE(nested_boolean, test_nested_boolean),
    BDD_WIP_TEST_CASE(boolean_type_inference, test_boolean_type_inference),
    BDD_WIP_TEST_CASE(mutable_boolean, test_mutable_boolean),
    BDD_TEST_CASE(type_mismatch_not, test_type_mismatch_not),
    BDD_TEST_CASE(type_mismatch_and, test_type_mismatch_and),
    BDD_TEST_CASE(type_mismatch_or, test_type_mismatch_or),
    BDD_TEST_CASE(type_mismatch_if, test_type_mismatch_if),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Boolean operators",
                              boolean_operators_test_cases,
                              sizeof(boolean_operators_test_cases) / sizeof(boolean_operators_test_cases[0]),
                              bdd_cleanup_temp_files);
}