#include "bdd_unit_common.h"
// Test scenarios for enum patterns

// Basic enum variant matching
void test_match_enum_variant_without_data(void) {
    const char* source = 
        "package test;\n"
        "pub enum Status {\n"
        "    Active,\n"
        "    Inactive\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let s: Status = Status.Active(none);\n"
        "    match s {\n"
        "        Status.Active(none) => { return 1; }\n"
        "        Status.Inactive(none) => { return 0; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match enum variant without data",
                               "enum_no_data.asthra",
                               source,
                               NULL,
                               1);
}

void test_match_enum_variant_with_single_value(void) {
    const char* source = 
        "package test;\n"
        "pub enum Result {\n"
        "    Ok(i32),\n"
        "    Err(i32)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let r: Result = Result.Ok(42);\n"
        "    match r {\n"
        "        Result.Ok(value) => { return value; }\n"
        "        Result.Err(code) => { return code; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match enum variant with single value",
                               "enum_single_value.asthra",
                               source,
                               NULL,
                               42);
}

void test_match_enum_variant_with_tuple_data(void) {
    const char* source = 
        "package test;\n"
        "pub enum Message {\n"
        "    Point(i32, i32),\n"
        "    Quit\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let msg: Message = Message.Point(10, 32);\n"
        "    match msg {\n"
        "        Message.Point(x, y) => { return x + y; }\n"
        "        Message.Quit => { return 0; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match enum variant with tuple data",
                               "enum_tuple_data.asthra",
                               source,
                               NULL,
                               42);
}

// Nested pattern matching
void test_match_nested_enum_patterns(void) {
    // Simplified test that works with current enum-as-integer implementation
    // TODO: Update this test when enums support associated data
    const char* source = 
        "package test;\n"
        "pub enum Direction {\n"
        "    North,\n"
        "    South,\n"
        "    East,\n"
        "    West\n"
        "}\n"
        "pub enum Action {\n"
        "    Move,\n"
        "    Stop,\n"
        "    Turn\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let dir: Direction = Direction.North;\n"
        "    let act: Action = Action.Move;\n"
        "    \n"
        "    // Nested matching on simple enums\n"
        "    match act {\n"
        "        Action.Move => {\n"
        "            match dir {\n"
        "                Direction.North => { return 42; }\n"
        "                Direction.South => { return 24; }\n"
        "                Direction.East => { return 12; }\n"
        "                Direction.West => { return 6; }\n"
        "            }\n"
        "        }\n"
        "        Action.Stop => { return 0; }\n"
        "        Action.Turn => { return -1; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match nested enum patterns",
                               "enum_nested.asthra",
                               source,
                               NULL,
                               42);
}

// If-let pattern matching
void test_if_let_with_enum_variant(void) {
    const char* source = 
        "package test;\n"
        "pub enum Option {\n"
        "    Some(i32),\n"
        "    None(none)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let opt: Option = Option.Some(42);\n"
        "    if let Option.Some(value) = opt {\n"
        "        return value;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("If-let with enum variant",
                               "if_let_enum.asthra",
                               source,
                               NULL,
                               42);
}

void test_if_let_with_enum_variant_returning_false_case(void) {
    const char* source = 
        "package test;\n"
        "pub enum Option {\n"
        "    Some(i32),\n"
        "    None(none)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let opt: Option = Option.None(none);\n"
        "    if let Option.Some(value) = opt {\n"
        "        return value;\n"
        "    } else {\n"
        "        return 42;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("If-let with enum variant returning false case",
                               "if_let_enum_else.asthra",
                               source,
                               NULL,
                               42);
}

// Pattern with wildcards
void test_match_with_wildcard_pattern(void) {
    const char* source = 
        "package test;\n"
        "pub enum Color {\n"
        "    Red(none),\n"
        "    Green(none),\n"
        "    Blue(none),\n"
        "    RGB(i32, i32, i32)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let c: Color = Color.RGB(42, 0, 0);\n"
        "    match c {\n"
        "        Color.RGB(r, _, _) => { return r; }\n"
        "        Color.Red(none) => { return 255; }\n"
        "        Color.Green(none) => { return 0; }\n"
        "        Color.Blue(none) => { return 0; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match with wildcard pattern",
                               "enum_wildcard.asthra",
                               source,
                               NULL,
                               42);
}

