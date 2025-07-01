#include "bdd_unit_common.h"
// Test scenarios for tuple literals

// Basic tuple creation (minimum 2 elements)
void test_two_element_integer_tuple(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let pair: (i32, i32) = (20, 22);\n"
        "  return pair.0 + pair.1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Two-element integer tuple",
                               "tuple_two_int.asthra",
                               source,
                               NULL,
                               42);
}

void test_two_element_mixed_type_tuple(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let mixed: (i32, bool) = (42, true);\n"
        "  if mixed.1 {\n"
        "    return mixed.0;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Two-element mixed type tuple",
                               "tuple_mixed.asthra",
                               source,
                               NULL,
                               42);
}

void test_three_element_tuple(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let triple: (i32, i32, i32) = (10, 15, 17);\n"
        "  return triple.0 + triple.1 + triple.2;\n"
        "}\n";
    
    bdd_run_execution_scenario("Three-element tuple",
                               "tuple_three.asthra",
                               source,
                               NULL,
                               42);
}

void test_four_element_tuple(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let quad: (i8, i16, i32, i8) = (5, 10, 20, 7);\n"
        "  return (quad.0 as i32) + (quad.1 as i32) + quad.2 + (quad.3 as i32);\n"
        "}\n";
    
    bdd_run_execution_scenario("Four-element tuple",
                               "tuple_four.asthra",
                               source,
                               NULL,
                               42);
}

// Tuple element access
void test_access_first_element(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let tup: (i32, i32, i32) = (42, 100, 200);\n"
        "  return tup.0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Access first element",
                               "tuple_first.asthra",
                               source,
                               NULL,
                               42);
}

void test_access_middle_element(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let tup: (i32, i32, i32, i32, i32) = (1, 2, 42, 4, 5);\n"
        "  return tup.2;\n"
        "}\n";
    
    bdd_run_execution_scenario("Access middle element",
                               "tuple_middle.asthra",
                               source,
                               NULL,
                               42);
}

void test_access_last_element(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let tup: (i32, i32, i32) = (10, 20, 42);\n"
        "  return tup.2;\n"
        "}\n";
    
    bdd_run_execution_scenario("Access last element",
                               "tuple_last.asthra",
                               source,
                               NULL,
                               42);
}

// Mixed type tuples
void test_integer_and_boolean_tuple(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let data: (i32, bool) = (84, false);\n"
        "  if !data.1 {\n"
        "    return data.0 / 2;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Integer and boolean tuple",
                               "tuple_int_bool.asthra",
                               source,
                               NULL,
                               42);
}

void test_string_and_integer_tuple(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let info: (string, i32) = (\"answer\", 42);\n"
        "  if info.0 == \"answer\" {\n"
        "    return info.1;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("String and integer tuple",
                               "tuple_string_int.asthra",
                               source,
                               NULL,
                               42);
}

void test_float_and_integer_tuple(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let nums: (f32, i32) = (21.0, 21);\n"
        "  return (nums.0 as i32) + nums.1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Float and integer tuple",
                               "tuple_float_int.asthra",
                               source,
                               NULL,
                               42);
}

// Nested tuples
void test_tuple_containing_tuple(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let nested: ((i32, i32), i32) = ((20, 15), 7);\n"
        "  let inner: (i32, i32) = nested.0;\n"
        "  return inner.0 + inner.1 + nested.1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple containing tuple",
                               "tuple_nested.asthra",
                               source,
                               NULL,
                               42);
}

void test_deeply_nested_tuples(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let deep: (i32, (i32, (i32, i32))) = (10, (12, (15, 5)));\n"
        "  return deep.0 + deep.1.0 + deep.1.1.0 + deep.1.1.1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Deeply nested tuples",
                               "tuple_deep_nested.asthra",
                               source,
                               NULL,
                               42);
}

// Tuple pattern matching
void test_basic_tuple_destructuring(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let pair: (i32, i32) = (17, 25);\n"
        "  let (a, b): (i32, i32) = pair;\n"
        "  return a + b;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic tuple destructuring",
                               "tuple_destructure.asthra",
                               source,
                               NULL,
                               42);
}

void test_tuple_destructuring_with_underscore(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let triple: (i32, i32, i32) = (42, 100, 200);\n"
        "  let (x, _, _): (i32, i32, i32) = triple;\n"
        "  return x;\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple destructuring with underscore",
                               "tuple_destructure_underscore.asthra",
                               source,
                               NULL,
                               42);
}

