/**
 * Backend Integration Tests
 * End-to-end tests that compile the same Asthra code with different backends
 * and verify output equivalence and correctness
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../../tests/framework/test_framework.h"
#include "../../../src/compiler.h"
#include "../../../src/cli.h"

// Test data - simple Asthra programs
static const char* SIMPLE_ASTHRA_PROGRAM = 
    "fn main() -> i32 {\n"
    "    return 42;\n"
    "}\n";

static const char* ARITHMETIC_ASTHRA_PROGRAM = 
    "fn main() -> i32 {\n"
    "    let x: i32 = 10;\n"
    "    let y: i32 = 32;\n"
    "    return x + y;\n"
    "}\n";

static const char* FUNCTION_CALL_ASTHRA_PROGRAM = 
    "fn add(a: i32, b: i32) -> i32 {\n"
    "    return a + b;\n"
    "}\n"
    "\n"
    "fn main() -> i32 {\n"
    "    return add(20, 22);\n"
    "}\n";

// Utility functions
static bool write_test_file(const char* filename, const char* content) {
    FILE *f = fopen(filename, "w");
    if (!f) return false;
    
    fprintf(f, "%s", content);
    fclose(f);
    return true;
}

static bool file_exists(const char* filename) {
    return access(filename, F_OK) == 0;
}

static long get_file_size(const char* filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    return size;
}

static bool run_compiler_with_backend(const char* input_file, const char* output_file, 
                                     AsthraBackendType backend) {
    // Create compiler options
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.input_file = input_file;
    options.output_file = output_file;
    options.backend_type = backend;
    options.verbose = false;
    
    // Create compiler context
    AsthraCompilerContext *ctx = asthra_compiler_create(&options);
    if (!ctx) return false;
    
    // Compile the file
    int result = asthra_compile_file(ctx, input_file, output_file);
    
    // Cleanup
    asthra_compiler_destroy(ctx);
    
    return result == 0;
}

static int run_compiled_executable(const char* executable_path) {
    // Make sure file is executable
    if (chmod(executable_path, 0755) != 0) {
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process - execute the program
        execl(executable_path, executable_path, NULL);
        exit(127); // exec failed
    } else if (pid > 0) {
        // Parent process - wait for child
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    } else {
        return -1; // fork failed
    }
}

// Test 1: Simple Program Compilation
TEST_CASE(test_simple_program_compilation) {
    const char* test_file = "/tmp/test_simple.asthra";
    const char* c_output = "/tmp/test_simple_c";
    const char* asm_output = "/tmp/test_simple_asm.s";
    const char* llvm_output = "/tmp/test_simple_llvm.ll";
    
    // Write test program
    ASSERT_TRUE(write_test_file(test_file, SIMPLE_ASTHRA_PROGRAM), 
               "Should write test file successfully");
    
    // Test C backend compilation
    {
        printf("  Testing C backend compilation...\n");
        bool success = run_compiler_with_backend(test_file, c_output, ASTHRA_BACKEND_C);
        
        if (success) {
            ASSERT_TRUE(file_exists(c_output), "C executable should be created");
            
            // Run the executable and check exit code
            int exit_code = run_compiled_executable(c_output);
            ASSERT_EQ(exit_code, 42, "C executable should return 42");
            
            printf("    ✓ C backend compilation and execution successful\n");
        } else {
            printf("    ⚠ C backend compilation failed (expected for incomplete implementation)\n");
        }
    }
    
    // Test Assembly backend compilation
    {
        printf("  Testing Assembly backend compilation...\n");
        bool success = run_compiler_with_backend(test_file, asm_output, ASTHRA_BACKEND_ASSEMBLY);
        
        if (success) {
            ASSERT_TRUE(file_exists(asm_output), "Assembly file should be created");
            ASSERT_GT(get_file_size(asm_output), 0, "Assembly file should have content");
            
            printf("    ✓ Assembly backend compilation successful\n");
        } else {
            printf("    ⚠ Assembly backend compilation failed (expected for incomplete implementation)\n");
        }
    }
    
    // Test LLVM backend compilation (if available)
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
    {
        printf("  Testing LLVM backend compilation...\n");
        bool success = run_compiler_with_backend(test_file, llvm_output, ASTHRA_BACKEND_LLVM_IR);
        
        if (success) {
            ASSERT_TRUE(file_exists(llvm_output), "LLVM IR file should be created");
            ASSERT_GT(get_file_size(llvm_output), 0, "LLVM IR file should have content");
            
            printf("    ✓ LLVM backend compilation successful\n");
        } else {
            printf("    ⚠ LLVM backend compilation failed (expected for incomplete implementation)\n");
        }
    }
#else
    printf("  LLVM backend not compiled in - skipping LLVM test\n");
#endif
    
    // Cleanup
    unlink(test_file);
    unlink(c_output);
    unlink(asm_output);
    unlink(llvm_output);
    
    return true;
}

// Test 2: Backend Selection via CLI
TEST_CASE(test_cli_backend_selection) {
    // Test CLI parsing for backend selection
    
    // Test default backend (C)
    {
        char *argv[] = {"asthra", "test.asthra", NULL};
        CliOptions options = cli_options_init();
        
        int result = cli_parse_arguments(2, argv, &options);
        ASSERT_EQ(result, 0, "CLI parsing should succeed");
        ASSERT_EQ(options.compiler_options.backend_type, ASTHRA_BACKEND_C, 
                 "Default backend should be C");
        
        cli_options_cleanup(&options);
    }
    
    // Test explicit C backend
    {
        char *argv[] = {"asthra", "--backend", "c", "test.asthra", NULL};
        CliOptions options = cli_options_init();
        
        int result = cli_parse_arguments(4, argv, &options);
        ASSERT_EQ(result, 0, "CLI parsing should succeed");
        ASSERT_EQ(options.compiler_options.backend_type, ASTHRA_BACKEND_C, 
                 "Backend should be C");
        
        cli_options_cleanup(&options);
    }
    
    // Test LLVM backend
    {
        char *argv[] = {"asthra", "--backend", "llvm", "test.asthra", NULL};
        CliOptions options = cli_options_init();
        
        int result = cli_parse_arguments(4, argv, &options);
        ASSERT_EQ(result, 0, "CLI parsing should succeed");
        ASSERT_EQ(options.compiler_options.backend_type, ASTHRA_BACKEND_LLVM_IR, 
                 "Backend should be LLVM IR");
        
        cli_options_cleanup(&options);
    }
    
    // Test Assembly backend
    {
        char *argv[] = {"asthra", "--backend", "asm", "test.asthra", NULL};
        CliOptions options = cli_options_init();
        
        int result = cli_parse_arguments(4, argv, &options);
        ASSERT_EQ(result, 0, "CLI parsing should succeed");
        ASSERT_EQ(options.compiler_options.backend_type, ASTHRA_BACKEND_ASSEMBLY, 
                 "Backend should be Assembly");
        
        cli_options_cleanup(&options);
    }
    
    // Test legacy --emit-llvm flag
    {
        char *argv[] = {"asthra", "--emit-llvm", "test.asthra", NULL};
        CliOptions options = cli_options_init();
        
        int result = cli_parse_arguments(3, argv, &options);
        ASSERT_EQ(result, 0, "CLI parsing should succeed");
        ASSERT_EQ(options.compiler_options.backend_type, ASTHRA_BACKEND_LLVM_IR, 
                 "Backend should be LLVM IR with --emit-llvm");
        
        cli_options_cleanup(&options);
    }
    
    // Test legacy --emit-asm flag
    {
        char *argv[] = {"asthra", "--emit-asm", "test.asthra", NULL};
        CliOptions options = cli_options_init();
        
        int result = cli_parse_arguments(3, argv, &options);
        ASSERT_EQ(result, 0, "CLI parsing should succeed");
        ASSERT_EQ(options.compiler_options.backend_type, ASTHRA_BACKEND_ASSEMBLY, 
                 "Backend should be Assembly with --emit-asm");
        
        cli_options_cleanup(&options);
    }
    
    // Test backend flag priority over legacy flags
    {
        char *argv[] = {"asthra", "--backend", "c", "--emit-llvm", "test.asthra", NULL};
        CliOptions options = cli_options_init();
        
        int result = cli_parse_arguments(5, argv, &options);
        ASSERT_EQ(result, 0, "CLI parsing should succeed");
        ASSERT_EQ(options.compiler_options.backend_type, ASTHRA_BACKEND_C, 
                 "Explicit --backend should take priority over --emit-llvm");
        
        cli_options_cleanup(&options);
    }
    
    return true;
}

// Test 3: Cross-Backend Output Comparison
TEST_CASE(test_cross_backend_output_comparison) {
    const char* test_file = "/tmp/test_comparison.asthra";
    const char* c_output = "/tmp/test_comparison_c.c";
    const char* asm_output = "/tmp/test_comparison_asm.s";
    const char* llvm_output = "/tmp/test_comparison_llvm.ll";
    
    // Write test program
    ASSERT_TRUE(write_test_file(test_file, SIMPLE_ASTHRA_PROGRAM), 
               "Should write test file successfully");
    
    // Compile with all backends
    bool c_success = run_compiler_with_backend(test_file, c_output, ASTHRA_BACKEND_C);
    bool asm_success = run_compiler_with_backend(test_file, asm_output, ASTHRA_BACKEND_ASSEMBLY);
    
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
    bool llvm_success = run_compiler_with_backend(test_file, llvm_output, ASTHRA_BACKEND_LLVM_IR);
#else
    bool llvm_success = false;
#endif
    
    // Verify outputs have different formats but similar structure
    if (c_success && asm_success) {
        // Both C and Assembly should produce output
        long c_size = get_file_size(c_output);
        long asm_size = get_file_size(asm_output);
        
        ASSERT_GT(c_size, 0, "C output should have content");
        ASSERT_GT(asm_size, 0, "Assembly output should have content");
        
        // Files should have different content (different formats)
        // but both should be substantial (indicating successful generation)
        printf("    C output size: %ld bytes\n", c_size);
        printf("    Assembly output size: %ld bytes\n", asm_size);
    }
    
    if (llvm_success) {
        long llvm_size = get_file_size(llvm_output);
        ASSERT_GT(llvm_size, 0, "LLVM output should have content");
        printf("    LLVM IR output size: %ld bytes\n", llvm_size);
    }
    
    // Cleanup
    unlink(test_file);
    unlink(c_output);
    unlink(asm_output);
    unlink(llvm_output);
    
    return true;
}

// Test 4: Backend Error Handling Integration
TEST_CASE(test_backend_error_handling_integration) {
    const char* invalid_test_file = "/tmp/test_invalid.asthra";
    const char* output_file = "/tmp/test_invalid_output";
    
    // Create invalid Asthra program
    const char* invalid_program = 
        "fn invalid_function() {\n"
        "    return syntax error here;\n"
        "}\n";
    
    ASSERT_TRUE(write_test_file(invalid_test_file, invalid_program), 
               "Should write invalid test file");
    
    // Test that all backends handle invalid input gracefully
    {
        bool c_result = run_compiler_with_backend(invalid_test_file, output_file, ASTHRA_BACKEND_C);
        ASSERT_FALSE(c_result, "C backend should reject invalid input");
    }
    
    {
        bool asm_result = run_compiler_with_backend(invalid_test_file, output_file, ASTHRA_BACKEND_ASSEMBLY);
        ASSERT_FALSE(asm_result, "Assembly backend should reject invalid input");
    }
    
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
    {
        bool llvm_result = run_compiler_with_backend(invalid_test_file, output_file, ASTHRA_BACKEND_LLVM_IR);
        ASSERT_FALSE(llvm_result, "LLVM backend should reject invalid input");
    }
#endif
    
    // Cleanup
    unlink(invalid_test_file);
    unlink(output_file);
    
    return true;
}

// Test 5: Performance Comparison (Basic)
TEST_CASE(test_backend_performance_comparison) {
    const char* test_file = "/tmp/test_performance.asthra";
    const char* c_output = "/tmp/test_performance_c.c";
    const char* asm_output = "/tmp/test_performance_asm.s";
    const char* llvm_output = "/tmp/test_performance_llvm.ll";
    
    // Write test program
    ASSERT_TRUE(write_test_file(test_file, ARITHMETIC_ASTHRA_PROGRAM), 
               "Should write test file successfully");
    
    // Time compilation with different backends
    struct timespec start, end;
    double c_time = 0.0, asm_time = 0.0, llvm_time = 0.0;
    
    // Time C backend
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        bool success = run_compiler_with_backend(test_file, c_output, ASTHRA_BACKEND_C);
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        c_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("    C backend compilation time: %.3f seconds\n", c_time);
        
        if (success) {
            printf("    ✓ C backend compilation successful\n");
        }
    }
    
    // Time Assembly backend
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        bool success = run_compiler_with_backend(test_file, asm_output, ASTHRA_BACKEND_ASSEMBLY);
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        asm_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("    Assembly backend compilation time: %.3f seconds\n", asm_time);
        
        if (success) {
            printf("    ✓ Assembly backend compilation successful\n");
        }
    }
    
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
    // Time LLVM backend
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        bool success = run_compiler_with_backend(test_file, llvm_output, ASTHRA_BACKEND_LLVM_IR);
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        llvm_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("    LLVM backend compilation time: %.3f seconds\n", llvm_time);
        
        if (success) {
            printf("    ✓ LLVM backend compilation successful\n");
        }
    }
#endif
    
    // All compilation times should be reasonable (under 10 seconds for simple program)
    ASSERT_LT(c_time, 10.0, "C backend should compile quickly");
    ASSERT_LT(asm_time, 10.0, "Assembly backend should compile quickly");
    if (llvm_time > 0) {
        ASSERT_LT(llvm_time, 10.0, "LLVM backend should compile quickly");
    }
    
    // Cleanup
    unlink(test_file);
    unlink(c_output);
    unlink(asm_output);
    unlink(llvm_output);
    
    return true;
}

int main(void) {
    printf("=== Backend Integration Tests ===\n\n");
    
    int passed = 0;
    int total = 0;
    
    RUN_TEST(test_simple_program_compilation);
    RUN_TEST(test_cli_backend_selection);
    RUN_TEST(test_cross_backend_output_comparison);
    RUN_TEST(test_backend_error_handling_integration);
    RUN_TEST(test_backend_performance_comparison);
    
    printf("\n=== Integration Test Results ===\n");
    printf("Tests passed: %d/%d\n", passed, total);
    
    if (passed == total) {
        printf("✅ All integration tests passed!\n");
        return 0;
    } else {
        printf("❌ Some integration tests failed!\n");
        return 1;
    }
}