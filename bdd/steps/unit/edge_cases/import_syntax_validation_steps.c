#include "bdd_unit_common.h"

// Test scenarios for import and module syntax validation

// Test 1: Reject Python-style import syntax
void test_python_style_import(void) {
    const char* source = 
        "package test;\n"
        "from stdlib.io import println;  // Error: No 'from...import' syntax\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Python-style from...import",
                                 "python_import.astra",
                                 source,
                                 0,  // should fail
                                 "expected declaration but found 'from'");
}

// Test 2: Reject Rust-style use statements
void test_rust_style_use(void) {
    const char* source = 
        "package test;\n"
        "use std::collections::HashMap;  // Error: Use 'import' not 'use'\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Rust-style use statement",
                                 "rust_use.astra",
                                 source,
                                 0,  // should fail
                                 "expected declaration but found 'use'");
}

// Test 3: Reject import without quotes
void test_import_without_quotes(void) {
    const char* source = 
        "package test;\n"
        "import stdlib/io;  // Error: Path must be quoted\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Import without quotes",
                                 "import_no_quotes.astra",
                                 source,
                                 0,  // should fail
                                 "expected string literal");
}

// Test 4: Reject module declaration syntax
void test_module_keyword(void) {
    const char* source = 
        "module test;  // Error: Use 'package' not 'module'\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("module keyword instead of package",
                                 "module_keyword.astra",
                                 source,
                                 0,  // should fail
                                 "expected 'package' keyword");
}

// Test 5: Positive test - correct import and package syntax
void test_correct_import_syntax(void) {
    const char* source = 
        "package test;\n"
        "import \"stdlib/io\";\n"
        "import \"stdlib/collections\" as coll;\n"
        "import \"./local_module\";\n"
        "import \"github.com/user/repo\";\n"
        "pub fn main(none) -> i32 {\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_compilation_scenario("Correct import syntax",
                                 "correct_imports.astra",
                                 source,
                                 1,  // should succeed
                                 NULL);
}

// BDD test registry
BddTestCase import_syntax_validation_tests[] = {
    BDD_TEST_CASE(python_style_import, test_python_style_import),
    BDD_TEST_CASE(rust_style_use, test_rust_style_use),
    BDD_TEST_CASE(import_without_quotes, test_import_without_quotes),
    BDD_TEST_CASE(module_keyword, test_module_keyword),
    BDD_TEST_CASE(correct_import_syntax, test_correct_import_syntax)
};

// Main entry point for the test suite
void run_import_syntax_validation_tests(void) {
    bdd_run_test_suite("Import Syntax Validation",
                       import_syntax_validation_tests,
                       sizeof(import_syntax_validation_tests) / sizeof(BddTestCase),
                       NULL);  // No special cleanup needed
}

// Main test runner
int main(void) {
    printf("=== BDD Test: Import Syntax Validation ===\n\n");
    
    return bdd_run_test_suite("Import Syntax Validation",
                              import_syntax_validation_tests,
                              sizeof(import_syntax_validation_tests) / sizeof(BddTestCase),
                              NULL);
}