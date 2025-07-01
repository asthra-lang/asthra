#include "bdd_unit_common.h"
// Test scenarios for pattern bindings

// Simple identifier patterns
void test_bind_variable_in_match_arm(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let value: i32 = 42;\n"
        "  match value {\n"
        "    x => { return x; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind variable in match arm",
                               "bind_match_arm.asthra",
                               source,
                               NULL,
                               42);
}

void test_bind_multiple_variables_in_match(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let pair: (i32, i32) = (20, 22);\n"
        "  match pair {\n"
        "    (a, b) => { return a + b; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind multiple variables in match",
                               "bind_multiple.asthra",
                               source,
                               NULL,
                               42);
}

// Wildcard patterns
void test_use_wildcard_to_ignore_values(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let triple: (i32, i32, i32) = (42, 100, 200);\n"
        "  match triple {\n"
        "    (x, _, _) => { return x; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Use wildcard to ignore values",
                               "wildcard_ignore.asthra",
                               source,
                               NULL,
                               42);
}

// Nested pattern bindings
void test_bind_variables_in_nested_patterns(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let nested: ((i32, i32), i32) = ((20, 22), 0);\n"
        "  match nested {\n"
        "    ((x, y), _) => { return x + y; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind variables in nested patterns",
                               "nested_bindings.asthra",
                               source,
                               NULL,
                               42);
}

// Struct pattern bindings
void test_bind_struct_fields_to_variables(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "  x: i32,\n"
        "  y: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let p: Point = Point { x: 20, y: 22 };\n"
        "  match p {\n"
        "    Point { x: a, y: b } => { return a + b; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind struct fields to variables",
                               "struct_field_bindings.asthra",
                               source,
                               NULL,
                               42);
}

void test_mix_bindings_and_wildcards_in_struct_pattern(void) {
    const char* source = 
        "package test;\n"
        "pub struct Data {\n"
        "  value: i32,\n"
        "  flag: bool,\n"
        "  count: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let d: Data = Data { value: 42, flag: true, count: 100 };\n"
        "  match d {\n"
        "    Data { value: v, flag: _, count: _ } => { return v; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Mix bindings and wildcards in struct pattern",
                               "struct_mixed_bindings.asthra",
                               source,
                               NULL,
                               42);
}

// Enum pattern bindings
void test_bind_enum_variant_data(void) {
    const char* source = 
        "package test;\n"
        "pub enum Result {\n"
        "  Ok(i32),\n"
        "  Err(i32)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let r: Result = Result.Ok(42);\n"
        "  match r {\n"
        "    Result.Ok(value) => { return value; }\n"
        "    Result.Err(code) => { return code; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind enum variant data",
                               "enum_variant_binding.asthra",
                               source,
                               NULL,
                               42);
}

void test_bind_multiple_values_from_enum_variant(void) {
    const char* source = 
        "package test;\n"
        "pub enum Message {\n"
        "  Move(i32, i32),\n"
        "  Quit(none)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let msg: Message = Message.Move(20, 22);\n"
        "  match msg {\n"
        "    Message.Move(dx, dy) => { return dx + dy; }\n"
        "    Message.Quit(none) => { return 0; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind multiple values from enum variant",
                               "enum_multiple_bindings.asthra",
                               source,
                               NULL,
                               42);
}

// If-let pattern bindings
void test_bind_variable_in_if_let(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let value: i32 = 42;\n"
        "  if let x = value {\n"
        "    return x;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind variable in if-let",
                               "if_let_binding.asthra",
                               source,
                               NULL,
                               42);
}

void test_bind_tuple_elements_in_if_let(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let pair: (i32, i32) = (40, 2);\n"
        "  if let (a, b) = pair {\n"
        "    return a + b;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind tuple elements in if-let",
                               "if_let_tuple_binding.asthra",
                               source,
                               NULL,
                               42);
}

// Shadowing in patterns
void test_shadow_existing_variables_in_pattern(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let x: i32 = 10;\n"
        "  let pair: (i32, i32) = (20, 22);\n"
        "  match pair {\n"
        "    (x, y) => { return x + y; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Shadow existing variables in pattern",
                               "pattern_shadowing.asthra",
                               source,
                               NULL,
                               42);
}

// Complex nested bindings
void test_bind_in_deeply_nested_structure(void) {
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
    
    bdd_run_execution_scenario("Bind in deeply nested structure",
                               "deeply_nested_binding.asthra",
                               source,
                               NULL,
                               42);
}

// Pattern bindings with literals
void test_mix_literal_matching_and_bindings(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let triple: (i32, i32, i32) = (1, 42, 3);\n"
        "  match triple {\n"
        "    (1, x, 3) => { return x; }\n"
        "    (_, x, _) => { return 0; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Mix literal matching and bindings",
                               "literal_and_binding.asthra",
                               source,
                               NULL,
                               42);
}

// Multiple bindings in single pattern
void test_bind_all_elements_of_a_tuple(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let quad: (i32, i32, i32, i32) = (10, 11, 12, 9);\n"
        "  match quad {\n"
        "    (a, b, c, d) => { return a + b + c + d; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind all elements of a tuple",
                               "bind_all_elements.asthra",
                               source,
                               NULL,
                               42);
}

// Pattern bindings in variable declarations
void test_destructure_in_let_binding(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let pair: (i32, i32) = (20, 22);\n"
        "  let (x, y): (i32, i32) = pair;\n"
        "  return x + y;\n"
        "}\n";
    
    bdd_run_execution_scenario("Destructure in let binding",
                               "let_destructure.asthra",
                               source,
                               NULL,
                               42);
}

