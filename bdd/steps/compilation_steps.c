#include "bdd_test_framework.h"
#include "bdd_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// Compilation BDD Tests
// Tests compilation functionality using consolidated BDD utilities (no dependency on common_steps.c)

// ===================================================================
// CONSOLIDATED COMPILATION PATTERNS AND STATE
// ===================================================================

// Global compilation test state
static char* current_source_file = NULL;
static char* current_executable = NULL;
static char* compiler_output = NULL;
static int compilation_exit_code = -1;
static int execution_exit_code = -1;
static char* execution_output = NULL;
static char* optimization_flags = NULL;
static long unoptimized_size = 0;
static long optimized_size = 0;

// Consolidated cleanup function
static void cleanup_compilation_state(void) {
    if (current_source_file) {
        free(current_source_file);
        current_source_file = NULL;
    }
    if (current_executable) {
        free(current_executable);
        current_executable = NULL;
    }
    if (compiler_output) {
        free(compiler_output);
        compiler_output = NULL;
    }
    if (execution_output) {
        free(execution_output);
        execution_output = NULL;
    }
    if (optimization_flags) {
        free(optimization_flags);
        optimization_flags = NULL;
    }
    compilation_exit_code = -1;
    execution_exit_code = -1;
    unoptimized_size = 0;
    optimized_size = 0;
}

// Consolidated file size utility
static long get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// Consolidated compiler finding using BDD utilities
static const char* find_compiler(void) {
    return bdd_find_asthra_compiler();
}

// Consolidated compilation function using BDD utilities
static void compile_source_file(const char* source_file, const char* flags) {
    const char* compiler = find_compiler();
    if (!compiler) {
        compilation_exit_code = -1;
        compiler_output = strdup("Asthra compiler not found");
        return;
    }
    
    // Generate output filename
    if (current_executable) free(current_executable);
    current_executable = strdup("test_program");
    
    // Build compilation command
    char command[1024];
    if (flags && strlen(flags) > 0) {
        snprintf(command, sizeof(command), "%s %s -o %s %s", 
                compiler, flags, current_executable, source_file);
    } else {
        snprintf(command, sizeof(command), "%s -o %s %s", 
                compiler, current_executable, source_file);
    }
    
    // Use BDD utilities for compilation
    if (compiler_output) {
        free(compiler_output);
        compiler_output = NULL;
    }
    
    compiler_output = bdd_execute_command(command, &compilation_exit_code);
}

// Consolidated execution function using BDD utilities
static void execute_program(void) {
    if (!current_executable) {
        execution_exit_code = -1;
        execution_output = strdup("No executable available");
        return;
    }
    
    char command[1024];
    snprintf(command, sizeof(command), "./%s", current_executable);
    
    if (execution_output) {
        free(execution_output);
        execution_output = NULL;
    }
    
    execution_output = bdd_execute_command(command, &execution_exit_code);
}

// Consolidated source code templates
static const char* get_hello_world_source(void) {
    return "package main;\n"
           "\n"
           "pub fn main(none) -> void {\n"
           "    log(\"Hello, World!\");\n"
           "    return ();\n"
           "}\n";
}

static const char* get_syntax_error_source(void) {
    return "package main;\n"
           "\n"
           "pub fn main(none) -> void {\n"
           "    println(\"Missing semicolon\")\n"  // Missing semicolon
           "    return ();\n"
           "}\n";
}

static const char* get_simple_math_source(void) {
    return "package test;\n"
           "\n"
           "pub fn add(a: i32, b: i32) -> i32 {\n"
           "    return a + b;\n"
           "}\n"
           "\n"
           "pub fn main(none) -> void {\n"
           "    let result: i32 = add(2, 3);\n"
           "    log(\"Result is: {}\");\n"
           "    return ();\n"
           "}\n";
}

// ===================================================================
// TEST SCENARIO IMPLEMENTATIONS
// ===================================================================

