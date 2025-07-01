#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <sys/stat.h>

// Test scenarios for tuple patterns

// Basic tuple destructuring
void test_match_tuple_with_two_elements(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, i32) = (10, 32);\n"
        "  match t {\n"
        "    (a, b) => { return a + b; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match tuple with two elements",
                               "tuple_two_elements.asthra",
                               source,
                               NULL,
                               42);
}

void test_match_tuple_with_three_elements(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, i32, i32) = (20, 20, 2);\n"
        "  match t {\n"
        "    (a, b, c) => { return a + b + c; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match tuple with three elements",
                               "tuple_three_elements.asthra",
                               source,
                               NULL,
                               42);
}

void test_match_tuple_with_wildcard_patterns(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, i32, i32) = (42, 100, 200);\n"
        "  match t {\n"
        "    (x, _, _) => { return x; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match tuple with wildcard patterns",
                               "tuple_wildcards.asthra",
                               source,
                               NULL,
                               42);
}

// If-let tuple patterns
void test_if_let_with_tuple_pattern(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, i32) = (40, 2);\n"
        "  if let (a, b) = t {\n"
        "    return a + b;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("If-let with tuple pattern",
                               "if_let_tuple.asthra",
                               source,
                               NULL,
                               42);
}

// Nested tuple patterns
void test_match_nested_tuples(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: ((i32, i32), i32) = ((20, 22), 0);\n"
        "  match t {\n"
        "    ((a, b), _) => { return a + b; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match nested tuples",
                               "nested_tuples.asthra",
                               source,
                               NULL,
                               42);
}

void test_match_deeply_nested_tuples(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, (i32, (i32, i32))) = (1, (10, (11, 20)));\n"
        "  match t {\n"
        "    (a, (b, (c, d))) => { return a + b + c + d; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match deeply nested tuples",
                               "deeply_nested_tuples.asthra",
                               source,
                               NULL,
                               42);
}

// Mixed type tuples
void test_match_tuple_with_different_types(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, bool, i32) = (20, true, 22);\n"
        "  match t {\n"
        "    (a, true, b) => { return a + b; }\n"
        "    (a, false, b) => { return a - b; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match tuple with different types",
                               "tuple_mixed_types.asthra",
                               source,
                               NULL,
                               42);
}

// Tuple patterns in functions
void test_tuple_pattern_in_function_parameter(void) {
    const char* source = 
        "package test;\n"
        "pub fn add_pair(p: (i32, i32)) -> i32 {\n"
        "  match p {\n"
        "    (a, b) => { return a + b; }\n"
        "  }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let pair: (i32, i32) = (20, 22);\n"
        "  return add_pair(pair);\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple pattern in function parameter",
                               "tuple_func_param.asthra",
                               source,
                               NULL,
                               42);
}

// Tuple element access
void test_access_tuple_elements_with_index_syntax(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, i32, i32) = (42, 100, 200);\n"
        "  return t.0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Access tuple elements with index syntax",
                               "tuple_index_access.asthra",
                               source,
                               NULL,
                               42);
}

void test_access_nested_tuple_elements(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: ((i32, i32), i32) = ((42, 100), 200);\n"
        "  return t.0.0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Access nested tuple elements",
                               "nested_tuple_access.asthra",
                               source,
                               NULL,
                               42);
}

// Complex patterns
void test_match_tuple_in_struct(void) {
    const char* source = 
        "package test;\n"
        "pub struct Container {\n"
        "  data: (i32, i32)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let c: Container = Container { data: (20, 22) };\n"
        "  match c.data {\n"
        "    (a, b) => { return a + b; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match tuple in struct",
                               "tuple_in_struct.asthra",
                               source,
                               NULL,
                               42);
}

void test_match_tuple_in_enum_variant(void) {
    const char* source = 
        "package test;\n"
        "pub enum Data {\n"
        "  Pair(i32, i32),\n"
        "  Single(i32)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let d: Data = Data.Pair(20, 22);\n"
        "  match d {\n"
        "    Data.Pair(a, b) => { return a + b; }\n"
        "    Data.Single(x) => { return x; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match tuple in enum variant",
                               "tuple_in_enum.asthra",
                               source,
                               NULL,
                               42);
}

// Multiple patterns
void test_match_multiple_tuple_patterns(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, i32) = (42, 0);\n"
        "  match t {\n"
        "    (0, y) => { return y; }\n"
        "    (x, 0) => { return x; }\n"
        "    (x, y) => { return x + y; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match multiple tuple patterns",
                               "tuple_multiple_patterns.asthra",
                               source,
                               NULL,
                               42);
}

// Pattern matching expressions
void test_tuple_pattern_in_match_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn compute(none) -> (i32, i32) {\n"
        "  return (40, 2);\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  match compute(none) {\n"
        "    (a, b) => { return a + b; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple pattern in match expression",
                               "tuple_match_expr.asthra",
                               source,
                               NULL,
                               42);
}

// Variable binding in tuple patterns
void test_bind_variables_in_nested_tuple_pattern(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, (i32, i32)) = (10, (12, 20));\n"
        "  match t {\n"
        "    (x, (y, z)) => {\n"
        "      let sum: i32 = x + y + z;\n"
        "      return sum;\n"
        "    }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind variables in nested tuple pattern",
                               "tuple_var_binding.asthra",
                               source,
                               NULL,
                               42);
}