// Multiple patterns in single match arm
void test_match_multiple_patterns(void) {
    const char* source = 
        "package test;\n"
        "pub enum State {\n"
        "    Starting(none),\n"
        "    Running(none),\n"
        "    Stopping(none),\n"
        "    Stopped(none)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let s: State = State.Running(none);\n"
        "    match s {\n"
        "        State.Starting(none) => { return 1; }\n"
        "        State.Running(none) => { return 42; }\n"
        "        State.Stopping(none) => { return 2; }\n"
        "        State.Stopped(none) => { return 0; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match multiple patterns",
                               "enum_multiple.asthra",
                               source,
                               NULL,
                               42);
}

// Generic enum patterns
void test_match_generic_enum_variant(void) {
    const char* source = 
        "package test;\n"
        "pub enum Result<T, E> {\n"
        "    Ok(T),\n"
        "    Err(E)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let r: Result<i32, i32> = Result<i32, i32>.Ok(42);\n"
        "    match r {\n"
        "        Result.Ok(value) => { return value; }\n"
        "        Result.Err(error) => { return error; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match generic enum variant",
                               "enum_generic.asthra",
                               source,
                               NULL,
                               42);
}

// Enum construction
void test_construct_enum_variant_without_data(void) {
    const char* source = 
        "package test;\n"
        "pub enum Flag {\n"
        "    True(none),\n"
        "    False(none)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let f: Flag = Flag.True(none);\n"
        "    match f {\n"
        "        Flag.True(none) => { return 42; }\n"
        "        Flag.False(none) => { return 0; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Construct enum variant without data",
                               "enum_construct_no_data.asthra",
                               source,
                               NULL,
                               42);
}

void test_construct_enum_variant_with_data(void) {
    const char* source = 
        "package test;\n"
        "pub enum Container {\n"
        "    Empty(none),\n"
        "    Value(i32)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let c: Container = Container.Value(42);\n"
        "    match c {\n"
        "        Container.Empty(none) => { return 0; }\n"
        "        Container.Value(n) => { return n; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Construct enum variant with data",
                               "enum_construct_data.asthra",
                               source,
                               NULL,
                               42);
}

// Complex patterns
void test_match_with_variable_binding(void) {
    const char* source = 
        "package test;\n"
        "pub enum Command {\n"
        "    Move(i32, i32),\n"
        "    Stop(none)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let cmd: Command = Command.Move(20, 22);\n"
        "    match cmd {\n"
        "        Command.Move(dx, dy) => {\n"
        "            let sum: i32 = dx + dy;\n"
        "            return sum;\n"
        "        }\n"
        "        Command.Stop(none) => { return 0; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match with variable binding",
                               "enum_var_binding.asthra",
                               source,
                               NULL,
                               42);
}

void test_nested_match_expressions(void) {
    const char* source = 
        "package test;\n"
        "pub enum Outer {\n"
        "    A(i32),\n"
        "    B(i32)\n"
        "}\n"
        "pub enum Inner {\n"
        "    X(Outer),\n"
        "    Y(none)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let outer: Outer = Outer.A(42);\n"
        "    let inner: Inner = Inner.X(outer);\n"
        "    match inner {\n"
        "        Inner.X(o) => {\n"
        "            match o {\n"
        "                Outer.A(n) => { return n; }\n"
        "                Outer.B(n) => { return n; }\n"
        "            }\n"
        "        }\n"
        "        Inner.Y(none) => { return 0; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested match expressions",
                               "enum_nested_match.asthra",
                               source,
                               NULL,
                               42);
}

// Edge cases
void test_empty_enum_with_none_content(void) {
    const char* source = 
        "package test;\n"
        "pub enum Empty {\n"
        "    none\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty enum with none content",
                               "enum_empty.asthra",
                               source,
                               NULL,
                               42);
}