void test_nested_tuple_destructuring(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let nested: ((i32, i32), i32) = ((15, 20), 7);\n"
        "  let ((a, b), c): ((i32, i32), i32) = nested;\n"
        "  return a + b + c;\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested tuple destructuring",
                               "tuple_nested_destructure.asthra",
                               source,
                               NULL,
                               42);
}

// Tuples as function parameters
void test_pass_tuple_to_function(void) {
    const char* source = 
        "package test;\n"
        "pub fn sum_pair(p: (i32, i32)) -> i32 {\n"
        "  return p.0 + p.1;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let pair: (i32, i32) = (20, 22);\n"
        "  return sum_pair(pair);\n"
        "}\n";
    
    bdd_run_execution_scenario("Pass tuple to function",
                               "tuple_func_param.asthra",
                               source,
                               NULL,
                               42);
}

void test_return_tuple_from_function(void) {
    const char* source = 
        "package test;\n"
        "pub fn make_pair(none) -> (i32, i32) {\n"
        "  return (17, 25);\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let p: (i32, i32) = make_pair(none);\n"
        "  return p.0 + p.1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Return tuple from function",
                               "tuple_func_return.asthra",
                               source,
                               NULL,
                               42);
}

// Mutable tuples
void test_mutable_tuple_reassignment(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let mut tup: (i32, i32) = (10, 20);\n"
        "  tup = (15, 27);\n"
        "  return tup.0 + tup.1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Mutable tuple reassignment",
                               "tuple_mutable.asthra",
                               source,
                               NULL,
                               42);
}

// Tuple comparison
void test_compare_equal_tuples(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t1: (i32, i32) = (21, 21);\n"
        "  let t2: (i32, i32) = (21, 21);\n"
        "  if t1 == t2 {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Compare equal tuples",
                               "tuple_compare_eq.asthra",
                               source,
                               NULL,
                               42);
}

void test_compare_different_tuples(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let t1: (i32, bool) = (42, true);\n"
        "  let t2: (i32, bool) = (42, false);\n"
        "  if t1 != t2 {\n"
        "    return t1.0;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Compare different tuples",
                               "tuple_compare_ne.asthra",
                               source,
                               NULL,
                               42);
}

// Tuples in expressions
void test_tuple_in_conditional_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let result: (i32, bool) = (42, true);\n"
        "  return if result.1 { result.0 } else { 0 };\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple in conditional expression",
                               "tuple_conditional.asthra",
                               source,
                               NULL,
                               42);
}

void test_tuple_element_in_arithmetic(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let nums: (i32, i32, i32) = (8, 5, 3);\n"
        "  return nums.0 * nums.1 + nums.2 - 1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple element in arithmetic",
                               "tuple_arithmetic.asthra",
                               source,
                               NULL,
                               42);
}

// Tuples with arrays
void test_array_of_tuples(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let pairs: [](i32, i32) = [(10, 5), (15, 7), (20, 10)];\n"
        "  let p: (i32, i32) = pairs[1];\n"
        "  return p.0 + p.1 + 20;\n"
        "}\n";
    
    bdd_run_execution_scenario("Array of tuples",
                               "tuple_array.asthra",
                               source,
                               NULL,
                               42);
}

void test_tuple_containing_array(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let data: ([]i32, i32) = ([10, 20, 30], 12);\n"
        "  return data.0[0] + data.0[1] + data.1 - 10;\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple containing array",
                               "tuple_with_array.asthra",
                               source,
                               NULL,
                               42);
}

// Edge cases
void test_large_tuple(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let big: (i32, i32, i32, i32, i32, i32, i32, i32) = (1, 2, 3, 4, 5, 6, 7, 12);\n"
        "  return big.0 + big.1 + big.2 + big.3 + big.4 + big.5 + big.6 + big.7;\n"
        "}\n";
    
    bdd_run_execution_scenario("Large tuple",
                               "tuple_large.asthra",
                               source,
                               NULL,
                               42);
}

void test_tuple_with_unit_types(void) {
    const char* source = 
        "package test;\n"
        "pub fn nothing(none) -> void { return (); }\n"
        "pub fn main(none) -> i32 {\n"
        "  let t: (i32, void) = (42, nothing(none));\n"
        "  return t.0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple with unit types",
                               "tuple_unit.asthra",
                               source,
                               NULL,
                               42);
}

