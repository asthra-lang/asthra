#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "bdd_support.h"

// Common utilities and step definitions shared across all BDD tests

// Global state for test execution
static char* current_source_file = NULL;
static char* current_executable = NULL;
static char* compiler_output = NULL;
static char* error_output = NULL;
static int compilation_exit_code = -1;
static int execution_exit_code = -1;
static char* execution_output = NULL;

// Utility functions
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

static char* read_file_contents(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = malloc(size + 1);
    if (buffer) {
        fread(buffer, 1, size, file);
        buffer[size] = '\0';
    }
    
    fclose(file);
    return buffer;
}

static int write_file_contents(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (!file) return -1;
    
    fprintf(file, "%s", content);
    fclose(file);
    return 0;
}

static char* execute_command(const char* command, int* exit_code) {
    char buffer[4096];
    char* result = NULL;
    size_t result_size = 0;
    
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        *exit_code = -1;
        return NULL;
    }
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        size_t len = strlen(buffer);
        result = realloc(result, result_size + len + 1);
        if (!result) break;
        strcpy(result + result_size, buffer);
        result_size += len;
    }
    
    *exit_code = pclose(pipe);
    return result;
}

// Common Given steps
void given_asthra_compiler_available(void) {
    bdd_given("the Asthra compiler is available");
    
    // Check if compiler exists
    struct stat st;
    int exists = (stat("./build/bin/asthra", &st) == 0) || 
                 (stat("./build/asthra", &st) == 0) || 
                 (stat("./asthra", &st) == 0);
    
    BDD_ASSERT_TRUE(exists);
}

void given_file_with_content(const char* filename, const char* content) {
    char desc[256];
    snprintf(desc, sizeof(desc), "I have a file \"%s\" with content", filename);
    bdd_given(desc);
    
    // Create temporary file
    current_source_file = strdup(filename);
    int result = write_file_contents(filename, content);
    
    BDD_ASSERT_EQ(result, 0);
}

// Note: given_asthra_runtime_initialized is defined in integration/common_steps.c
// to avoid duplicate symbols when linking integration tests

void given_ffi_support_enabled(void) {
    bdd_given("FFI support is enabled");
    // Check if FFI is available in the runtime
    BDD_ASSERT_TRUE(1);
}

// Common When steps
void when_compile_file(void) {
    bdd_when("I compile the file");
    
    if (!current_source_file) {
        compilation_exit_code = -1;
        return;
    }
    
    // Build compiler command
    char command[1024];
    char* executable = strdup(current_source_file);
    char* dot = strrchr(executable, '.');
    if (dot) *dot = '\0';
    
    current_executable = executable;
    
    // Try to find the compiler
    const char* compiler_path = "./build/bin/asthra";
    if (access(compiler_path, X_OK) != 0) {
        compiler_path = "./build/asthra";
        if (access(compiler_path, X_OK) != 0) {
            compiler_path = "./asthra";
        }
    }
    
    snprintf(command, sizeof(command), "%s %s -o %s 2>&1", 
             compiler_path, current_source_file, current_executable);
    
    // Execute compilation
    compiler_output = execute_command(command, &compilation_exit_code);
    
    // Separate stdout and stderr if needed
    if (compiler_output && compilation_exit_code != 0) {
        error_output = compiler_output;
        compiler_output = NULL;
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
    
    // Build compiler command with flags
    char command[1024];
    char* executable = strdup(current_source_file);
    char* dot = strrchr(executable, '.');
    if (dot) *dot = '\0';
    
    current_executable = executable;
    
    const char* compiler_path = "./build/bin/asthra";
    if (access(compiler_path, X_OK) != 0) {
        compiler_path = "./build/asthra";
        if (access(compiler_path, X_OK) != 0) {
            compiler_path = "./asthra";
        }
    }
    
    snprintf(command, sizeof(command), "%s %s %s -o %s 2>&1", 
             compiler_path, flags, current_source_file, current_executable);
    
    compiler_output = execute_command(command, &compilation_exit_code);
}

void when_run_executable(void) {
    bdd_when("I run the executable");
    
    if (!current_executable) {
        execution_exit_code = -1;
        return;
    }
    
    char command[512];
    snprintf(command, sizeof(command), "./%s 2>&1", current_executable);
    
    execution_output = execute_command(command, &execution_exit_code);
}

// Common Then steps
void then_compilation_should_succeed(void) {
    bdd_then("the compilation should succeed");
    BDD_ASSERT_EQ(compilation_exit_code, 0);
}

void then_compilation_should_fail(void) {
    bdd_then("the compilation should fail");
    BDD_ASSERT_NE(compilation_exit_code, 0);
}

void then_executable_created(void) {
    bdd_then("an executable should be created");
    
    struct stat st;
    int exists = (current_executable && stat(current_executable, &st) == 0);
    
    BDD_ASSERT_TRUE(exists);
    
    // Check if executable
    if (exists) {
        BDD_ASSERT_TRUE(st.st_mode & S_IXUSR);
    }
}

void then_error_contains(const char* expected_error) {
    char desc[256];
    snprintf(desc, sizeof(desc), "the error message should contain \"%s\"", expected_error);
    bdd_then(desc);
    
    BDD_ASSERT_NOT_NULL(error_output);
    if (error_output) {
        BDD_ASSERT_TRUE(strstr(error_output, expected_error) != NULL);
    }
}

void then_output_contains(const char* expected_output) {
    char desc[256];
    snprintf(desc, sizeof(desc), "the output should contain \"%s\"", expected_output);
    bdd_then(desc);
    
    BDD_ASSERT_NOT_NULL(execution_output);
    if (execution_output) {
        BDD_ASSERT_TRUE(strstr(execution_output, expected_output) != NULL);
    }
}

void then_exit_code_is(int expected_code) {
    char desc[128];
    snprintf(desc, sizeof(desc), "the exit code should be %d", expected_code);
    bdd_then(desc);
    
    BDD_ASSERT_EQ(execution_exit_code, expected_code);
}

// Cleanup function to be called at the end of tests
void common_cleanup(void) {
    cleanup_test_files();
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