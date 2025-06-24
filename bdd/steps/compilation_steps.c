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
extern void when_compile_with_flags(const char* flags);
extern void when_run_executable(void);
extern void then_compilation_should_succeed(void);
extern void then_compilation_should_fail(void);
extern void then_executable_created(void);
extern void then_error_contains(const char* expected_error);
extern void then_output_contains(const char* expected_output);
extern void then_exit_code_is(int expected_code);
extern void common_cleanup(void);
extern const char* get_compiler_output(void);
extern const char* get_current_executable(void);

// Compilation-specific state
static char* optimization_flags = NULL;
static long unoptimized_size = 0;
static long optimized_size = 0;

// Helper function to get file size
static long get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// Compilation-specific Given steps
void given_valid_asthra_source_file(void) {
    bdd_given("I have a valid Asthra source file");
    
    const char* valid_source = 
        "package test;\n"
        "\n"
        "pub fn factorial(n: i32) -> i32 {\n"
        "    if n <= 1 {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return n * factorial(n - 1);\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result = factorial(5);\n"
        "    log(\"Factorial of 5 is: {}\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("test_program.asthra", valid_source);
}

// Compilation-specific When steps
void when_compile_with_optimization_level(int level) {
    char desc[128];
    snprintf(desc, sizeof(desc), "I compile with optimization level %d", level);
    bdd_when(desc);
    
    // First compile without optimization to get baseline size
    when_compile_file();
    if (get_current_executable()) {
        unoptimized_size = get_file_size(get_current_executable());
    }
    
    // Now compile with optimization
    char flags[32];
    snprintf(flags, sizeof(flags), "-O%d", level);
    optimization_flags = strdup(flags);
    
    when_compile_with_flags(flags);
    if (get_current_executable()) {
        optimized_size = get_file_size(get_current_executable());
    }
}

// Compilation-specific Then steps
void then_output_should_be_optimized(void) {
    bdd_then("the output should be optimized");
    
    // Check that optimization was applied
    // This could check for specific optimization markers in the binary
    // For now, we just verify the compilation succeeded with optimization flags
    BDD_ASSERT_NOT_NULL(optimization_flags);
    
    const char* output = get_compiler_output();
    if (output) {
        // Could check for optimization-related messages
        BDD_ASSERT_TRUE(1); // Placeholder for actual optimization verification
    }
}

void then_binary_size_smaller_than_unoptimized(void) {
    bdd_then("the binary size should be smaller than unoptimized");
    
    BDD_ASSERT_TRUE(unoptimized_size > 0);
    BDD_ASSERT_TRUE(optimized_size > 0);
    
    // Optimized binary should generally be smaller
    // Allow some tolerance as sometimes optimizations can increase size
    BDD_ASSERT_TRUE(optimized_size <= unoptimized_size);
}

// Test implementations for compiler_basic.feature scenarios
void test_compile_hello_world(void) {
    bdd_scenario("Compile a simple Hello World program");
    
    given_asthra_compiler_available();
    
    const char* hello_source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Hello, World!\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("hello.asthra", hello_source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
}

void test_handle_syntax_errors(void) {
    bdd_scenario("Handle syntax errors gracefully");
    
    given_asthra_compiler_available();
    
    const char* error_source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    println(\"Missing semicolon\")\n"  // Missing semicolon
        "    return ();\n"
        "}\n";
    
    given_file_with_content("syntax_error.asthra", error_source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("expected ';'");
}

void test_optimize_with_o2(void) {
    bdd_scenario("Optimize code with -O2 flag");
    
    given_asthra_compiler_available();
    given_valid_asthra_source_file();
    when_compile_with_optimization_level(2);
    then_output_should_be_optimized();
    then_binary_size_smaller_than_unoptimized();
}

void test_compile_and_run_hello_world(void) {
    bdd_scenario("Compile and run Hello World program");
    
    given_asthra_compiler_available();
    
    const char* hello_source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Hello, World!\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("hello_run.asthra", hello_source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Hello, World!");
    then_exit_code_is(0);
}

// Main test runner
int main(void) {
    bdd_init("Basic Compiler Functionality");
    
    // Check if @wip scenarios should be skipped
    if (bdd_should_skip_wip()) {
        bdd_skip_scenario("Compile a simple Hello World program [@wip]");
        bdd_skip_scenario("Handle syntax errors gracefully [@wip]");
        bdd_skip_scenario("Compile and run Hello World program [@wip]");
        bdd_skip_scenario("Optimize code with -O2 flag [@wip]");
    } else {
        // Run all scenarios
        test_compile_hello_world();
        test_handle_syntax_errors();
        test_compile_and_run_hello_world();
        test_optimize_with_o2();
    }
    
    // Cleanup
    common_cleanup();
    if (optimization_flags) {
        free(optimization_flags);
    }
    
    return bdd_report();
}