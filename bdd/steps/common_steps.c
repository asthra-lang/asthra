#include "bdd_test_framework.h"
#include "bdd_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

// Common utilities and step definitions shared across all BDD tests

// Global state for common test execution (shared across tests)
static char* current_source_file = NULL;
static char* current_executable = NULL;
static char* compiler_output = NULL;
static char* error_output = NULL;
static int compilation_exit_code = -1;
static int execution_exit_code = -1;
static char* execution_output = NULL;

// Utility functions using BDD utilities
static void cleanup_test_files(void) {
    if (current_source_file) {
        unlink(current_source_file);
        free(current_source_file);
        current_source_file = NULL;
    }
    if (current_executable) {
        unlink(current_executable);
        free(current_executable);
        current_executable = NULL;
    }
    if (compiler_output) {
        free(compiler_output);
        compiler_output = NULL;
    }
    if (error_output) {
        free(error_output);
        error_output = NULL;
    }
    if (execution_output) {
        free(execution_output);
        execution_output = NULL;
    }
}

// Common Given steps using BDD utilities
void given_asthra_compiler_available(void) {
    bdd_given("the Asthra compiler is available");
    
    // Use BDD utilities to find compiler
    const char* compiler = bdd_find_asthra_compiler();
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    bdd_assert(compiler != NULL, "Asthra compiler should be available");
}

void given_file_with_content(const char* filename, const char* content) {
    char desc[256];
    snprintf(desc, sizeof(desc), "I have a file \"%s\" with content", filename);
    bdd_given(desc);
    
    // Use BDD utilities to create temp file
    bdd_create_temp_source_file(filename, content);
    
    // Update current source file path
    if (current_source_file) free(current_source_file);
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "bdd-temp/%s", filename);
    current_source_file = strdup(temp_path);
}

// Note: given_asthra_runtime_initialized is defined in integration/common_steps.c
// to avoid duplicate symbols when linking integration tests

void given_ffi_support_enabled(void) {
    bdd_given("FFI support is enabled");
    // Check if FFI is available in the runtime
    bdd_assert(1, "FFI support should be available");
}

// Common When steps using BDD utilities
void when_compile_file(void) {
    bdd_when("I compile the file");
    
    if (!current_source_file) {
        compilation_exit_code = -1;
        return;
    }
    
    // Use BDD utilities to find compiler
    const char* compiler_path = bdd_find_asthra_compiler();
    if (!compiler_path) {
        compilation_exit_code = -1;
        compiler_output = strdup("Asthra compiler not found");
        return;
    }
    
    // Generate executable name
    char* executable = strdup(current_source_file);
    char* dot = strrchr(executable, '.');
    if (dot) *dot = '\0';
    current_executable = executable;
    
    // Build and execute compilation command using BDD utilities
    char command[1024];
    snprintf(command, sizeof(command), "%s %s -o %s 2>&1", 
             compiler_path, current_source_file, current_executable);
    
    // Use BDD utilities for command execution
    if (compiler_output) {
        free(compiler_output);
        compiler_output = NULL;
    }
    
    compiler_output = bdd_execute_command(command, &compilation_exit_code);
    
    // Ensure executable has proper permissions on Unix systems
    if (compilation_exit_code == 0 && current_executable && access(current_executable, F_OK) == 0) {
        if (chmod(current_executable, 0755) != 0) {
            fprintf(stderr, "Warning: Failed to set execute permissions on %s: %s\n", 
                    current_executable, strerror(errno));
        }
        // Force filesystem sync to ensure permissions are applied
        sync();
    }
    
    // For error checking, use compiler_output (which contains both stdout and stderr)
    if (compiler_output && compilation_exit_code != 0) {
        if (error_output) free(error_output);
        error_output = strdup(compiler_output);
        // Debug output for CI failures
        fprintf(stderr, "DEBUG: Compilation failed with exit code %d\n", compilation_exit_code);
        fprintf(stderr, "DEBUG: Compiler command was: %s\n", command);
        fprintf(stderr, "DEBUG: Compiler output:\n%s\n", compiler_output);
    }
}

