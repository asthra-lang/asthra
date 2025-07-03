#include "bdd_unit_common.h"

// Test scenarios for predeclared identifier shadowing

void test_shadow_log_with_function(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn log(level: string, msg: string) -> void {\n"
        "    // Custom logging implementation\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"INFO\", \"Using custom log function\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Shadow predeclared log with custom function",
                               "shadow_log_function.asthra",
                               source,
                               "",
                               0);
}

void test_shadow_range_with_different_signature(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn range(name: string) -> string {\n"  // Different signature than predeclared
        "    return name;\n"  // String concatenation not supported
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: string = range(\"test\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Shadow range with different signature",
                               "shadow_range_diff_sig.asthra",
                               source,
                               "",
                               0);
}

void test_shadow_panic_with_non_never_return(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn panic(recover: bool) -> bool {\n"  // Original returns Never
        "    return !recover;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let should_panic: bool = panic(false);\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Shadow panic with non-Never return type",
                               "shadow_panic_bool_return.asthra",
                               source,
                               "",
                               0);
}

void test_shadow_predeclared_with_variables(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let log: i32 = 100;\n"
        "    let range: bool = true;\n"
        "    let panic: string = \"don't panic\";\n"
        "    let exit: f64 = 3.14;\n"
        "    let args: [3]i32 = [1, 2, 3];\n"
        "    let len: u64 = 42;\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Shadow all predeclared identifiers with variables",
                               "shadow_all_as_vars.asthra",
                               source,
                               "",
                               0);
}

void test_nested_scope_shadowing(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Outer: using predeclared log\");\n"
        "    \n"
        "    {\n"
        "        let log: string = \"shadowed\";\n"
        "        // Cannot use log as function here\n"
        "    }\n"
        "    \n"
        "    log(\"Outer again: predeclared log available\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested scope shadowing and restoration",
                               "nested_scope_shadow.asthra",
                               source,
                               "Outer: using predeclared log\nOuter again: predeclared log available",
                               0);
}

void test_shadow_as_struct_type(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct log {\n"
        "    pub level: i32,\n"
        "    pub message: string\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let entry: log = log { level: 1, message: \"test\" };\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Shadow log as struct type",
                               "shadow_log_struct.asthra",
                               source,
                               "",
                               0);
}

void test_shadow_as_enum_type(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub enum range {\n"
        "    Empty,\n"
        "    Bounded((i32, i32)),\n"
        "    Unbounded\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let r: range = range.Bounded((0, 10));\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Shadow range as enum type",
                               "shadow_range_enum.asthra",
                               source,
                               "",
                               0);
}

void test_function_uses_predeclared_while_variable_shadows(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn process(none) -> void {\n"
        "    log(\"Using predeclared log in function\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let log: i32 = 42;\n"  // Shadow in main
        "    process();  // Function still uses predeclared\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Function uses predeclared while main shadows",
                               "function_vs_main_shadow.asthra",
                               source,
                               "Using predeclared log in function",
                               0);
}

void test_shadow_multiple_in_same_scope(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn log(msg: string) -> void { return (); }\n"
        "pub fn range(dummy: i32) -> i32 { return 10; }\n"
        "pub fn panic(none) -> bool { return true; }\n"
        "pub fn exit(none) -> i32 { return 0; }\n"
        "pub fn args(dummy: i32) -> []string { \n"
        "    // Return an empty slice\n"
        "    return [none];\n"
        "}\n"
        "pub fn len(x: i32) -> i32 { return x; }\n"
        "pub fn infinite(dummy: i32) -> bool { return false; }\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"custom\");\n"
        "    let r: i32 = range(1);\n"
        "    let p: bool = panic();\n"
        "    let e: i32 = exit();\n"
        "    let a: []string = args(0);\n"
        "    let l: i32 = len(5);\n"
        "    let inf: bool = infinite(0);\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Shadow multiple predeclared as functions",
                               "shadow_multiple_funcs.asthra",
                               source,
                               "",
                               0);
}

