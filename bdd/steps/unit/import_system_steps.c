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

// Test scenario: Import standard library module
void test_import_stdlib_module(void) {
    bdd_scenario("Import standard library module");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "import \"stdlib/io\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Import works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("import_stdlib.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Import works");
    then_exit_code_is(0);
}

// Test scenario: Import with alias
void test_import_with_alias(void) {
    bdd_scenario("Import with alias");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "import \"stdlib/collections\" as col;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Import alias works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("import_alias.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Import alias works");
    then_exit_code_is(0);
}

// Test scenario: Multiple imports
void test_multiple_imports(void) {
    bdd_scenario("Multiple imports");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "import \"stdlib/io\";\n"
        "import \"stdlib/math\";\n"
        "import \"stdlib/strings\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Multiple imports work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("multiple_imports.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Multiple imports work");
    then_exit_code_is(0);
}

// Test scenario: Import relative path
void test_import_relative_path(void) {
    bdd_scenario("Import relative path");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "import \"./utils\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Relative import works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("import_relative.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Relative import works");
    then_exit_code_is(0);
}

// Test scenario: Import GitHub package
void test_import_github_package(void) {
    bdd_scenario("Import GitHub package");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "import \"github.com/user/package\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"GitHub import works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("import_github.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("GitHub import works");
    then_exit_code_is(0);
}

// Test scenario: Import without quotes
void test_import_without_quotes(void) {
    bdd_scenario("Import without quotes");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "import stdlib/io;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"No quotes\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("import_no_quotes.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("expected string literal");
}

// Test scenario: Import without semicolon
void test_import_without_semicolon(void) {
    bdd_scenario("Import without semicolon");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "import \"stdlib/io\"\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"No semicolon\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("import_no_semicolon.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("expected ';'");
}

// Test scenario: Import before package declaration
void test_import_before_package(void) {
    bdd_scenario("Import before package declaration");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "import \"stdlib/io\";\n"
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Import before package\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("import_before_package.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("expected 'package'");
}

// Test scenario: Import with invalid path
void test_import_invalid_path(void) {
    bdd_scenario("Import with invalid path");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "import \"invalid://path\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Invalid path\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("import_invalid_path.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("invalid import path");
}

// Test scenario: Duplicate imports
void test_duplicate_imports(void) {
    bdd_scenario("Duplicate imports");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "import \"stdlib/io\";\n"
        "import \"stdlib/io\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Duplicate imports\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("duplicate_imports.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("duplicate import");
}

// Test scenario: Import with conflicting aliases
void test_conflicting_aliases(void) {
    bdd_scenario("Import with conflicting aliases");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "import \"stdlib/io\" as utils;\n"
        "import \"stdlib/math\" as utils;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Conflicting aliases\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("conflicting_aliases.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("alias 'utils' already defined");
}

// Main test runner
int main(void) {
    bdd_init("Import System");
    
    if (bdd_should_skip_wip()) {
        // Skip @wip scenarios, run only passing scenarios
        test_import_stdlib_module();
        test_import_with_alias();
        test_multiple_imports();
        test_import_relative_path();
        test_import_github_package();
        test_import_without_semicolon();
        test_import_before_package();
    } else {
        // Run all scenarios from import_system.feature
        test_import_stdlib_module();
        test_import_with_alias();
        test_multiple_imports();
        test_import_relative_path();
        test_import_github_package();
        test_import_without_quotes();
        test_import_without_semicolon();
        test_import_before_package();
        test_import_invalid_path();
        test_duplicate_imports();
        test_conflicting_aliases();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}