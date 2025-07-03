#include "bdd_utilities.h"
#include "bdd_support.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

// Global state for temp file management
static char* temp_source_file = NULL;
static char* temp_executable = NULL;
static char* temp_compiler_output = NULL;

// Command execution utilities
char* bdd_execute_command(const char* command, int* exit_code) {
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
    
    int status = pclose(pipe);
    
    // Extract the actual exit code from the wait status
    if (WIFEXITED(status)) {
        *exit_code = WEXITSTATUS(status);
    } else {
        // Process didn't exit normally (e.g., killed by signal)
        *exit_code = -1;
    }
    
    // If no output was captured, return an empty string instead of NULL
    // This prevents NULL pointer issues in test assertions
    if (result == NULL) {
        result = strdup("");
        if (!result) {
            // Memory allocation failed, return NULL
            return NULL;
        }
    }
    
    return result;
}

// File operations utilities
char* bdd_read_file_contents(const char* filename) {
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

int bdd_write_file_contents(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (!file) return -1;
    
    fprintf(file, "%s", content);
    fclose(file);
    return 0;
}

// Compiler utilities
const char* bdd_find_asthra_compiler(void) {
    static const char* compiler_paths[] = {
        "./build/bin/asthra",
        "./bin/asthra",
        "../build/bin/asthra",
        "./asthra",
        NULL
    };
    
    static int first_call = 1;
    
    for (int i = 0; compiler_paths[i] != NULL; i++) {
        if (access(compiler_paths[i], X_OK) == 0) {
            if (first_call) {
                fprintf(stderr, "BDD: Using Asthra compiler at: %s\n", compiler_paths[i]);
                first_call = 0;
            }
            return compiler_paths[i];
        }
    }
    
    if (first_call) {
        fprintf(stderr, "BDD: ERROR - Asthra compiler not found in any of these paths:\n");
        for (int i = 0; compiler_paths[i] != NULL; i++) {
            fprintf(stderr, "  - %s\n", compiler_paths[i]);
        }
        first_call = 0;
    }
    
    return NULL;
}

int bdd_compiler_available(void) {
    return bdd_find_asthra_compiler() != NULL;
}

// String cleanup utilities
void bdd_cleanup_string(char** str) {
    if (str && *str) {
        free(*str);
        *str = NULL;
    }
}

void bdd_cleanup_strings(char** strings, int count) {
    if (!strings) return;
    
    for (int i = 0; i < count; i++) {
        bdd_cleanup_string(&strings[i]);
    }
}

// Output validation utilities
int bdd_output_contains(const char* output, const char* expected) {
    if (!output || !expected) return 0;
    return strstr(output, expected) != NULL;
}

int bdd_output_not_contains(const char* output, const char* unexpected) {
    if (!output || !unexpected) return 1;
    return strstr(output, unexpected) == NULL;
}

void bdd_assert_output_contains(const char* output, const char* expected) {
    BDD_ASSERT_NOT_NULL(output);
    if (output) {
        BDD_ASSERT_TRUE(bdd_output_contains(output, expected));
    }
}

void bdd_assert_output_not_contains(const char* output, const char* unexpected) {
    BDD_ASSERT_NOT_NULL(output);
    if (output) {
        BDD_ASSERT_TRUE(bdd_output_not_contains(output, unexpected));
    }
}

// Source file management
void bdd_create_temp_directory(const char* dirname) {
    char command[1024];
    snprintf(command, sizeof(command), "mkdir -p \"%s\"", dirname);
    system(command);
}

void bdd_create_temp_source_file(const char* filename, const char* content) {
    // Ensure temp directory exists
    struct stat st = {0};
    if (stat("bdd-temp", &st) == -1) {
        mkdir("bdd-temp", 0755);
    }
    
    // Create temporary file in BDD temp directory
    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "bdd-temp/%s", filename);
    
    // Cleanup previous file if exists
    bdd_cleanup_string(&temp_source_file);
    
    temp_source_file = strdup(temp_path);
    int result = bdd_write_file_contents(temp_path, content);
    
    BDD_ASSERT_EQ(result, 0);
}

void bdd_cleanup_temp_files(void) {
    if (temp_source_file) {
        unlink(temp_source_file);
        bdd_cleanup_string(&temp_source_file);
    }
    if (temp_executable) {
        unlink(temp_executable);
        bdd_cleanup_string(&temp_executable);
    }
    if (temp_compiler_output) {
        bdd_cleanup_string(&temp_compiler_output);
    }
    
    // Check environment variable to control cleanup behavior
    const char* keep_artifacts = getenv("BDD_KEEP_ARTIFACTS");
    if (!keep_artifacts || strcmp(keep_artifacts, "1") != 0) {
        const char* clean_temp = getenv("BDD_CLEAN_TEMP");
        if (clean_temp && strcmp(clean_temp, "1") == 0) {
            system("rm -rf bdd-temp");
        }
    }
}

// Compilation utilities
int bdd_compile_source_file(const char* source_file, const char* output_file, const char* flags) {
    const char* compiler_path = bdd_find_asthra_compiler();
    if (!compiler_path) {
        return -1;
    }
    
    char command[1024];
    if (flags && strlen(flags) > 0) {
        snprintf(command, sizeof(command), "%s %s %s -o %s 2>&1", 
                 compiler_path, flags, source_file, output_file);
    } else {
        snprintf(command, sizeof(command), "%s %s -o %s 2>&1", 
                 compiler_path, source_file, output_file);
    }
    
    int exit_code;
    char* output = bdd_execute_command(command, &exit_code);
    
    // Store the compiler output globally for error checking
    bdd_cleanup_string(&temp_compiler_output);
    temp_compiler_output = output ? strdup(output) : NULL;
    
    // If compilation failed, show the compiler output for debugging
    if (exit_code != 0 && output && strlen(output) > 0) {
        fprintf(stderr, "Compilation failed with exit code %d\n", exit_code);
        fprintf(stderr, "Compiler output:\n%s\n", output);
    }
    
    // Also check for LLVM verification errors even if exit code is 0
    if (exit_code == 0 && output && strstr(output, "LLVM function verification failed")) {
        fprintf(stderr, "LLVM verification error detected (exit code was 0)\n");
        fprintf(stderr, "Compiler output:\n%s\n", output);
        exit_code = 255; // Force failure for LLVM verification errors
    }
    
    bdd_cleanup_string(&output);
    
    // Store executable path for cleanup
    bdd_cleanup_string(&temp_executable);
    temp_executable = strdup(output_file);
    
    // Ensure executable has proper permissions on Unix systems
    if (exit_code == 0 && access(output_file, F_OK) == 0) {
        if (chmod(output_file, 0755) != 0) {
            fprintf(stderr, "Warning: Failed to set execute permissions on %s: %s\n", 
                    output_file, strerror(errno));
        }
        // Force filesystem sync to ensure permissions are applied
        sync();
    }
    
    return exit_code;
}

// Get temp file paths for access by step functions
const char* bdd_get_temp_source_file(void) {
    return temp_source_file;
}

const char* bdd_get_temp_executable(void) {
    return temp_executable;
}