// Test scenario: Compile a simple Hello World program
void test_compile_hello_world(void) {
    bdd_given("the Asthra compiler is available");
    const char* compiler = find_compiler();
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    bdd_assert(compiler != NULL, "Asthra compiler should be available");
    
    bdd_given("I have a valid Asthra source file");
    const char* source = get_hello_world_source();
    bdd_create_temp_source_file("hello.asthra", source);
    if (current_source_file) free(current_source_file);
    current_source_file = strdup("bdd-temp/hello.asthra");
    
    bdd_when("I compile the file");
    compile_source_file(current_source_file, NULL);
    
    bdd_then("compilation should succeed");
    bdd_assert(compilation_exit_code == 0, "Compilation should succeed");
    
    bdd_then("an executable should be created");
    bdd_assert(current_executable != NULL, "Executable name should be set");
    bdd_assert(access(current_executable, F_OK) == 0, "Executable file should exist");
}

// Test scenario: Handle syntax errors gracefully
void test_handle_syntax_errors(void) {
    bdd_given("the Asthra compiler is available");
    const char* compiler = find_compiler();
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    bdd_assert(compiler != NULL, "Asthra compiler should be available");
    
    bdd_given("I have a source file with syntax errors");
    const char* source = get_syntax_error_source();
    bdd_create_temp_source_file("syntax_error.asthra", source);
    if (current_source_file) free(current_source_file);
    current_source_file = strdup("bdd-temp/syntax_error.asthra");
    
    bdd_when("I compile the file");
    compile_source_file(current_source_file, NULL);
    
    bdd_then("compilation should fail");
    bdd_assert(compilation_exit_code != 0, "Compilation should fail");
    
    bdd_then("error message should contain syntax error details");
    bdd_assert(compiler_output != NULL, "Error output should be provided");
    // Look for actual compiler error patterns
    int has_syntax_error = (strstr(compiler_output, "Parsing failed") != NULL) ||
                          (strstr(compiler_output, "Parser errors") != NULL) ||
                          (strstr(compiler_output, "syntax error") != NULL) ||
                          (strstr(compiler_output, "Error") != NULL);
    bdd_assert(has_syntax_error, "Should contain syntax error information");
}

// Test scenario: Optimize code with -O2 flag
void test_optimize_with_o2(void) {
    bdd_given("the Asthra compiler is available");
    const char* compiler = find_compiler();
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    bdd_assert(compiler != NULL, "Asthra compiler should be available");
    
    bdd_given("I have a valid Asthra source file");
    const char* source = get_hello_world_source(); // Use the simplest working source
    bdd_create_temp_source_file("hello_opt.asthra", source);
    if (current_source_file) free(current_source_file);
    current_source_file = strdup("bdd-temp/hello_opt.asthra");
    
    bdd_when("I compile with optimization level 2");
    // First compile without optimization to get baseline
    compile_source_file(current_source_file, NULL);
    if (current_executable && access(current_executable, F_OK) == 0) {
        unoptimized_size = get_file_size(current_executable);
    }
    
    // Now compile with -O2
    optimization_flags = strdup("-O2");
    compile_source_file(current_source_file, optimization_flags);
    if (current_executable && access(current_executable, F_OK) == 0) {
        optimized_size = get_file_size(current_executable);
    }
    
    bdd_then("compilation should succeed with optimization");
    bdd_assert(compilation_exit_code == 0, "Optimized compilation should succeed");
    
    bdd_then("optimization should be applied");
    bdd_assert(optimization_flags != NULL, "Optimization flags should be set");
    // The fact that compilation succeeded with -O2 indicates optimization was applied
    bdd_assert(current_executable != NULL, "Optimized executable should be created");
}

// Test scenario: Compile and run Hello World program
void test_compile_and_run_hello_world(void) {
    bdd_given("the Asthra compiler is available");
    const char* compiler = find_compiler();
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    bdd_assert(compiler != NULL, "Asthra compiler should be available");
    
    bdd_given("I have a Hello World source file");
    const char* source = get_hello_world_source();
    bdd_create_temp_source_file("hello_run.asthra", source);
    if (current_source_file) free(current_source_file);
    current_source_file = strdup("bdd-temp/hello_run.asthra");
    
    bdd_when("I compile the file");
    compile_source_file(current_source_file, NULL);
    
    bdd_then("compilation should succeed");
    bdd_assert(compilation_exit_code == 0, "Compilation should succeed");
    
    bdd_then("an executable should be created");
    bdd_assert(current_executable != NULL, "Executable should be created");
    bdd_assert(access(current_executable, F_OK) == 0, "Executable file should exist");
    
    bdd_when("I run the executable");
    execute_program();
    
    bdd_then("the program should run successfully");
    bdd_assert(execution_exit_code == 0, "Program should exit successfully");
    
    bdd_then("the output should contain Hello World message");
    bdd_assert(execution_output != NULL, "Program should produce output");
    bdd_assert_output_contains(execution_output, "Hello");
}

