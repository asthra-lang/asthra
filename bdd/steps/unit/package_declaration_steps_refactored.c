#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"

// Test scenarios using the new reusable framework

void test_simple_package_declaration(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Package declaration works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple package declaration",
                               "simple_package.asthra",
                               source,
                               "Package declaration works",
                               0);
}

void test_package_with_identifier(void) {
    const char* source = 
        "package myapp;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Named package works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Package declaration with identifier",
                               "named_package.asthra",
                               source,
                               "Named package works",
                               0);
}

void test_package_with_underscore(void) {
    const char* source = 
        "package my_app;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Underscore package works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Package declaration with underscored identifier",
                               "underscore_package.asthra",
                               source,
                               "Underscore package works",
                               0);
}

void test_missing_package_declaration(void) {
    const char* source = 
        "pub fn main(none) -> void {\n"
        "    log(\"No package\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Missing package declaration",
                                 "no_package.asthra",
                                 source,
                                 0,  // should fail
                                 "expected 'package'");
}

void test_package_without_semicolon(void) {
    const char* source = 
        "package main\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Missing semicolon\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Package declaration without semicolon",
                                 "no_semicolon.asthra",
                                 source,
                                 0,  // should fail
                                 "expected ';'");
}

void test_package_invalid_characters(void) {
    const char* source = 
        "package my-app;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Invalid package name\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Package declaration with invalid characters",
                                 "invalid_package.asthra",
                                 source,
                                 0,  // should fail
                                 "invalid package name");
}

void test_multiple_package_declarations(void) {
    const char* source = 
        "package first;\n"
        "package second;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Multiple packages\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Multiple package declarations",
                                 "multiple_packages.asthra",
                                 source,
                                 0,  // should fail
                                 "multiple package declarations");
}

void test_package_not_at_beginning(void) {
    const char* source = 
        "pub fn helper(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Package after code\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Package declaration not at beginning",
                                 "package_after_code.asthra",
                                 source,
                                 0,  // should fail
                                 "expected 'package'");
}

// Define test cases using the new framework - @wip tags based on original file
BddTestCase package_declaration_test_cases[] = {
    BDD_TEST_CASE(simple_package_declaration, test_simple_package_declaration),
    BDD_TEST_CASE(package_with_identifier, test_package_with_identifier),
    BDD_TEST_CASE(package_with_underscore, test_package_with_underscore),
    BDD_TEST_CASE(missing_package_declaration, test_missing_package_declaration),
    BDD_TEST_CASE(package_without_semicolon, test_package_without_semicolon),
    BDD_WIP_TEST_CASE(package_invalid_characters, test_package_invalid_characters),
    BDD_WIP_TEST_CASE(multiple_package_declarations, test_multiple_package_declarations),
    BDD_TEST_CASE(package_not_at_beginning, test_package_not_at_beginning),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Package Declaration Syntax",
                              package_declaration_test_cases,
                              sizeof(package_declaration_test_cases) / sizeof(package_declaration_test_cases[0]),
                              bdd_cleanup_temp_files);
}