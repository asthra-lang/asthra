#include "bdd_unit_common.h"

// Test scenarios for reserved keyword enforcement

void test_keyword_as_variable_name(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let fn: i32 = 42;\n"  // 'fn' is a reserved keyword
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Reserved keyword 'fn' as variable name",
                                 "keyword_fn_as_var.asthra",
                                 source,
                                 0,  // should fail
                                 "reserved keyword");
}

void test_keyword_let_as_variable(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let let: i32 = 10;\n"  // 'let' is a reserved keyword
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Reserved keyword 'let' as variable name",
                                 "keyword_let_as_var.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

void test_keyword_struct_as_variable(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let struct: bool = true;\n"  // 'struct' is a reserved keyword
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Reserved keyword 'struct' as variable name",
                                 "keyword_struct_as_var.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

void test_keyword_as_function_name(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn if(x: i32) -> i32 {\n"  // 'if' is a reserved keyword
        "    return x * 2;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Reserved keyword 'if' as function name",
                                 "keyword_if_as_func.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

void test_keyword_match_as_function(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn match(pattern: string) -> bool {\n"  // 'match' is a reserved keyword
        "    return true;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Reserved keyword 'match' as function name",
                                 "keyword_match_as_func.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

void test_keyword_as_struct_name(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct enum {\n"  // 'enum' is a reserved keyword
        "    value: i32\n"
        "}\n";
    
    bdd_run_compilation_scenario("Reserved keyword 'enum' as struct name",
                                 "keyword_enum_as_struct.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

void test_keyword_unsafe_as_enum(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub enum unsafe {\n"  // 'unsafe' is a reserved keyword
        "    Safe,\n"
        "    Dangerous\n"
        "}\n";
    
    bdd_run_compilation_scenario("Reserved keyword 'unsafe' as enum name",
                                 "keyword_unsafe_as_enum.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

void test_keyword_as_field_name(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Config {\n"
        "    pub return: i32,\n"  // 'return' is a reserved keyword
        "    pub value: bool\n"
        "}\n";
    
    bdd_run_compilation_scenario("Reserved keyword 'return' as field name",
                                 "keyword_return_as_field.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

void test_multiple_keywords_as_fields(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct BadStruct {\n"
        "    pub const: i32,\n"     // 'const' is reserved
        "    pub mut: bool,\n"       // 'mut' is reserved
        "    pub spawn: string\n"    // 'spawn' is reserved
        "}\n";
    
    bdd_run_compilation_scenario("Multiple reserved keywords as field names",
                                 "keywords_multiple_fields.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

// Test that predeclared identifiers CAN be used as identifiers
void test_predeclared_log_as_variable(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let log: string = \"custom log\";\n"  // 'log' is predeclared, not reserved
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Predeclared 'log' as variable name",
                               "predeclared_log_var.asthra",
                               source,
                               "",  // No output expected
                               0);
}

void test_predeclared_range_as_function(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn range(start: i32, end: i32) -> i32 {\n"  // 'range' is predeclared, not reserved
        "    return end - start;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let diff: i32 = range(5, 10);\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Predeclared 'range' as function name",
                               "predeclared_range_func.asthra",
                               source,
                               "",
                               0);
}

void test_predeclared_as_struct_name(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct panic {\n"  // 'panic' is predeclared, not reserved
        "    pub message: string\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: panic = panic { message: \"test\" };\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Predeclared 'panic' as struct name",
                               "predeclared_panic_struct.asthra",
                               source,
                               "",
                               0);
}

void test_predeclared_as_enum_name(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub enum exit {\n"  // 'exit' is predeclared, not reserved
        "    Success,\n"
        "    Failure(i32)\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let status: exit = exit.Success;\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Predeclared 'exit' as enum name",
                               "predeclared_exit_enum.asthra",
                               source,
                               "",
                               0);
}

void test_predeclared_as_field_names(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Logger {\n"
        "    pub log: string,\n"      // 'log' is predeclared, not reserved
        "    pub panic: bool,\n"      // 'panic' is predeclared, not reserved
        "    pub range: i32\n"        // 'range' is predeclared, not reserved
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let logger: Logger = Logger { log: \"test\", panic: false, range: 10 };\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Predeclared identifiers as field names",
                               "predeclared_fields.asthra",
                               source,
                               "",
                               0);
}

void test_all_predeclared_shadowing(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let log: i32 = 1;\n"
        "    let range: i32 = 2;\n"
        "    let panic: i32 = 3;\n"
        "    let exit: i32 = 4;\n"
        "    let args: i32 = 5;\n"
        "    let len: i32 = 6;\n"
        "    let infinite: i32 = 7;\n"
        "    let total: i32 = log + range + panic + exit + args + len + infinite;\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("All predeclared identifiers shadowed",
                               "all_predeclared_shadow.asthra",
                               source,
                               "",
                               0);
}

// Test case for special keywords
void test_special_keyword_void(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let void: i32 = 42;\n"  // 'void' is a reserved keyword for types
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Reserved keyword 'void' as variable",
                                 "keyword_void_var.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

void test_special_keyword_none(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let none: string = \"nothing\";\n"  // 'none' is a reserved keyword
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Reserved keyword 'none' as variable",
                                 "keyword_none_var.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

void test_builtin_types_as_identifiers(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let Result: i32 = 1;\n"     // 'Result' is reserved
        "    let Option: i32 = 2;\n"     // 'Option' is reserved
        "    let TaskHandle: i32 = 3;\n" // 'TaskHandle' is reserved
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Built-in type names as variables",
                                 "builtin_types_vars.asthra",
                                 source,
                                 0,
                                 "reserved keyword");
}

// Define test cases
BddTestCase reserved_keywords_test_cases[] = {
    // Reserved keyword tests - should fail
    BDD_TEST_CASE(keyword_as_variable_name, test_keyword_as_variable_name),
    BDD_TEST_CASE(keyword_let_as_variable, test_keyword_let_as_variable),
    BDD_TEST_CASE(keyword_struct_as_variable, test_keyword_struct_as_variable),
    BDD_TEST_CASE(keyword_as_function_name, test_keyword_as_function_name),
    BDD_TEST_CASE(keyword_match_as_function, test_keyword_match_as_function),
    BDD_TEST_CASE(keyword_as_struct_name, test_keyword_as_struct_name),
    BDD_TEST_CASE(keyword_unsafe_as_enum, test_keyword_unsafe_as_enum),
    BDD_TEST_CASE(keyword_as_field_name, test_keyword_as_field_name),
    BDD_TEST_CASE(multiple_keywords_as_fields, test_multiple_keywords_as_fields),
    BDD_TEST_CASE(special_keyword_void, test_special_keyword_void),
    BDD_TEST_CASE(special_keyword_none, test_special_keyword_none),
    BDD_TEST_CASE(builtin_types_as_identifiers, test_builtin_types_as_identifiers),
    
    // Predeclared identifier tests - should succeed
    BDD_TEST_CASE(predeclared_log_as_variable, test_predeclared_log_as_variable),
    BDD_TEST_CASE(predeclared_range_as_function, test_predeclared_range_as_function),
    BDD_TEST_CASE(predeclared_as_struct_name, test_predeclared_as_struct_name),
    BDD_TEST_CASE(predeclared_as_enum_name, test_predeclared_as_enum_name),
    BDD_TEST_CASE(predeclared_as_field_names, test_predeclared_as_field_names),
    BDD_TEST_CASE(all_predeclared_shadowing, test_all_predeclared_shadowing),
};

BDD_UNIT_TEST_MAIN("Reserved Keywords", reserved_keywords_test_cases)