// Edge cases
void test_match_tuple_with_all_wildcards_except_one(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, i32, i32, i32) = (1, 42, 3, 4);\n"
        "  match t {\n"
        "    (_, x, _, _) => { return x; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match tuple with all wildcards except one",
                               "tuple_mostly_wildcards.asthra",
                               source,
                               NULL,
                               42);
}

void test_tuple_pattern_with_literal_matching(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, i32) = (1, 41);\n"
        "  match t {\n"
        "    (1, x) => { return x + 1; }\n"
        "    (2, x) => { return x + 2; }\n"
        "    (_, x) => { return x; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple pattern with literal matching",
                               "tuple_literal_match.asthra",
                               source,
                               NULL,
                               42);
}

// Tuple assignment through pattern matching
void test_destructure_tuple_in_let_binding(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, i32) = (20, 22);\n"
        "  let (a, b): (i32, i32) = t;\n"
        "  return a + b;\n"
        "}\n";
    
    bdd_run_execution_scenario("Destructure tuple in let binding",
                               "tuple_let_destructure.asthra",
                               source,
                               NULL,
                               42);
}

// Mutable tuple elements
void test_match_and_modify_tuple_elements(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let mut t: (i32, i32) = (20, 20);\n"
        "  match t {\n"
        "    (a, b) => {\n"
        "      t = (a + 1, b + 1);\n"
        "    }\n"
        "  }\n"
        "  match t {\n"
        "    (x, y) => { return x + y; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match and modify tuple elements",
                               "tuple_mutable.asthra",
                               source,
                               NULL,
                               42);
}

// Tuple patterns with string types
void test_match_tuple_containing_strings(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (string, i32) = (\"answer\", 42);\n"
        "  match t {\n"
        "    (\"answer\", x) => { return x; }\n"
        "    (_, x) => { return 0; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match tuple containing strings",
                               "tuple_with_strings.asthra",
                               source,
                               NULL,
                               42);
}

// Define test cases
BddTestCase tuple_patterns_test_cases[] = {
    // Basic tuple destructuring
    BDD_WIP_TEST_CASE(match_tuple_with_two_elements, test_match_tuple_with_two_elements),
    BDD_WIP_TEST_CASE(match_tuple_with_three_elements, test_match_tuple_with_three_elements),
    BDD_WIP_TEST_CASE(match_tuple_with_wildcard_patterns, test_match_tuple_with_wildcard_patterns),
    
    // If-let tuple patterns
    BDD_WIP_TEST_CASE(if_let_with_tuple_pattern, test_if_let_with_tuple_pattern),
    
    // Nested tuple patterns
    BDD_WIP_TEST_CASE(match_nested_tuples, test_match_nested_tuples),
    BDD_WIP_TEST_CASE(match_deeply_nested_tuples, test_match_deeply_nested_tuples),
    
    // Mixed type tuples
    BDD_WIP_TEST_CASE(match_tuple_with_different_types, test_match_tuple_with_different_types),
    
    // Tuple patterns in functions
    BDD_WIP_TEST_CASE(tuple_pattern_in_function_parameter, test_tuple_pattern_in_function_parameter),
    
    // Tuple element access
    BDD_WIP_TEST_CASE(access_tuple_elements_with_index_syntax, test_access_tuple_elements_with_index_syntax),
    BDD_WIP_TEST_CASE(access_nested_tuple_elements, test_access_nested_tuple_elements),
    
    // Complex patterns
    BDD_WIP_TEST_CASE(match_tuple_in_struct, test_match_tuple_in_struct),
    BDD_WIP_TEST_CASE(match_tuple_in_enum_variant, test_match_tuple_in_enum_variant),
    
    // Multiple patterns
    BDD_WIP_TEST_CASE(match_multiple_tuple_patterns, test_match_multiple_tuple_patterns),
    
    // Pattern matching expressions
    BDD_WIP_TEST_CASE(tuple_pattern_in_match_expression, test_tuple_pattern_in_match_expression),
    
    // Variable binding in tuple patterns
    BDD_WIP_TEST_CASE(bind_variables_in_nested_tuple_pattern, test_bind_variables_in_nested_tuple_pattern),
    
    // Edge cases
    BDD_WIP_TEST_CASE(match_tuple_with_all_wildcards_except_one, test_match_tuple_with_all_wildcards_except_one),
    BDD_WIP_TEST_CASE(tuple_pattern_with_literal_matching, test_tuple_pattern_with_literal_matching),
    
    // Tuple assignment through pattern matching
    BDD_WIP_TEST_CASE(destructure_tuple_in_let_binding, test_destructure_tuple_in_let_binding),
    
    // Mutable tuple elements
    BDD_WIP_TEST_CASE(match_and_modify_tuple_elements, test_match_and_modify_tuple_elements),
    
    // Tuple patterns with string types
    BDD_WIP_TEST_CASE(match_tuple_containing_strings, test_match_tuple_containing_strings),
};

// Main test runner
int main(void) {
    return bdd_run_test_suite("Tuple Patterns",
                              tuple_patterns_test_cases,
                              sizeof(tuple_patterns_test_cases) / sizeof(tuple_patterns_test_cases[0]),
                              bdd_cleanup_temp_files);
}