void test_match_with_all_patterns_covered(void) {
    const char* source = 
        "package test;\n"
        "pub enum Binary {\n"
        "    Zero(none),\n"
        "    One(none)\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let b: Binary = Binary.One(none);\n"
        "    match b {\n"
        "        Binary.Zero(none) => { return 0; }\n"
        "        Binary.One(none) => { return 42; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match with all patterns covered",
                               "enum_exhaustive.asthra",
                               source,
                               NULL,
                               42);
}

// Pattern matching in functions
void test_pattern_match_as_function_parameter(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub enum MyOption {\n"
        "    Some(i32),\n"
        "    None\n"
        "}\n"
        "\n"
        "pub fn unwrap(opt: MyOption) -> i32 {\n"
        "    match opt {\n"
        "        MyOption.Some(value) => { return value; }\n"
        "        MyOption.None => { return 0; }\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let o: MyOption = MyOption.Some(42);\n"
        "    return unwrap(o);\n"
        "}\n"
        "\n"
        "// Dummy function to work around parser EOF bug\n"
        "priv fn dummy(none) -> void { return (); }\n";
    
    bdd_run_execution_scenario("Pattern match as function parameter",
                               "enum_function_param.asthra",
                               source,
                               NULL,
                               42);
}

// Pattern matching with expressions
void test_match_expression_result(void) {
    const char* source = 
        "package test;\n"
        "pub enum Result {\n"
        "    Success(i32),\n"
        "    Failure(none)\n"
        "}\n"
        "pub fn compute(none) -> Result {\n"
        "    return Result.Success(42);\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    match compute(none) {\n"
        "        Result.Success(n) => { return n; }\n"
        "        Result.Failure(none) => { return 0; }\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match expression result",
                               "enum_expr_result.asthra",
                               source,
                               NULL,
                               42);
}

// Define test cases
BDD_DECLARE_TEST_CASES(enum_patterns)
    // Basic enum variant matching
    BDD_WIP_TEST_CASE(match_enum_variant_without_data, test_match_enum_variant_without_data),
    BDD_WIP_TEST_CASE(match_enum_variant_with_single_value, test_match_enum_variant_with_single_value),
    BDD_WIP_TEST_CASE(match_enum_variant_with_tuple_data, test_match_enum_variant_with_tuple_data),
    
    // Nested pattern matching
    BDD_WIP_TEST_CASE(match_nested_enum_patterns, test_match_nested_enum_patterns),
    
    // If-let pattern matching
    BDD_WIP_TEST_CASE(if_let_with_enum_variant, test_if_let_with_enum_variant),
    BDD_WIP_TEST_CASE(if_let_with_enum_variant_returning_false_case, test_if_let_with_enum_variant_returning_false_case),
    
    // Pattern with wildcards
    BDD_WIP_TEST_CASE(match_with_wildcard_pattern, test_match_with_wildcard_pattern),
    
    // Multiple patterns
    BDD_WIP_TEST_CASE(match_multiple_patterns, test_match_multiple_patterns),
    
    // Generic enum patterns
    BDD_WIP_TEST_CASE(match_generic_enum_variant, test_match_generic_enum_variant),
    
    // Enum construction
    BDD_WIP_TEST_CASE(construct_enum_variant_without_data, test_construct_enum_variant_without_data),
    BDD_WIP_TEST_CASE(construct_enum_variant_with_data, test_construct_enum_variant_with_data),
    
    // Complex patterns
    BDD_WIP_TEST_CASE(match_with_variable_binding, test_match_with_variable_binding),
    BDD_WIP_TEST_CASE(nested_match_expressions, test_nested_match_expressions),
    
    // Edge cases
    BDD_WIP_TEST_CASE(empty_enum_with_none_content, test_empty_enum_with_none_content),
    BDD_WIP_TEST_CASE(match_with_all_patterns_covered, test_match_with_all_patterns_covered),
    
    // Pattern matching in functions
    BDD_WIP_TEST_CASE(pattern_match_as_function_parameter, test_pattern_match_as_function_parameter),
    
    // Pattern matching with expressions
    BDD_WIP_TEST_CASE(match_expression_result, test_match_expression_result),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Enum Patterns", enum_patterns_test_cases)