#include "bdd_unit_common.h"

void test_basic_log_function(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    log(\"Hello, World!\");\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic log function",
                               "log_basic.asthra",
                               source,
                               NULL,
                               0);
}

void test_range_single_argument(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut sum: i32 = 0;\n"
        "    for i in range(5) {\n"
        "        sum = sum + i;\n"
        "    }\n"
        "    return sum;\n"
        "}\n";
    
    bdd_run_execution_scenario("Range single argument",
                               "range_single.asthra",
                               source,
                               NULL,
                               10);
}

void test_range_start_and_end(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut sum: i32 = 0;\n"
        "    for i in range(3, 7) {\n"
        "        sum = sum + i;\n"
        "    }\n"
        "    return sum;\n"
        "}\n";
    
    bdd_run_execution_scenario("Range start and end",
                               "range_start_end.asthra",
                               source,
                               NULL,
                               18);
}

void test_panic_function(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    panic(\"This is a panic message\");\n"
        "    return 0;\n"
        "}\n";
    
    // Panic should cause non-zero exit
    bdd_run_compilation_scenario("Panic function",
                                 "panic_basic.asthra",
                                 source,
                                 1,  // Should compile successfully
                                 NULL);
}

void test_exit_function(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    exit(123);\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Exit function",
                               "exit_basic.asthra",
                               source,
                               NULL,
                               123);
}

void test_args_function(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let arguments: []string = args();\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Args function",
                               "args_basic.asthra",
                               source,
                               NULL,
                               0);
}

void test_shadowing_log_function(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn log(level: string, message: string) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    log(\"INFO\", \"Custom logging\");\n"
        "    return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Shadowing log function",
                               "log_shadow.asthra",
                               source,
                               NULL,
                               42);
}

void test_log_error_wrong_type(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    log(42);\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_type_mismatch_scenario("Log wrong type error",
                                   "log_error_type.asthra",
                                   source);
}

BDD_DECLARE_TEST_CASES(predeclared_identifiers)
    BDD_TEST_CASE("basic log function", test_basic_log_function),
    BDD_TEST_CASE("range single argument", test_range_single_argument),
    BDD_TEST_CASE("range start and end", test_range_start_and_end),
    BDD_TEST_CASE("panic function", test_panic_function),
    BDD_TEST_CASE("exit function", test_exit_function),
    BDD_TEST_CASE("args function", test_args_function),
    BDD_TEST_CASE("shadowing log function", test_shadowing_log_function),
    BDD_TEST_CASE("log error wrong type", test_log_error_wrong_type)
BDD_END_TEST_CASES()

BDD_UNIT_TEST_MAIN("Predeclared identifiers", predeclared_identifiers_test_cases)