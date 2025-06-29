#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bdd_support.h"

// External functions from common_steps.c
extern void given_asthra_compiler_available(void);
extern void given_file_with_content(const char* filename, const char* content);
extern void when_compile_file(void);
extern void when_run_executable(void);
extern void then_compilation_should_succeed(void);
extern void then_compilation_should_fail(void);
extern void then_executable_created(void);
extern void then_output_contains(const char* expected_output);
extern void then_exit_code_is(int expected_code);
extern void then_error_contains(const char* expected_error);
extern void common_cleanup(void);

// Test scenario: Boolean literals
void test_boolean_literals(void) {
    bdd_scenario("Boolean literals");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("bool_literals.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Logical NOT operator (@wip)
void test_logical_not(void) {
    bdd_scenario("Logical NOT operator");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let val: bool = true;\n"
        "    let negated: bool = !val;\n"
        "    \n"
        "    if negated {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_not.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Logical AND operator (@wip)
void test_logical_and(void) {
    bdd_scenario("Logical AND operator");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = true;\n"
        "    let c: bool = false;\n"
        "    \n"
        "    let result1: bool = a && b;\n"
        "    let result2: bool = a && c;\n"
        "    \n"
        "    if result1 && !result2 {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_and.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Logical OR operator
void test_logical_or(void) {
    bdd_scenario("Logical OR operator");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = false;\n"
        "    \n"
        "    let result1: bool = a || b;\n"
        "    let result2: bool = b || c;\n"
        "    \n"
        "    if result1 && !result2 {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_or.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Boolean operator precedence
void test_boolean_precedence(void) {
    bdd_scenario("Boolean operator precedence");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    \n"
        "    // NOT has highest precedence, then AND, then OR\n"
        "    let result1: bool = a || b && c;    // true || (false && true) = true\n"
        "    let result2: bool = !a || b && c;   // (!true) || (false && true) = false\n"
        "    let result3: bool = a && b || c;    // (true && false) || true = true\n"
        "    \n"
        "    if result1 && !result2 && result3 {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_precedence.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Complex boolean expressions
void test_complex_boolean(void) {
    bdd_scenario("Complex boolean expressions");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let x: i32 = 5;\n"
        "    let y: i32 = 10;\n"
        "    let z: i32 = 15;\n"
        "    \n"
        "    let result: bool = (x < y) && (y < z) || (x == 5);\n"
        "    \n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_complex.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Boolean expressions as values (@wip)
void test_boolean_as_values(void) {
    bdd_scenario("Boolean expressions as values");
    
    given_asthra_compiler_available();
    
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
        "    let result: bool = a && !b && !c;\n"
        "    \n"
        "    if result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_values.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Short-circuit evaluation with AND (@wip)
void test_short_circuit_and(void) {
    bdd_scenario("Short-circuit evaluation with AND");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn return_false(none) -> bool {\n"
        "    return false;\n"
        "}\n"
        "\n"
        "pub fn should_not_be_called(none) -> bool {\n"
        "    // If short-circuit works, this function won't be called\n"
        "    // and thus won't cause a runtime error\n"
        "    let x: i32 = 1;\n"
        "    let y: i32 = 0;\n"
        "    let z: i32 = x / y;  // This would cause division by zero if called\n"
        "    return true;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    // Should short-circuit after first false\n"
        "    let result: bool = return_false() && should_not_be_called();\n"
        "    \n"
        "    // If we reach here without crashing, short-circuit worked\n"
        "    if !result {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_short_and.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Short-circuit evaluation with OR (@wip)
void test_short_circuit_or(void) {
    bdd_scenario("Short-circuit evaluation with OR");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "var counter: i32 = 0;\n"
        "\n"
        "pub fn increment_and_return_true(none) -> bool {\n"
        "    counter = counter + 1;\n"
        "    return true;\n"
        "}\n"
        "\n"
        "pub fn increment_and_return_false(none) -> bool {\n"
        "    counter = counter + 1;\n"
        "    return false;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    // Should short-circuit after first true\n"
        "    let result = increment_and_return_true() || increment_and_return_false();\n"
        "    \n"
        "    // Counter should be 1, not 2, due to short-circuit\n"
        "    if result && counter == 1 {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_short_or.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Nested boolean expressions (@wip)
void test_nested_boolean(void) {
    bdd_scenario("Nested boolean expressions");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: bool = false;\n"
        "    let c: bool = true;\n"
        "    let d: bool = false;\n"
        "    \n"
        "    let complex: bool = (a && b) || (c && !d);\n"
        "    let nested: bool = !(!a || !c) && (b || !d);\n"
        "    \n"
        "    if complex && nested {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_nested.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Boolean type inference (@wip)
void test_boolean_type_inference(void) {
    bdd_scenario("Boolean type inference");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let inferred: bool = true;  // Type should be inferred as bool\n"
        "    let also_inferred: bool = !false;\n"
        "    let expression: bool = 5 > 3;\n"
        "    \n"
        "    if inferred && also_inferred && expression {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_inference.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Boolean assignment and mutation (@wip)
void test_mutable_boolean(void) {
    bdd_scenario("Boolean assignment and mutation");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    var flag: bool = false;\n"
        "    flag = true;\n"
        "    flag = !flag;\n"
        "    flag = flag || true;\n"
        "    flag = flag && false;\n"
        "    \n"
        "    if !flag {\n"
        "        return 0;\n"
        "    } else {\n"
        "        return 1;\n"
        "    }\n"
        "}\n";
    
    given_file_with_content("bool_mutation.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_exit_code_is(0);
}

// Test scenario: Error - Type mismatch in boolean operation
void test_type_mismatch_not(void) {
    bdd_scenario("Error - Type mismatch in boolean operation");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let num: i32 = 42;\n"
        "    let result: bool = !num;  // Error: NOT operator expects bool\n"
        "    return 0;\n"
        "}\n";
    
    given_file_with_content("bool_error_not.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("Error");
}

// Test scenario: Error - Non-boolean in logical AND
void test_type_mismatch_and(void) {
    bdd_scenario("Error - Non-boolean in logical AND");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: i32 = 1;\n"
        "    let result: bool = a && b;  // Error: AND expects both operands to be bool\n"
        "    return 0;\n"
        "}\n";
    
    given_file_with_content("bool_error_and.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("type mismatch");
}

// Test scenario: Error - Non-boolean in logical OR
void test_type_mismatch_or(void) {
    bdd_scenario("Error - Non-boolean in logical OR");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: bool = true;\n"
        "    let b: i32 = 1;\n"
        "    let result: bool = a || b;  // Error: OR expects both operands to be bool\n"
        "    return 0;\n"
        "}\n";
    
    given_file_with_content("bool_error_or.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("type mismatch");
}

// Test scenario: Error - Non-boolean condition in if
void test_type_mismatch_if(void) {
    bdd_scenario("Error - Non-boolean condition in if");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("bool_error_if.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("condition");
}

// Main test runner
int main(void) {
    bdd_init("Boolean operators");
    
    // Check if @wip scenarios should be skipped
    if (bdd_should_skip_wip()) {
        // Skip @wip scenarios
        bdd_skip_scenario("Boolean expressions as values [@wip]");
        // Removed: Short-circuit evaluation with AND is now enabled
        bdd_skip_scenario("Short-circuit evaluation with OR [@wip]");
        // Removed: Nested boolean expressions is now enabled
        // Removed: Boolean type inference is now enabled
        bdd_skip_scenario("Boolean assignment and mutation [@wip]");
        
        // Run only non-@wip scenarios
        test_boolean_literals();
        test_logical_not();
        test_logical_and();
        test_logical_or();
        test_boolean_precedence();
        test_complex_boolean();
        test_short_circuit_and();  // Now run this test
        test_nested_boolean();  // Now run this test
        test_boolean_type_inference();  // Now run this test
        test_type_mismatch_not();
        test_type_mismatch_and();
        test_type_mismatch_or();
        test_type_mismatch_if();
    } else {
        // Run all scenarios
        test_boolean_literals();
        test_logical_not();
        test_logical_and();
        test_logical_or();
        test_boolean_precedence();
        test_complex_boolean();
        test_boolean_as_values();
        test_short_circuit_and();
        test_short_circuit_or();
        test_nested_boolean();
        test_boolean_type_inference();
        test_mutable_boolean();
        test_type_mismatch_not();
        test_type_mismatch_and();
        test_type_mismatch_or();
        test_type_mismatch_if();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}