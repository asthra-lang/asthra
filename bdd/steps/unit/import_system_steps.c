#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"

// Test scenarios using the new reusable framework

void test_import_stdlib_module(void) {
    const char* source = 
        "package main;\n"
        "import \"stdlib/io\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Import works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Import standard library module",
                               "import_stdlib.asthra",
                               source,
                               "Import works",
                               0);
}

void test_import_with_alias(void) {
    const char* source = 
        "package main;\n"
        "import \"stdlib/collections\" as col;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Import alias works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Import with alias",
                               "import_alias.asthra",
                               source,
                               "Import alias works",
                               0);
}

void test_multiple_imports(void) {
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
    
    bdd_run_execution_scenario("Multiple imports",
                               "multiple_imports.asthra",
                               source,
                               "Multiple imports work",
                               0);
}

void test_import_relative_path(void) {
    const char* source = 
        "package main;\n"
        "import \"./utils\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Relative import works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Import relative path",
                               "import_relative.asthra",
                               source,
                               "Relative import works",
                               0);
}

void test_import_github_package(void) {
    const char* source = 
        "package main;\n"
        "import \"github.com/user/package\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"GitHub import works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Import GitHub package",
                               "import_github.asthra",
                               source,
                               "GitHub import works",
                               0);
}

void test_import_without_quotes(void) {
    const char* source = 
        "package main;\n"
        "import stdlib/io;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"No quotes\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Import without quotes",
                                 "import_no_quotes.asthra",
                                 source,
                                 0,  // should fail
                                 "expected string literal");
}

void test_import_without_semicolon(void) {
    const char* source = 
        "package main;\n"
        "import \"stdlib/io\"\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"No semicolon\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Import without semicolon",
                                 "import_no_semicolon.asthra",
                                 source,
                                 0,  // should fail
                                 "expected ';'");
}

void test_import_before_package(void) {
    const char* source = 
        "import \"stdlib/io\";\n"
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Import before package\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Import before package declaration",
                                 "import_before_package.asthra",
                                 source,
                                 0,  // should fail
                                 "expected 'package'");
}

void test_import_invalid_path(void) {
    const char* source = 
        "package main;\n"
        "import \"invalid://path\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Invalid path\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Import with invalid path",
                                 "import_invalid_path.asthra",
                                 source,
                                 0,  // should fail
                                 "invalid import path");
}

void test_duplicate_imports(void) {
    const char* source = 
        "package main;\n"
        "import \"stdlib/io\";\n"
        "import \"stdlib/io\";\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Duplicate imports\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Duplicate imports",
                                 "duplicate_imports.asthra",
                                 source,
                                 0,  // should fail
                                 "duplicate import");
}

void test_conflicting_aliases(void) {
    const char* source = 
        "package main;\n"
        "import \"stdlib/io\" as utils;\n"
        "import \"stdlib/math\" as utils;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Conflicting aliases\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Import with conflicting aliases",
                                 "conflicting_aliases.asthra",
                                 source,
                                 0,  // should fail
                                 "alias 'utils' already defined");
}

// Define test cases using the new framework - @wip tags based on original file
BddTestCase import_system_test_cases[] = {
    BDD_TEST_CASE(import_stdlib_module, test_import_stdlib_module),
    BDD_TEST_CASE(import_with_alias, test_import_with_alias),
    BDD_TEST_CASE(multiple_imports, test_multiple_imports),
    BDD_TEST_CASE(import_relative_path, test_import_relative_path),
    BDD_TEST_CASE(import_github_package, test_import_github_package),
    BDD_WIP_TEST_CASE(import_without_quotes, test_import_without_quotes),
    BDD_TEST_CASE(import_without_semicolon, test_import_without_semicolon),
    BDD_TEST_CASE(import_before_package, test_import_before_package),
    BDD_WIP_TEST_CASE(import_invalid_path, test_import_invalid_path),
    BDD_TEST_CASE(duplicate_imports, test_duplicate_imports),
    BDD_WIP_TEST_CASE(conflicting_aliases, test_conflicting_aliases),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Import System",
                              import_system_test_cases,
                              sizeof(import_system_test_cases) / sizeof(import_system_test_cases[0]),
                              bdd_cleanup_temp_files);
}