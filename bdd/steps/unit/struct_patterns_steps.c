#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <sys/stat.h>

// Test scenarios for struct patterns

// Basic struct destructuring
void test_match_struct_with_all_fields(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "  x: i32,\n"
        "  y: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let p: Point = Point { x: 42, y: 100 };\n"
        "  match p {\n"
        "    Point { x: a, y: b } => { return a; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match struct with all fields",
                               "struct_all_fields.asthra",
                               source,
                               NULL,
                               42);
}

void test_match_struct_with_field_reordering(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "  x: i32,\n"
        "  y: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let p: Point = Point { x: 10, y: 32 };\n"
        "  match p {\n"
        "    Point { y: b, x: a } => { return a + b; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match struct with field reordering",
                               "struct_reorder.asthra",
                               source,
                               NULL,
                               42);
}

void test_match_struct_with_wildcard_fields(void) {
    const char* source = 
        "package test;\n"
        "pub struct Rectangle {\n"
        "  width: i32,\n"
        "  height: i32,\n"
        "  color: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let r: Rectangle = Rectangle { width: 42, height: 20, color: 255 };\n"
        "  match r {\n"
        "    Rectangle { width: w, height: _, color: _ } => { return w; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match struct with wildcard fields",
                               "struct_wildcard.asthra",
                               source,
                               NULL,
                               42);
}

// If-let struct patterns
void test_if_let_with_struct_pattern(void) {
    const char* source = 
        "package test;\n"
        "pub struct Config {\n"
        "  value: i32,\n"
        "  enabled: bool\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let c: Config = Config { value: 42, enabled: true };\n"
        "  if let Config { value: v, enabled: _ } = c {\n"
        "    return v;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("If-let with struct pattern",
                               "if_let_struct.asthra",
                               source,
                               NULL,
                               42);
}

// Nested struct patterns
void test_match_nested_struct_patterns(void) {
    const char* source = 
        "package test;\n"
        "pub struct Inner {\n"
        "  value: i32\n"
        "}\n"
        "pub struct Outer {\n"
        "  inner: Inner,\n"
        "  count: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let i: Inner = Inner { value: 40 };\n"
        "  let o: Outer = Outer { inner: i, count: 2 };\n"
        "  match o {\n"
        "    Outer { inner: Inner { value: v }, count: c } => { return v + c; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match nested struct patterns",
                               "struct_nested.asthra",
                               source,
                               NULL,
                               42);
}

// Generic struct patterns
void test_match_generic_struct_pattern(void) {
    const char* source = 
        "package test;\n"
        "pub struct Container<T> {\n"
        "  value: T\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let c: Container<i32> = Container<i32> { value: 42 };\n"
        "  match c {\n"
        "    Container<i32> { value: v } => { return v; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match generic struct pattern",
                               "struct_generic.asthra",
                               source,
                               NULL,
                               42);
}

// Complex field bindings
void test_match_struct_with_same_field_and_binding_names(void) {
    const char* source = 
        "package test;\n"
        "pub struct Data {\n"
        "  value: i32,\n"
        "  flag: bool\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let d: Data = Data { value: 42, flag: false };\n"
        "  match d {\n"
        "    Data { value: value, flag: flag } => { \n"
        "      if flag {\n"
        "        return 0;\n"
        "      } else {\n"
        "        return value;\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match struct with same field and binding names",
                               "struct_same_names.asthra",
                               source,
                               NULL,
                               42);
}

void test_match_struct_with_renamed_bindings(void) {
    const char* source = 
        "package test;\n"
        "pub struct Coordinate {\n"
        "  x: i32,\n"
        "  y: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let coord: Coordinate = Coordinate { x: 20, y: 22 };\n"
        "  match coord {\n"
        "    Coordinate { x: horizontal, y: vertical } => { return horizontal + vertical; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match struct with renamed bindings",
                               "struct_renamed.asthra",
                               source,
                               NULL,
                               42);
}

// Multiple patterns
void test_match_multiple_struct_patterns(void) {
    const char* source = 
        "package test;\n"
        "pub struct Status {\n"
        "  code: i32,\n"
        "  active: bool\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: Status = Status { code: 42, active: true };\n"
        "  match s {\n"
        "    Status { code: 0, active: _ } => { return 0; }\n"
        "    Status { code: c, active: true } => { return c; }\n"
        "    Status { code: _, active: false } => { return -1; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match multiple struct patterns",
                               "struct_multiple.asthra",
                               source,
                               NULL,
                               42);
}

// Struct patterns in functions
void test_struct_pattern_in_function_parameter(void) {
    const char* source = 
        "package test;\n"
        "pub struct Pair {\n"
        "  first: i32,\n"
        "  second: i32\n"
        "}\n"
        "pub fn get_first(p: Pair) -> i32 {\n"
        "  match p {\n"
        "    Pair { first: f, second: _ } => { return f; }\n"
        "  }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let pair: Pair = Pair { first: 42, second: 100 };\n"
        "  return get_first(pair);\n"
        "}\n";
    
    bdd_run_execution_scenario("Struct pattern in function parameter",
                               "struct_func_param.asthra",
                               source,
                               NULL,
                               42);
}

