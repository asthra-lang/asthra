#include "bdd_unit_common.h"
// Test scenarios for control flow loops

void test_simple_for_range(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(3) {\n"
        "        log(\"Iteration\");\n"
        "    }\n"
        "    log(\"For loop with range works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple for loop with range",
                               "simple_for_range.asthra",
                               source,
                               "For loop with range works",
                               0);
}

void test_range_start_end(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(5, 8) {\n"
        "        log(\"Counting\");\n"
        "    }\n"
        "    log(\"Range with start/end works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("For loop with range start and end",
                               "range_start_end.asthra",
                               source,
                               "Range with start/end works",
                               0);
}

void test_array_iteration(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let arr: [5]i32 = [10, 20, 30, 40, 50];\n"
        "    for elem in arr[:] {\n"
        "        log(\"Element\");\n"
        "    }\n"
        "    log(\"Array iteration works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("For loop over array slice",
                               "array_iteration.asthra",
                               source,
                               "Array iteration works",
                               0);
}

void test_for_with_break(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(10) {\n"
        "        if i == 3 {\n"
        "            break;\n"
        "        }\n"
        "        log(\"Before break\");\n"
        "    }\n"
        "    log(\"Break statement works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("For loop with break",
                               "for_with_break.asthra",
                               source,
                               "Break statement works",
                               0);
}

void test_nested_break(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(3) {\n"
        "        for j in range(3) {\n"
        "            if j == 1 {\n"
        "                break;\n"
        "            }\n"
        "            log(\"Inner\");\n"
        "        }\n"
        "        log(\"Outer\");\n"
        "    }\n"
        "    log(\"Nested break works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested loops with break",
                               "nested_break.asthra",
                               source,
                               "Nested break works",
                               0);
}

void test_for_with_continue(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(5) {\n"
        "        if i == 2 {\n"
        "            continue;\n"
        "        }\n"
        "        log(\"Not skipped\");\n"
        "    }\n"
        "    log(\"Continue statement works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("For loop with continue",
                               "for_with_continue.asthra",
                               source,
                               "Continue statement works",
                               0);
}

void test_nested_continue(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(2) {\n"
        "        for j in range(3) {\n"
        "            if j == 1 {\n"
        "                continue;\n"
        "            }\n"
        "            log(\"Inner iteration\");\n"
        "        }\n"
        "        log(\"Outer iteration\");\n"
        "    }\n"
        "    log(\"Nested continue works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested loops with continue",
                               "nested_continue.asthra",
                               source,
                               "Nested continue works",
                               0);
}

void test_break_and_continue(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(10) {\n"
        "        if i == 2 {\n"
        "            continue;\n"
        "        }\n"
        "        if i == 5 {\n"
        "            break;\n"
        "        }\n"
        "        log(\"Processing\");\n"
        "    }\n"
        "    log(\"Break and continue work together\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Loop with break and continue",
                               "break_and_continue.asthra",
                               source,
                               "Break and continue work together",
                               0);
}

void test_immutable_loop_var(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(3) {\n"
        "        i = 10;\n"
        "        log(\"Should not compile\");\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Loop variable is immutable",
                                 "immutable_loop_var.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "immutable");
}

void test_empty_range(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(0) {\n"
        "        log(\"Should not execute\");\n"
        "    }\n"
        "    log(\"Empty range works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty range loop",
                               "empty_range.asthra",
                               source,
                               "Empty range works",
                               0);
}

void test_single_iteration(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(1) {\n"
        "        log(\"Single iteration\");\n"
        "    }\n"
        "    log(\"Single iteration works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Single iteration loop",
                               "single_iteration.asthra",
                               source,
                               "Single iteration works",
                               0);
}

void test_break_outside_loop(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    break;\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Break outside of loop",
                                 "break_outside_loop.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "break");
}

void test_continue_outside_loop(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    continue;\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Continue outside of loop",
                                 "continue_outside_loop.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "continue");
}

void test_mutable_accumulator(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut sum: i32 = 0;\n"
        "    for i in range(5) {\n"
        "        sum = sum + 1;\n"
        "    }\n"
        "    log(\"Mutable accumulator works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Loop with mutable accumulator",
                               "mutable_accumulator.asthra",
                               source,
                               "Mutable accumulator works",
                               0);
}

void test_string_iteration(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let text: string = \"ABC\";\n"
        "    for ch in text {\n"
        "        log(\"Char\");\n"
        "    }\n"
        "    log(\"String iteration works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("For loop over string characters",
                               "string_iteration.asthra",
                               source,
                               "String iteration works",
                               0);
}

void test_multiple_breaks(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    for i in range(2) {\n"
        "        log(\"Outer start\");\n"
        "        for j in range(5) {\n"
        "            if j == 2 {\n"
        "                break;\n"
        "            }\n"
        "            log(\"Inner\");\n"
        "        }\n"
        "        log(\"Outer end\");\n"
        "    }\n"
        "    log(\"Multiple breaks work correctly\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple breaks affect only innermost loop",
                               "multiple_breaks.asthra",
                               source,
                               "Multiple breaks work correctly",
                               0);
}

void test_return_from_loop(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn find_value(none) -> i32 {\n"
        "    for i in range(10) {\n"
        "        if i == 5 {\n"
        "            return i;\n"
        "        }\n"
        "    }\n"
        "    return -1;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = find_value();\n"
        "    log(\"Early return from loop works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Return from within loop",
                               "return_from_loop.asthra",
                               source,
                               "Early return from loop works",
                               0);
}

// Define test cases
BDD_DECLARE_TEST_CASES(control_flow_loops)
    BDD_TEST_CASE(simple_for_range, test_simple_for_range),
    BDD_TEST_CASE(range_start_end, test_range_start_end),
    BDD_WIP_TEST_CASE(array_iteration, test_array_iteration),  // Codegen bug: slice GEP uses value instead of pointer
    BDD_TEST_CASE(for_with_break, test_for_with_break),
    BDD_TEST_CASE(nested_break, test_nested_break),
    BDD_TEST_CASE(for_with_continue, test_for_with_continue),
    BDD_TEST_CASE(nested_continue, test_nested_continue),
    BDD_TEST_CASE(break_and_continue, test_break_and_continue),
    BDD_TEST_CASE(immutable_loop_var, test_immutable_loop_var),
    BDD_TEST_CASE(empty_range, test_empty_range),
    BDD_TEST_CASE(single_iteration, test_single_iteration),
    BDD_TEST_CASE(break_outside_loop, test_break_outside_loop),
    BDD_TEST_CASE(continue_outside_loop, test_continue_outside_loop),
    BDD_TEST_CASE(mutable_accumulator, test_mutable_accumulator),
    BDD_WIP_TEST_CASE(string_iteration, test_string_iteration),  // String iteration not implemented in semantic analyzer
    BDD_TEST_CASE(multiple_breaks, test_multiple_breaks),
    BDD_TEST_CASE(return_from_loop, test_return_from_loop),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("Control Flow Loops", control_flow_loops_test_cases)