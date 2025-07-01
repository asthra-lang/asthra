#include "bdd_unit_common.h"
// Test scenarios for mutability system

void test_immutable_by_default(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 42;\n"
        "    log(\"Immutable variable works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Variables are immutable by default",
                               "immutable_by_default.asthra",
                               source,
                               "Immutable variable works",
                               0);
}

void test_cannot_reassign_immutable(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 42;\n"
        "    x = 100;\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Cannot reassign immutable variable",
                                 "cannot_reassign_immutable.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "immutable");
}

void test_mutable_variable(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut x: i32 = 42;\n"
        "    x = 100;\n"
        "    log(\"Mutable variable works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Mutable variable declaration",
                               "mutable_variable.asthra",
                               source,
                               "Mutable variable works",
                               0);
}

void test_multiple_reassignments(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut count: i32 = 0;\n"
        "    count = count + 1;\n"
        "    count = count + 1;\n"
        "    count = count + 1;\n"
        "    log(\"Multiple reassignments work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple reassignments to mutable variable",
                               "multiple_reassignments.asthra",
                               source,
                               "Multiple reassignments work",
                               0);
}

void test_immutable_parameters(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn process(value: i32) -> void {\n"
        "    value = 100;\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    process(42);\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Function parameters are immutable",
                                 "immutable_parameters.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "parameter");
}

void test_mut_parameter_error(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn process(mut value: i32) -> void {\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    process(42);\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Cannot use mut on function parameters",
                                 "mut_parameter_error.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "parameter");
}

void test_mutable_struct_fields(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Point {\n"
        "    pub x: i32,\n"
        "    pub y: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut p: Point = Point { x: 10, y: 20 };\n"
        "    p.x = 30;\n"
        "    p.y = 40;\n"
        "    log(\"Mutable struct fields work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Mutable struct allows field modification",
                               "mutable_struct_fields.asthra",
                               source,
                               "Mutable struct fields work",
                               0);
}

void test_immutable_struct_fields(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Point {\n"
        "    pub x: i32,\n"
        "    pub y: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: Point = Point { x: 10, y: 20 };\n"
        "    p.x = 30;\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Immutable struct prevents field modification",
                                 "immutable_struct_fields.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "immutable");
}

void test_mutable_array_elements(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut arr: [3]i32 = [1, 2, 3];\n"
        "    arr[0] = 10;\n"
        "    arr[1] = 20;\n"
        "    arr[2] = 30;\n"
        "    log(\"Mutable array elements work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Mutable array allows element modification",
                               "mutable_array_elements.asthra",
                               source,
                               "Mutable array elements work",
                               0);
}

void test_immutable_array_elements(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let arr: [3]i32 = [1, 2, 3];\n"
        "    arr[0] = 10;\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Immutable array prevents element modification",
                                 "immutable_array_elements.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "immutable");
}

void test_nested_mutability(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Inner {\n"
        "    pub value: i32\n"
        "}\n"
        "\n"
        "pub struct Outer {\n"
        "    pub inner: Inner\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut o: Outer = Outer { inner: Inner { value: 42 } };\n"
        "    o.inner.value = 100;\n"
        "    log(\"Nested mutability works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested struct mutability",
                               "nested_mutability.asthra",
                               source,
                               "Nested mutability works",
                               0);
}

void test_mutable_pointer(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut x: i32 = 42;\n"
        "    let ptr: *mut i32 = &x;\n"
        "    unsafe {\n"
        "        *ptr = 100;\n"
        "    }\n"
        "    log(\"Mutable pointer works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Mutable pointer allows dereferenced assignment",
                               "mutable_pointer.asthra",
                               source,
                               "Mutable pointer works",
                               0);
}

void test_const_pointer(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 42;\n"
        "    let ptr: *const i32 = &x;\n"
        "    unsafe {\n"
        "        *ptr = 100;\n"
        "    }\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Const pointer prevents dereferenced assignment",
                                 "const_pointer.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "const");
}

void test_variable_shadowing(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 42;\n"
        "    let x: i32 = 100;\n"
        "    log(\"Variable shadowing works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Immutable variable can be shadowed",
                               "variable_shadowing.asthra",
                               source,
                               "Variable shadowing works",
                               0);
}

void test_shadowing_mutability_change(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = 42;\n"
        "    let mut x: i32 = x;\n"
        "    x = 100;\n"
        "    log(\"Mutability change via shadowing works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Shadowing can change mutability",
                               "shadowing_mutability_change.asthra",
                               source,
                               "Mutability change via shadowing works",
                               0);
}

void test_compound_assignments(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut sum: i32 = 0;\n"
        "    sum = sum + 10;\n"
        "    sum = sum * 2;\n"
        "    sum = sum - 5;\n"
        "    log(\"Compound assignments work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Compound assignment to mutable variable",
                               "compound_assignments.asthra",
                               source,
                               "Compound assignments work",
                               0);
}

void test_immutable_self(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Counter {\n"
        "    pub value: i32\n"
        "}\n"
        "\n"
        "impl Counter {\n"
        "    pub fn try_modify(self) -> void {\n"
        "        self.value = 100;\n"
        "        return ();\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let c: Counter = Counter { value: 0 };\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Method self parameter is immutable",
                                 "immutable_self.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "self");
}

// Define test cases
BDD_DECLARE_TEST_CASES(mutability_system)
    BDD_TEST_CASE(immutable_by_default, test_immutable_by_default),
    BDD_TEST_CASE(cannot_reassign_immutable, test_cannot_reassign_immutable),
    BDD_TEST_CASE(mutable_variable, test_mutable_variable),
    BDD_TEST_CASE(multiple_reassignments, test_multiple_reassignments),
    BDD_TEST_CASE(immutable_parameters, test_immutable_parameters),
    BDD_TEST_CASE(mut_parameter_error, test_mut_parameter_error),
    BDD_WIP_TEST_CASE(mutable_struct_fields, test_mutable_struct_fields),
    BDD_TEST_CASE(immutable_struct_fields, test_immutable_struct_fields),
    BDD_WIP_TEST_CASE(mutable_array_elements, test_mutable_array_elements),
    BDD_TEST_CASE(immutable_array_elements, test_immutable_array_elements),
    BDD_WIP_TEST_CASE(nested_mutability, test_nested_mutability),
    BDD_TEST_CASE(mutable_pointer, test_mutable_pointer),
    BDD_TEST_CASE(const_pointer, test_const_pointer),
    BDD_WIP_TEST_CASE(variable_shadowing, test_variable_shadowing),
    BDD_WIP_TEST_CASE(shadowing_mutability_change, test_shadowing_mutability_change),
    BDD_TEST_CASE(compound_assignments, test_compound_assignments),
    BDD_TEST_CASE(immutable_self, test_immutable_self),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Mutability System", mutability_system_test_cases)