#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bdd_support.h"

// External functions from common_steps.c
extern void given_asthra_compiler_available(void);
extern void given_file_with_content(const char* filename, const char* content);
extern void when_compile_file(void);
extern void when_run_executable(void);
extern void then_compilation_should_succeed(void);
extern void then_compilation_should_fail(void);
extern void then_executable_created(void);
extern void then_output_contains(const char* expected_output);
extern void then_exit_code_is(int expected_code);
extern void then_error_contains(const char* expected_error);
extern void common_cleanup(void);

// Test scenario: Simple package declaration
void test_simple_package_declaration(void) {
    bdd_scenario("Simple package declaration");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Package declaration works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("simple_package.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Package declaration works");
    then_exit_code_is(0);
}

// Test scenario: Package declaration with identifier
void test_package_with_identifier(void) {
    bdd_scenario("Package declaration with identifier");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package myapp;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Named package works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("named_package.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Named package works");
    then_exit_code_is(0);
}

// Test scenario: Package declaration with underscored identifier
void test_package_with_underscore(void) {
    bdd_scenario("Package declaration with underscored identifier");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package my_app;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Underscore package works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("underscore_package.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Underscore package works");
    then_exit_code_is(0);
}

// Test scenario: Missing package declaration
void test_missing_package_declaration(void) {
    bdd_scenario("Missing package declaration");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "pub fn main(none) -> void {\n"
        "    log(\"No package\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("no_package.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("expected 'package'");
}

// Test scenario: Package declaration without semicolon
void test_package_without_semicolon(void) {
    bdd_scenario("Package declaration without semicolon");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Missing semicolon\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("no_semicolon.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("expected ';'");
}

// Test scenario: Package declaration with invalid characters
void test_package_invalid_characters(void) {
    bdd_scenario("Package declaration with invalid characters");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package my-app;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Invalid package name\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("invalid_package.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("invalid package name");
}

// Test scenario: Multiple package declarations
void test_multiple_package_declarations(void) {
    bdd_scenario("Multiple package declarations");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package first;\n"
        "package second;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Multiple packages\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("multiple_packages.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("multiple package declarations");
}

// Test scenario: Package declaration not at beginning
void test_package_not_at_beginning(void) {
    bdd_scenario("Package declaration not at beginning");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("package_after_code.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("expected 'package'");
}

// Main test runner
int main(void) {
    bdd_init("Package Declaration Syntax");
    
    // Check if @wip scenarios should be skipped
    if (bdd_should_skip_wip()) {
        // Skip @wip scenarios
        bdd_skip_scenario("Package declaration with invalid characters [@wip]");
        bdd_skip_scenario("Multiple package declarations [@wip]");
        
        // Run non-@wip scenarios
        test_simple_package_declaration();
        test_package_with_identifier();
        test_package_with_underscore();
        test_missing_package_declaration();
        test_package_without_semicolon();
        test_package_not_at_beginning();
    } else {
        // Run all scenarios from package_declaration.feature
        test_simple_package_declaration();
        test_package_with_identifier();
        test_package_with_underscore();
        test_missing_package_declaration();
        test_package_without_semicolon();
        test_package_invalid_characters();
        test_multiple_package_declarations();
        test_package_not_at_beginning();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}