void test_mixed_shadowing_types(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn log(none) -> void { return (); }\n"  // Function
        "pub struct range { pub value: i32 }\n"   // Struct
        "pub enum panic { Ok, Error }\n"          // Enum
        "\n"
        "pub fn main(none) -> void {\n"
        "    log();\n"
        "    let r: range = range { value: 10 };\n"
        "    let p: panic = panic.Ok;\n"
        "    \n"
        "    let exit: string = \"shadowed\";\n"  // Variable
        "    let args: bool = true;\n"            // Variable
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Mixed shadowing with different types",
                               "mixed_shadow_types.asthra",
                               source,
                               "",
                               0);
}

void test_shadow_then_use_as_wrong_type(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let log: i32 = 42;\n"
        "    log(\"This should fail\");  // log is now a variable, not a function\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Use shadowed variable as function",
                                 "shadow_wrong_type_use.asthra",
                                 source,
                                 0,  // should fail
                                 "not a function");
}

void test_struct_field_with_predeclared_name(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Config {\n"
        "    pub log: string,\n"
        "    pub range: i32,\n"
        "    pub panic: bool\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let cfg: Config = Config {\n"
        "        log: \"enabled\",\n"
        "        range: 100,\n"
        "        panic: false\n"
        "    };\n"
        "    log(\"Config created\");  // Still can use predeclared log\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Struct fields with predeclared names",
                               "struct_fields_predeclared.asthra",
                               source,
                               "Config created",
                               0);
}

void test_method_with_predeclared_name(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Logger {\n"
        "    pub enabled: bool\n"
        "}\n"
        "\n"
        "impl Logger {\n"
        "    pub fn log(self, msg: string) -> void {\n"  // Method named 'log'
        "        if self.enabled {\n"
        "            // Would use original log here if needed\n"
        "        }\n"
        "        return ();\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let logger: Logger = Logger { enabled: true };\n"
        "    logger.log(\"Method log\");\n"
        "    log(\"Predeclared log\");\n"  // Can still use predeclared
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Method with predeclared name",
                               "method_predeclared_name.asthra",
                               source,
                               "Predeclared log",
                               0);
}

void test_associated_function_with_predeclared_name(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub struct Utils {\n"
        "    value: i32\n"
        "}\n"
        "\n"
        "impl Utils {\n"
        "    pub fn range(start: i32, end: i32) -> []i32 {\n"
        "        return [none];\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let nums: []i32 = Utils::range(1, 5);\n"
        "    // Note: the predeclared range is shadowed by Utils type\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Associated function with predeclared name",
                               "assoc_func_predeclared.asthra",
                               source,
                               "",
                               0);
}

// Define test cases
BddTestCase identifier_shadowing_test_cases[] = {
    BDD_TEST_CASE(shadow_log_with_function, test_shadow_log_with_function),
    BDD_TEST_CASE(shadow_range_with_different_signature, test_shadow_range_with_different_signature),
    BDD_TEST_CASE(shadow_panic_with_non_never_return, test_shadow_panic_with_non_never_return),
    BDD_TEST_CASE(shadow_predeclared_with_variables, test_shadow_predeclared_with_variables),
    BDD_TEST_CASE(nested_scope_shadowing, test_nested_scope_shadowing),
    BDD_TEST_CASE(shadow_as_struct_type, test_shadow_as_struct_type),
    BDD_TEST_CASE(shadow_as_enum_type, test_shadow_as_enum_type),
    BDD_TEST_CASE(function_uses_predeclared_while_variable_shadows, test_function_uses_predeclared_while_variable_shadows),
    BDD_TEST_CASE(shadow_multiple_in_same_scope, test_shadow_multiple_in_same_scope),
    BDD_TEST_CASE(mixed_shadowing_types, test_mixed_shadowing_types),
    BDD_TEST_CASE(shadow_then_use_as_wrong_type, test_shadow_then_use_as_wrong_type),
    BDD_TEST_CASE(struct_field_with_predeclared_name, test_struct_field_with_predeclared_name),
    BDD_TEST_CASE(method_with_predeclared_name, test_method_with_predeclared_name),
    BDD_TEST_CASE(associated_function_with_predeclared_name, test_associated_function_with_predeclared_name),
};

BDD_UNIT_TEST_MAIN("Identifier Shadowing", identifier_shadowing_test_cases)