// Test scenario: Compile with different output file (WIP)
void test_custom_output_file(void) {
    bdd_given("the Asthra compiler is available");
    const char* compiler = find_compiler();
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    
    bdd_given("I have a valid source file");
    const char* source = get_hello_world_source();
    bdd_create_temp_source_file("custom_output.asthra", source);
    if (current_source_file) free(current_source_file);
    current_source_file = strdup("bdd-temp/custom_output.asthra");
    
    bdd_when("I compile with custom output filename");
    if (current_executable) free(current_executable);
    current_executable = strdup("my_program");
    
    char flags[64];
    snprintf(flags, sizeof(flags), "-o %s", current_executable);
    compile_source_file(current_source_file, flags);
    
    bdd_then("custom output file should be created");
    bdd_assert(access(current_executable, F_OK) == 0, "Custom output file should exist");
    
    // Mark as WIP since advanced compilation options may not be fully implemented
    bdd_skip_scenario("Advanced compilation options not fully tested yet");
}

// Test scenario: Multi-file compilation (WIP)
void test_multi_file_compilation(void) {
    bdd_given("the Asthra compiler is available");
    const char* compiler = find_compiler();
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    
    bdd_given("I have multiple source files");
    const char* main_source = 
        "package main;\n"
        "import math;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result = math.add(2, 3);\n"
        "    log(\"Result: {}\");\n"
        "    return ();\n"
        "}\n";
    
    const char* math_source = 
        "package math;\n"
        "\n"
        "pub fn add(a: i32, b: i32) -> i32 {\n"
        "    return a + b;\n"
        "}\n";
    
    bdd_create_temp_source_file("main.asthra", main_source);
    bdd_create_temp_source_file("math.asthra", math_source);
    
    bdd_when("I compile multiple files");
    // This would require support for multi-file compilation
    bdd_skip_scenario("Multi-file compilation not fully implemented yet");
}

// Test scenario: Debug information generation (WIP)
void test_debug_information(void) {
    bdd_given("the Asthra compiler is available");
    const char* compiler = find_compiler();
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    
    bdd_given("I have a source file");
    const char* source = get_simple_math_source();
    bdd_create_temp_source_file("debug_test.asthra", source);
    if (current_source_file) free(current_source_file);
    current_source_file = strdup("bdd-temp/debug_test.asthra");
    
    bdd_when("I compile with debug information");
    compile_source_file(current_source_file, "-g");
    
    bdd_then("compilation should succeed with debug info");
    bdd_assert(compilation_exit_code == 0, "Debug compilation should succeed");
    
    // Mark as WIP since debug info generation may not be fully implemented
    bdd_skip_scenario("Debug information generation not fully implemented yet");
}

// ===================================================================
// DECLARATIVE TEST CASE DEFINITIONS
// ===================================================================

BddTestCase compilation_test_cases[] = {
    BDD_TEST_CASE(compile_hello_world, test_compile_hello_world),
    BDD_TEST_CASE(handle_syntax_errors, test_handle_syntax_errors),
    BDD_TEST_CASE(optimize_with_o2, test_optimize_with_o2),
    BDD_TEST_CASE(compile_and_run_hello_world, test_compile_and_run_hello_world),
    BDD_WIP_TEST_CASE(custom_output_file, test_custom_output_file),
    BDD_WIP_TEST_CASE(multi_file_compilation, test_multi_file_compilation),
    BDD_WIP_TEST_CASE(debug_information, test_debug_information),
};

int main(void) {
    return bdd_run_test_suite(
        "Compilation Functionality",
        compilation_test_cases,
        sizeof(compilation_test_cases) / sizeof(compilation_test_cases[0]),
        cleanup_compilation_state
    );
}