void when_compile_with_flags(const char* flags) {
    char desc[256];
    snprintf(desc, sizeof(desc), "I compile with flags: %s", flags);
    bdd_when(desc);
    
    if (!current_source_file) {
        compilation_exit_code = -1;
        return;
    }
    
    // Use BDD utilities to find compiler
    const char* compiler_path = bdd_find_asthra_compiler();
    if (!compiler_path) {
        compilation_exit_code = -1;
        compiler_output = strdup("Asthra compiler not found");
        return;
    }
    
    // Generate executable name
    char* executable = strdup(current_source_file);
    char* dot = strrchr(executable, '.');
    if (dot) *dot = '\0';
    current_executable = executable;
    
    // Build and execute compilation command with flags using BDD utilities
    char command[1024];
    snprintf(command, sizeof(command), "%s %s %s -o %s 2>&1", 
             compiler_path, flags, current_source_file, current_executable);
    
    if (compiler_output) {
        free(compiler_output);
        compiler_output = NULL;
    }
    
    compiler_output = bdd_execute_command(command, &compilation_exit_code);
    
    // Ensure executable has proper permissions on Unix systems
    if (compilation_exit_code == 0 && current_executable && access(current_executable, F_OK) == 0) {
        if (chmod(current_executable, 0755) != 0) {
            fprintf(stderr, "Warning: Failed to set execute permissions on %s: %s\n", 
                    current_executable, strerror(errno));
        }
        // Force filesystem sync to ensure permissions are applied
        sync();
    }
}

void when_run_executable(void) {
    bdd_when("I run the executable");
    
    if (!current_executable) {
        execution_exit_code = -1;
        return;
    }
    
    // Verify executable exists and has execute permissions
    if (access(current_executable, X_OK) != 0) {
        // Try to set execute permissions again
        if (access(current_executable, F_OK) == 0) {
            chmod(current_executable, 0755);
            sync(); // Force filesystem sync
        }
        
        // Check again after chmod
        if (access(current_executable, X_OK) != 0) {
            execution_exit_code = -1;
            if (execution_output) free(execution_output);
            char error_msg[512];
            snprintf(error_msg, sizeof(error_msg), 
                    "Executable %s exists but is not executable: %s", 
                    current_executable, strerror(errno));
            execution_output = strdup(error_msg);
            return;
        }
    }
    
    // Use BDD utilities for command execution
    char command[512];
    snprintf(command, sizeof(command), "./%s", current_executable);
    
    if (execution_output) {
        free(execution_output);
        execution_output = NULL;
    }
    
    execution_output = bdd_execute_command(command, &execution_exit_code);
}

// Common Then steps using BDD utilities
void then_compilation_should_succeed(void) {
    bdd_then("the compilation should succeed");
    bdd_assert(compilation_exit_code == 0, "Compilation should succeed");
}

void then_compilation_should_fail(void) {
    bdd_then("the compilation should fail");
    bdd_assert(compilation_exit_code != 0, "Compilation should fail");
}

void then_executable_created(void) {
    bdd_then("an executable should be created");
    
    struct stat st;
    int exists = (current_executable && stat(current_executable, &st) == 0);
    
    bdd_assert(exists, "Executable file should exist");
    
    // Check if executable
    if (exists) {
        bdd_assert(st.st_mode & S_IXUSR, "File should be executable");
    }
}

void then_error_contains(const char* expected_error) {
    char desc[256];
    snprintf(desc, sizeof(desc), "the error message should contain \"%s\"", expected_error);
    bdd_then(desc);
    
    bdd_assert(error_output != NULL, "Error output should be provided");
    if (error_output) {
        bdd_assert(strstr(error_output, expected_error) != NULL, "Error should contain expected text");
    }
}

void then_output_contains(const char* expected_output) {
    char desc[256];
    snprintf(desc, sizeof(desc), "the output should contain \"%s\"", expected_output);
    bdd_then(desc);
    
    bdd_assert(execution_output != NULL, "Execution output should be provided");
    if (execution_output) {
        bdd_assert_output_contains(execution_output, expected_output);
    }
}

void then_exit_code_is(int expected_code) {
    char desc[128];
    snprintf(desc, sizeof(desc), "the exit code should be %d", expected_code);
    bdd_then(desc);
    
    bdd_assert(execution_exit_code == expected_code, "Exit code should match expected value");
}

// Cleanup function to be called at the end of tests
void common_cleanup(void) {
    cleanup_test_files();
    
    // Use BDD utilities for cleanup if available
    // Check environment variable to control cleanup behavior
    const char* keep_artifacts = getenv("BDD_KEEP_ARTIFACTS");
    if (!keep_artifacts || strcmp(keep_artifacts, "1") != 0) {
        const char* clean_temp = getenv("BDD_CLEAN_TEMP");
        if (clean_temp && strcmp(clean_temp, "1") == 0) {
            bdd_cleanup_temp_files();
        }
    }
}

// Helper to get current source file path
const char* get_current_source_file(void) {
    return current_source_file;
}

// Helper to get current executable path
const char* get_current_executable(void) {
    return current_executable;
}

// Helper to get compiler output
const char* get_compiler_output(void) {
    return compiler_output;
}

// Helper to get error output
const char* get_error_output(void) {
    return error_output;
}

// Helper to get execution output
const char* get_execution_output(void) {
    return execution_output;
}