// Const tuples
void test_const_tuple_declaration(void) {
    const char* source = 
        "package test;\n"
        "pub const PAIR: (i32, i32) = (20, 22);\n"
        "pub fn main(none) -> i32 {\n"
        "  return PAIR.0 + PAIR.1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Const tuple declaration",
                               "tuple_const.asthra",
                               source,
                               NULL,
                               42);
}

// Tuple in match expressions
void test_match_on_tuple_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let pair: (i32, bool) = (42, true);\n"
        "  match pair {\n"
        "    (42, true) => { return 42; }\n"
        "    (42, false) => { return 0; }\n"
        "    _ => { return 1; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match on tuple value",
                               "tuple_match.asthra",
                               source,
                               NULL,
                               42);
}

// Tuple with if-let
void test_if_let_with_tuple_pattern(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let data: (i32, i32) = (21, 21);\n"
        "  if let (x, y) = data {\n"
        "    return x + y;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("If-let with tuple pattern",
                               "tuple_if_let.asthra",
                               source,
                               NULL,
                               42);
}

// Define test cases
BDD_DECLARE_TEST_CASES(tuple_literals)
    // Basic tuple creation (minimum 2 elements)
    BDD_WIP_TEST_CASE(two_element_integer_tuple, test_two_element_integer_tuple),
    BDD_WIP_TEST_CASE(two_element_mixed_type_tuple, test_two_element_mixed_type_tuple),
    BDD_WIP_TEST_CASE(three_element_tuple, test_three_element_tuple),
    BDD_WIP_TEST_CASE(four_element_tuple, test_four_element_tuple),
    
    // Tuple element access
    BDD_WIP_TEST_CASE(access_first_element, test_access_first_element),
    BDD_WIP_TEST_CASE(access_middle_element, test_access_middle_element),
    BDD_WIP_TEST_CASE(access_last_element, test_access_last_element),
    
    // Mixed type tuples
    BDD_WIP_TEST_CASE(integer_and_boolean_tuple, test_integer_and_boolean_tuple),
    BDD_WIP_TEST_CASE(string_and_integer_tuple, test_string_and_integer_tuple),
    BDD_WIP_TEST_CASE(float_and_integer_tuple, test_float_and_integer_tuple),
    
    // Nested tuples
    BDD_WIP_TEST_CASE(tuple_containing_tuple, test_tuple_containing_tuple),
    BDD_WIP_TEST_CASE(deeply_nested_tuples, test_deeply_nested_tuples),
    
    // Tuple pattern matching
    BDD_WIP_TEST_CASE(basic_tuple_destructuring, test_basic_tuple_destructuring),
    BDD_WIP_TEST_CASE(tuple_destructuring_with_underscore, test_tuple_destructuring_with_underscore),
    BDD_WIP_TEST_CASE(nested_tuple_destructuring, test_nested_tuple_destructuring),
    
    // Tuples as function parameters
    BDD_WIP_TEST_CASE(pass_tuple_to_function, test_pass_tuple_to_function),
    BDD_WIP_TEST_CASE(return_tuple_from_function, test_return_tuple_from_function),
    
    // Mutable tuples
    BDD_WIP_TEST_CASE(mutable_tuple_reassignment, test_mutable_tuple_reassignment),
    
    // Tuple comparison
    BDD_WIP_TEST_CASE(compare_equal_tuples, test_compare_equal_tuples),
    BDD_WIP_TEST_CASE(compare_different_tuples, test_compare_different_tuples),
    
    // Tuples in expressions
    BDD_WIP_TEST_CASE(tuple_in_conditional_expression, test_tuple_in_conditional_expression),
    BDD_WIP_TEST_CASE(tuple_element_in_arithmetic, test_tuple_element_in_arithmetic),
    
    // Tuples with arrays
    BDD_WIP_TEST_CASE(array_of_tuples, test_array_of_tuples),
    BDD_WIP_TEST_CASE(tuple_containing_array, test_tuple_containing_array),
    
    // Edge cases
    BDD_WIP_TEST_CASE(large_tuple, test_large_tuple),
    BDD_WIP_TEST_CASE(tuple_with_unit_types, test_tuple_with_unit_types),
    
    // Const tuples
    BDD_WIP_TEST_CASE(const_tuple_declaration, test_const_tuple_declaration),
    
    // Tuple in match expressions
    BDD_WIP_TEST_CASE(match_on_tuple_value, test_match_on_tuple_value),
    
    // Tuple with if-let
    BDD_WIP_TEST_CASE(if_let_with_tuple_pattern, test_if_let_with_tuple_pattern),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Tuple Literals", tuple_literals_test_cases)