#include "bdd_unit_common.h"
// Test scenarios using the new reusable framework

void test_public_function(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Public function\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Public function declaration",
                               "public_function.asthra",
                               source,
                               "Public function",
                               0);
}

void test_private_function(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "priv fn helper(none) -> void {\n"
        "    log(\"Private helper\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    helper();\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Private function declaration",
                               "private_function.asthra",
                               source,
                               "Private helper",
                               0);
}

void test_public_struct(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: Point = Point { x: 10, y: 20 };\n"
        "    log(\"Public struct works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Public struct declaration",
                               "public_struct.asthra",
                               source,
                               "Public struct works",
                               0);
}

void test_private_struct(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "priv struct InternalData {\n"
        "    value: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let data: InternalData = InternalData { value: 42 };\n"
        "    log(\"Private struct works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Private struct declaration",
                               "private_struct.asthra",
                               source,
                               "Private struct works",
                               0);
}

void test_public_enum(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub enum Status {\n"
        "    Ok,\n"
        "    Error(i32)\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let s: Status = Status.Ok;\n"
        "    log(\"Public enum works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Public enum declaration",
                               "public_enum.asthra",
                               source,
                               "Public enum works",
                               0);
}

void test_private_enum(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "priv enum InternalState {\n"
        "    Ready,\n"
        "    Busy,\n"
        "    Done\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let state: InternalState = InternalState.Ready;\n"
        "    log(\"Private enum works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Private enum declaration",
                               "private_enum.asthra",
                               source,
                               "Private enum works",
                               0);
}

void test_mixed_field_visibility(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct User {\n"
        "    pub name: string,\n"
        "    priv id: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let u: User = User { name: \"Alice\", id: 123 };\n"
        "    log(\"Mixed visibility works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Struct with mixed field visibility",
                               "mixed_field_visibility.asthra",
                               source,
                               "Mixed visibility works",
                               0);
}

void test_no_visibility_function(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "fn helper(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    helper();\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Missing visibility modifier on function",
                                 "no_visibility_function.asthra",
                                 source,
                                 0,  // should fail
                                 "expected visibility modifier");
}

void test_no_visibility_struct(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"No visibility\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Missing visibility modifier on struct",
                                 "no_visibility_struct.asthra",
                                 source,
                                 0,  // should fail
                                 "expected visibility modifier");
}

void test_no_visibility_enum(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "enum Status {\n"
        "    Ok,\n"
        "    Error\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"No visibility\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Missing visibility modifier on enum",
                                 "no_visibility_enum.asthra",
                                 source,
                                 0,  // should fail
                                 "expected visibility modifier");
}

void test_public_const(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub const MAX_SIZE: i32 = 100;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Public const works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Public constant declaration",
                               "public_const.asthra",
                               source,
                               "Public const works",
                               0);
}

void test_private_const(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "priv const INTERNAL_BUFFER_SIZE: i32 = 1024;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Private const works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Private constant declaration",
                               "private_const.asthra",
                               source,
                               "Private const works",
                               0);
}

// Define test cases using the new framework - based on original @wip configuration
BDD_DECLARE_TEST_CASES(visibility)
    BDD_TEST_CASE(public_function, test_public_function),
    BDD_TEST_CASE(private_function, test_private_function),
    BDD_WIP_TEST_CASE(public_struct, test_public_struct),
    BDD_WIP_TEST_CASE(private_struct, test_private_struct),
    BDD_WIP_TEST_CASE(public_enum, test_public_enum),
    BDD_WIP_TEST_CASE(private_enum, test_private_enum),
    BDD_TEST_CASE(mixed_field_visibility, test_mixed_field_visibility),
    BDD_TEST_CASE(no_visibility_function, test_no_visibility_function),
    BDD_WIP_TEST_CASE(no_visibility_struct, test_no_visibility_struct),
    BDD_WIP_TEST_CASE(no_visibility_enum, test_no_visibility_enum),
    BDD_TEST_CASE(public_const, test_public_const),
    BDD_TEST_CASE(private_const, test_private_const),
BDD_END_TEST_CASES()

// Main test runner using the new framework
BDD_UNIT_TEST_MAIN("Visibility Modifiers", visibility_test_cases)