// Mutable bindings in patterns
void test_bind_mutable_variables_in_pattern(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let mut pair: (i32, i32) = (20, 20);\n"
        "  match pair {\n"
        "    (a, b) => {\n"
        "      pair = (a + 1, b + 1);\n"
        "    }\n"
        "  }\n"
        "  match pair {\n"
        "    (x, y) => { return x + y; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind mutable variables in pattern",
                               "mutable_binding.asthra",
                               source,
                               NULL,
                               42);
}

// Pattern bindings with Option type
void test_bind_option_value(void) {
    const char* source = 
        "package test;\n"
        "pub enum Option<T> {\n"
        "  Some(T),\n"
        "  None(none)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let opt: Option<i32> = Option<i32>.Some(42);\n"
        "  match opt {\n"
        "    Option.Some(value) => { return value; }\n"
        "    Option.None(none) => { return 0; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind Option value",
                               "option_binding.asthra",
                               source,
                               NULL,
                               42);
}

// Pattern bindings with Result type
void test_bind_result_values(void) {
    const char* source = 
        "package test;\n"
        "pub enum Result<T, E> {\n"
        "  Ok(T),\n"
        "  Err(E)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let res: Result<i32, i32> = Result<i32, i32>.Ok(42);\n"
        "  match res {\n"
        "    Result.Ok(val) => { return val; }\n"
        "    Result.Err(err) => { return err; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind Result values",
                               "result_binding.asthra",
                               source,
                               NULL,
                               42);
}

// Binding with type annotations
void test_pattern_binding_preserves_types(void) {
    const char* source = 
        "package test;\n"
        "pub fn process(p: (i32, bool)) -> i32 {\n"
        "  match p {\n"
        "    (num, true) => { return num; }\n"
        "    (num, false) => { return -num; }\n"
        "  }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return process((42, true));\n"
        "}\n";
    
    bdd_run_execution_scenario("Pattern binding preserves types",
                               "binding_types.asthra",
                               source,
                               NULL,
                               42);
}

// Complex enum with struct patterns
void test_bind_from_enum_containing_structs(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "  x: i32,\n"
        "  y: i32\n"
        "}\n"
        "pub enum Shape {\n"
        "  Circle(i32),\n"
        "  Rectangle(Point)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  let p: Point = Point { x: 20, y: 22 };\n"
        "  let shape: Shape = Shape.Rectangle(p);\n"
        "  match shape {\n"
        "    Shape.Circle(radius) => { return radius; }\n"
        "    Shape.Rectangle(Point { x: a, y: b }) => { return a + b; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Bind from enum containing structs",
                               "enum_struct_binding.asthra",
                               source,
                               NULL,
                               42);
}

// Define test cases
BDD_DECLARE_TEST_CASES(pattern_bindings)
    // Simple identifier patterns
    BDD_WIP_TEST_CASE(bind_variable_in_match_arm, test_bind_variable_in_match_arm),
    BDD_WIP_TEST_CASE(bind_multiple_variables_in_match, test_bind_multiple_variables_in_match),
    
    // Wildcard patterns
    BDD_WIP_TEST_CASE(use_wildcard_to_ignore_values, test_use_wildcard_to_ignore_values),
    
    // Nested pattern bindings
    BDD_WIP_TEST_CASE(bind_variables_in_nested_patterns, test_bind_variables_in_nested_patterns),
    
    // Struct pattern bindings
    BDD_WIP_TEST_CASE(bind_struct_fields_to_variables, test_bind_struct_fields_to_variables),
    BDD_WIP_TEST_CASE(mix_bindings_and_wildcards_in_struct_pattern, test_mix_bindings_and_wildcards_in_struct_pattern),
    
    // Enum pattern bindings
    BDD_WIP_TEST_CASE(bind_enum_variant_data, test_bind_enum_variant_data),
    BDD_WIP_TEST_CASE(bind_multiple_values_from_enum_variant, test_bind_multiple_values_from_enum_variant),
    
    // If-let pattern bindings
    BDD_WIP_TEST_CASE(bind_variable_in_if_let, test_bind_variable_in_if_let),
    BDD_WIP_TEST_CASE(bind_tuple_elements_in_if_let, test_bind_tuple_elements_in_if_let),
    
    // Shadowing in patterns
    BDD_WIP_TEST_CASE(shadow_existing_variables_in_pattern, test_shadow_existing_variables_in_pattern),
    
    // Complex nested bindings
    BDD_WIP_TEST_CASE(bind_in_deeply_nested_structure, test_bind_in_deeply_nested_structure),
    
    // Pattern bindings with literals
    BDD_WIP_TEST_CASE(mix_literal_matching_and_bindings, test_mix_literal_matching_and_bindings),
    
    // Multiple bindings in single pattern
    BDD_WIP_TEST_CASE(bind_all_elements_of_a_tuple, test_bind_all_elements_of_a_tuple),
    
    // Pattern bindings in variable declarations
    BDD_WIP_TEST_CASE(destructure_in_let_binding, test_destructure_in_let_binding),
    
    // Mutable bindings in patterns
    BDD_WIP_TEST_CASE(bind_mutable_variables_in_pattern, test_bind_mutable_variables_in_pattern),
    
    // Pattern bindings with Option type
    BDD_WIP_TEST_CASE(bind_option_value, test_bind_option_value),
    
    // Pattern bindings with Result type
    BDD_WIP_TEST_CASE(bind_result_values, test_bind_result_values),
    
    // Binding with type annotations
    BDD_WIP_TEST_CASE(pattern_binding_preserves_types, test_pattern_binding_preserves_types),
    
    // Complex enum with struct patterns
    BDD_WIP_TEST_CASE(bind_from_enum_containing_structs, test_bind_from_enum_containing_structs),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Pattern Bindings", pattern_bindings_test_cases)