// Empty struct patterns
void test_match_empty_struct(void) {
    const char* source = 
        "package test;\n"
        "pub struct Empty {\n"
        "  none\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let e: Empty = Empty { none };\n"
        "  match e {\n"
        "    Empty { none } => { return 42; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match empty struct",
                               "struct_empty.asthra",
                               source,
                               NULL,
                               42);
}

// Combined patterns
void test_match_struct_with_tuple_fields(void) {
    const char* source = 
        "package test;\n"
        "pub struct Complex {\n"
        "  pair: (i32, i32),\n"
        "  single: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let c: Complex = Complex { pair: (20, 22), single: 0 };\n"
        "  match c {\n"
        "    Complex { pair: (a, b), single: _ } => { return a + b; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match struct with tuple fields",
                               "struct_tuple_fields.asthra",
                               source,
                               NULL,
                               42);
}

// Pattern matching expressions
void test_struct_pattern_in_match_expression(void) {
    const char* source = 
        "package test;\n"
        "pub struct Result {\n"
        "  value: i32,\n"
        "  success: bool\n"
        "}\n"
        "pub fn compute(none) -> Result {\n"
        "  return Result { value: 42, success: true };\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  match compute(none) {\n"
        "    Result { value: v, success: true } => { return v; }\n"
        "    Result { value: _, success: false } => { return 0; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Struct pattern in match expression",
                               "struct_match_expr.asthra",
                               source,
                               NULL,
                               42);
}

// Edge cases
void test_match_struct_with_single_field(void) {
    const char* source = 
        "package test;\n"
        "pub struct Wrapper {\n"
        "  data: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let w: Wrapper = Wrapper { data: 42 };\n"
        "  match w {\n"
        "    Wrapper { data: d } => { return d; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match struct with single field",
                               "struct_single_field.asthra",
                               source,
                               NULL,
                               42);
}

void test_nested_if_let_struct_patterns(void) {
    const char* source = 
        "package test;\n"
        "pub struct First {\n"
        "  value: i32\n"
        "}\n"
        "pub struct Second {\n"
        "  data: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let f: First = First { value: 42 };\n"
        "  if let First { value: v } = f {\n"
        "    let s: Second = Second { data: v };\n"
        "    if let Second { data: d } = s {\n"
        "      return d;\n"
        "    } else {\n"
        "      return 0;\n"
        "    }\n"
        "  } else {\n"
        "    return -1;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested if-let struct patterns",
                               "struct_nested_if_let.asthra",
                               source,
                               NULL,
                               42);
}

// Define test cases
BddTestCase struct_patterns_test_cases[] = {
    // Basic struct destructuring
    BDD_WIP_TEST_CASE(match_struct_with_all_fields, test_match_struct_with_all_fields),
    BDD_WIP_TEST_CASE(match_struct_with_field_reordering, test_match_struct_with_field_reordering),
    BDD_WIP_TEST_CASE(match_struct_with_wildcard_fields, test_match_struct_with_wildcard_fields),
    
    // If-let struct patterns
    BDD_WIP_TEST_CASE(if_let_with_struct_pattern, test_if_let_with_struct_pattern),
    
    // Nested struct patterns
    BDD_WIP_TEST_CASE(match_nested_struct_patterns, test_match_nested_struct_patterns),
    
    // Generic struct patterns
    BDD_WIP_TEST_CASE(match_generic_struct_pattern, test_match_generic_struct_pattern),
    
    // Complex field bindings
    BDD_WIP_TEST_CASE(match_struct_with_same_field_and_binding_names, test_match_struct_with_same_field_and_binding_names),
    BDD_WIP_TEST_CASE(match_struct_with_renamed_bindings, test_match_struct_with_renamed_bindings),
    
    // Multiple patterns
    BDD_WIP_TEST_CASE(match_multiple_struct_patterns, test_match_multiple_struct_patterns),
    
    // Struct patterns in functions
    BDD_WIP_TEST_CASE(struct_pattern_in_function_parameter, test_struct_pattern_in_function_parameter),
    
    // Empty struct patterns
    BDD_WIP_TEST_CASE(match_empty_struct, test_match_empty_struct),
    
    // Combined patterns
    BDD_WIP_TEST_CASE(match_struct_with_tuple_fields, test_match_struct_with_tuple_fields),
    
    // Pattern matching expressions
    BDD_WIP_TEST_CASE(struct_pattern_in_match_expression, test_struct_pattern_in_match_expression),
    
    // Edge cases
    BDD_WIP_TEST_CASE(match_struct_with_single_field, test_match_struct_with_single_field),
    BDD_WIP_TEST_CASE(nested_if_let_struct_patterns, test_nested_if_let_struct_patterns),
};

// Main test runner
int main(void) {
    return bdd_run_test_suite("Struct Patterns",
                              struct_patterns_test_cases,
                              sizeof(struct_patterns_test_cases) / sizeof(struct_patterns_test_cases[0]),
                              bdd_cleanup_temp_files);
}