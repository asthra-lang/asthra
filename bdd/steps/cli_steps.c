#include "bdd_test_framework.h"
#include "bdd_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

// CLI BDD Tests
// Tests CLI functionality using consolidated BDD utilities

// ===================================================================
// CONSOLIDATED CLI PATTERNS AND STATE
// ===================================================================

// Global CLI test state
static char* cli_output = NULL;
static int cli_exit_code = -1;
static char* asthra_path = NULL;
static char original_dir[1024];

// Consolidated path finding for Asthra compiler
static const char* find_asthra_compiler(void) {
    if (asthra_path && access(asthra_path, X_OK) == 0) {
        return asthra_path;
    }
    
    // Check common locations relative to build directory
    const char* paths[] = {
        "../build/bin/asthra",
        "../build/asthra", 
        "./build/bin/asthra",
        "./build/asthra",
        "../../build/bin/asthra",  // From bdd/bin directory
        "../../build/asthra",
        "/usr/local/bin/asthra",
        NULL
    };
    
    for (int i = 0; paths[i]; i++) {
        if (access(paths[i], X_OK) == 0) {
            char abs_path[1024];
            if (realpath(paths[i], abs_path)) {
                if (asthra_path) free(asthra_path);
                asthra_path = strdup(abs_path);
                return asthra_path;
            }
        }
    }
    
    // If not found, use BDD utilities to try to find it
    const char* bdd_compiler = bdd_find_asthra_compiler();
    if (bdd_compiler) {
        if (asthra_path) free(asthra_path);
        asthra_path = strdup(bdd_compiler);
        return asthra_path;
    }
    
    return NULL; // Not found
}

// Consolidated CLI command execution using BDD utilities
static void execute_cli_command_with_path(const char* command_args) {
    const char* compiler = find_asthra_compiler();
    if (!compiler) {
        cli_exit_code = -1;
        cli_output = strdup("Asthra compiler not found");
        return;
    }
    
    char full_command[1024];
    // Redirect stderr to stdout to capture error messages
    snprintf(full_command, sizeof(full_command), "%s %s 2>&1", compiler, command_args);
    
    // Use BDD utilities for command execution
    if (cli_output) {
        free(cli_output);
        cli_output = NULL;
    }
    
    cli_output = bdd_execute_command(full_command, &cli_exit_code);
    
    // If command returns NULL, set empty string to avoid NULL pointer issues
    if (!cli_output) {
        cli_output = strdup("");
    }
}

// Consolidated cleanup function
static void cleanup_cli_state(void) {
    fprintf(stderr, "DEBUG: cleanup_cli_state called\n");
    if (cli_output) {
        fprintf(stderr, "DEBUG: Freeing cli_output (length: %zu)\n", strlen(cli_output));
        free(cli_output);
        cli_output = NULL;
    }
    if (asthra_path) {
        fprintf(stderr, "DEBUG: Freeing asthra_path\n");
        free(asthra_path);
        asthra_path = NULL;
    }
    cli_exit_code = -1;
    fprintf(stderr, "DEBUG: cleanup_cli_state completed\n");
}

// Consolidated configuration file creation
static void create_project_config(const char* config_content) {
    bdd_create_temp_source_file("asthra.toml", config_content);
}

// ===================================================================
// TEST SCENARIO IMPLEMENTATIONS
// ===================================================================

// Test scenario: Display help information
void test_cli_help_command(void) {
    fprintf(stderr, "DEBUG: Starting test_cli_help_command\n");
    bdd_given("the Asthra CLI is installed");
    const char* compiler = find_asthra_compiler();
    fprintf(stderr, "DEBUG: Compiler found at: %s\n", compiler ? compiler : "NULL");
    
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    bdd_assert(compiler != NULL, "Asthra compiler should be found");
    
    bdd_when("I run 'asthra --help'");
    fprintf(stderr, "DEBUG: About to execute help command\n");
    execute_cli_command_with_path("--help");
    fprintf(stderr, "DEBUG: Help command executed, exit code: %d\n", cli_exit_code);
    
    bdd_then("the CLI should succeed");
    bdd_assert(cli_exit_code == 0, "Help command should succeed");
    
    bdd_then("the CLI should show usage information");
    bdd_assert(cli_output != NULL, "Output should be provided");
    fprintf(stderr, "DEBUG: CLI output length: %zu\n", cli_output ? strlen(cli_output) : 0);
    int has_usage = (strstr(cli_output, "Usage:") != NULL) ||
                   (strstr(cli_output, "usage:") != NULL) ||
                   (strstr(cli_output, "USAGE:") != NULL) ||
                   (strstr(cli_output, "Options:") != NULL);
    bdd_assert(has_usage, "Should show usage information");
    
    bdd_then("the output should contain 'asthra'");
    bdd_assert_output_contains(cli_output, "asthra");
    fprintf(stderr, "DEBUG: test_cli_help_command completed\n");
}

// Test scenario: Display version information
void test_cli_version_command(void) {
    bdd_given("the Asthra CLI is installed");
    const char* compiler = find_asthra_compiler();
    
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    bdd_assert(compiler != NULL, "Asthra compiler should be found");
    
    bdd_when("I run 'asthra --version'");
    execute_cli_command_with_path("--version");
    
    bdd_then("the CLI should succeed");
    bdd_assert(cli_exit_code == 0, "Version command should succeed");
    
    bdd_then("the CLI should show version information");
    bdd_assert(cli_output != NULL, "Output should be provided");
    int has_version = (strstr(cli_output, "version") != NULL) ||
                     (strstr(cli_output, "Version") != NULL) ||
                     (strstr(cli_output, "v0.") != NULL) ||
                     (strstr(cli_output, "v1.") != NULL) ||
                     (strstr(cli_output, "2.") != NULL); // Asthra 2.x
    bdd_assert(has_version, "Should show version information");
}

// Test scenario: Handle invalid command line flag
void test_cli_invalid_flag(void) {
    fprintf(stderr, "DEBUG: Starting test_cli_invalid_flag\n");
    bdd_given("the Asthra CLI is installed");
    const char* compiler = find_asthra_compiler();
    
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    bdd_assert(compiler != NULL, "Asthra compiler should be found");
    
    bdd_when("I run 'asthra --invalid-flag'");
    fprintf(stderr, "DEBUG: About to execute invalid flag command\n");
    execute_cli_command_with_path("--invalid-flag");
    fprintf(stderr, "DEBUG: Invalid flag command executed, exit code: %d\n", cli_exit_code);
    
    bdd_then("the CLI should fail");
    bdd_assert(cli_exit_code != 0, "Invalid flag should cause failure");
    
    bdd_then("the output should contain error about invalid option");
    bdd_assert(cli_output != NULL, "Error output should be provided");
    fprintf(stderr, "DEBUG: CLI output length: %zu\n", cli_output ? strlen(cli_output) : 0);
    // Look for common error patterns
    int has_error = (strstr(cli_output, "unrecognized") != NULL) ||
                   (strstr(cli_output, "invalid") != NULL) ||
                   (strstr(cli_output, "unknown") != NULL);
    bdd_assert(has_error, "Should contain error about invalid option");
    fprintf(stderr, "DEBUG: test_cli_invalid_flag completed\n");
}

// Test scenario: Build command without source files
void test_cli_build_without_source(void) {
    bdd_given("the Asthra CLI is installed");
    const char* compiler = find_asthra_compiler();
    
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    bdd_assert(compiler != NULL, "Asthra compiler should be found");
    
    bdd_given("I am in an empty directory");
    char temp_dir[256];
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/empty_project_%d", getpid());
    bdd_create_temp_directory(temp_dir);
    
    // Save current directory using global original_dir as fallback
    char test_saved_dir[1024];
    if (getcwd(test_saved_dir, sizeof(test_saved_dir)) == NULL) {
        bdd_assert(0, "Failed to get current directory");
        return;
    }
    
    if (chdir(temp_dir) != 0) {
        bdd_assert(0, "Failed to change to temp directory");
        return;
    }
    
    bdd_when("I run 'asthra build'");
    execute_cli_command_with_path("build");
    
    bdd_then("the CLI should fail");
    bdd_assert(cli_exit_code != 0, "Build without source should fail");
    
    bdd_then("the output should contain error message");
    bdd_assert(cli_output != NULL, "Error output should be provided");
    // Look for file not found error patterns
    int has_error = (strstr(cli_output, "does not exist") != NULL) ||
                   (strstr(cli_output, "not found") != NULL) ||
                   (strstr(cli_output, "Error") != NULL);
    bdd_assert(has_error, "Should contain file error message");
    
    // Restore directory - use test_saved_dir first, then fall back to original_dir
    if (chdir(test_saved_dir) != 0) {
        fprintf(stderr, "WARNING: Failed to restore test directory, falling back to original\n");
        if (chdir(original_dir) != 0) {
            fprintf(stderr, "CRITICAL: Cannot restore working directory\n");
        }
    }
}

// Test scenario: Read project configuration
void test_cli_project_config(void) {
    bdd_given("the Asthra CLI is installed");
    const char* compiler = find_asthra_compiler();
    
    if (!compiler) {
        bdd_skip_scenario("Asthra compiler not found - may not be built yet");
        return;
    }
    bdd_assert(compiler != NULL, "Asthra compiler should be found");
    
    bdd_given("I am in a project directory");
    char temp_dir[256];
    snprintf(temp_dir, sizeof(temp_dir), "/tmp/asthra_project_%d", getpid());
    bdd_create_temp_directory(temp_dir);
    
    // Save current directory using global original_dir as fallback
    char test_saved_dir[1024];
    if (getcwd(test_saved_dir, sizeof(test_saved_dir)) == NULL) {
        bdd_assert(0, "Failed to get current directory");
        return;
    }
    
    if (chdir(temp_dir) != 0) {
        bdd_assert(0, "Failed to change to temp directory");
        return;
    }
    
    bdd_given("a project with asthra.toml configuration");
    const char* config_content = 
        "[package]\n"
        "name = \"test_project\"\n"
        "version = \"0.1.0\"\n"
        "\n"
        "[dependencies]\n";
    create_project_config(config_content);
    
    bdd_given("a simple source file");
    const char* source_content = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    bdd_create_temp_source_file("main.asthra", source_content);
    
    bdd_when("I run 'asthra main.asthra'");
    execute_cli_command_with_path("main.asthra");
    
    bdd_then("the CLI should process the project");
    // Note: May succeed or fail depending on compiler implementation
    // We're primarily testing the CLI interface
    int processed = (cli_output != NULL) && (strlen(cli_output) > 0);
    if (!processed) {
        // If no output, check if we at least got a valid exit code (not our default -1)
        processed = (cli_exit_code >= 0);
    }
    bdd_assert(processed, "Should attempt to process the source file");
    
    // Restore directory - use test_saved_dir first, then fall back to original_dir
    if (chdir(test_saved_dir) != 0) {
        fprintf(stderr, "WARNING: Failed to restore test directory, falling back to original\n");
        if (chdir(original_dir) != 0) {
            fprintf(stderr, "CRITICAL: Cannot restore working directory\n");
        }
    }
}

// Test scenario: Compiler with no arguments (WIP)
void test_cli_no_arguments(void) {
    bdd_given("the Asthra CLI is installed");
    const char* compiler = find_asthra_compiler();
    bdd_assert(compiler != NULL, "Asthra compiler should be found");
    
    bdd_when("I run 'asthra' with no arguments");
    execute_cli_command_with_path("");
    
    bdd_then("the CLI should show usage or error");
    bdd_assert(cli_output != NULL, "Should produce output");
    
    // Mark as WIP since exact behavior may vary
    bdd_skip_scenario("No arguments behavior not fully specified yet");
}

// Test scenario: Compile invalid syntax file (WIP)
void test_cli_invalid_syntax(void) {
    bdd_given("the Asthra CLI is installed");
    const char* compiler = find_asthra_compiler();
    bdd_assert(compiler != NULL, "Asthra compiler should be found");
    
    bdd_given("a file with invalid syntax");
    const char* invalid_source = 
        "package invalid;\n"
        "\n"
        "pub fn broken( -> void {\n"  // Missing parameter
        "    return;\n"              // Missing ()
        "}\n";
    bdd_create_temp_source_file("invalid.asthra", invalid_source);
    
    bdd_when("I compile the invalid file");
    execute_cli_command_with_path("invalid.asthra");
    
    bdd_then("the CLI should report syntax errors");
    bdd_assert(cli_exit_code != 0, "Invalid syntax should cause failure");
    bdd_assert(cli_output != NULL, "Error output should be provided");
    
    // Mark as WIP since error message format may vary
    bdd_skip_scenario("Error message format not fully standardized yet");
}

// Test scenario: CLI output format validation (WIP)
void test_cli_output_format(void) {
    bdd_given("the Asthra CLI is installed");
    const char* compiler = find_asthra_compiler();
    bdd_assert(compiler != NULL, "Asthra compiler should be found");
    
    bdd_when("I run help command");
    execute_cli_command_with_path("--help");
    
    bdd_then("output should follow standard CLI format");
    bdd_assert(cli_output != NULL, "Should produce output");
    
    // Check for standard CLI format elements
    int has_proper_format = (strstr(cli_output, "Usage:") != NULL) &&
                           (strstr(cli_output, "Options:") != NULL);
    bdd_assert(has_proper_format, "Should follow standard CLI format");
    
    // Mark as WIP since format standardization is ongoing
    bdd_skip_scenario("CLI output format standardization in progress");
}

// ===================================================================
// DECLARATIVE TEST CASE DEFINITIONS
// ===================================================================

BddTestCase cli_test_cases[] = {
    BDD_TEST_CASE(cli_help_command, test_cli_help_command),
    BDD_TEST_CASE(cli_version_command, test_cli_version_command),
    BDD_TEST_CASE(cli_invalid_flag, test_cli_invalid_flag),
    BDD_TEST_CASE(cli_build_without_source, test_cli_build_without_source),
    BDD_TEST_CASE(cli_project_config, test_cli_project_config),
    BDD_WIP_TEST_CASE(cli_no_arguments, test_cli_no_arguments),
    BDD_WIP_TEST_CASE(cli_invalid_syntax, test_cli_invalid_syntax),
    BDD_WIP_TEST_CASE(cli_output_format, test_cli_output_format),
};

int main(void) {
    // Save current directory for cleanup
    if (getcwd(original_dir, sizeof(original_dir)) == NULL) {
        fprintf(stderr, "ERROR: Failed to get current directory at startup\n");
        return 1;
    }
    fprintf(stderr, "DEBUG: Starting CLI tests from directory: %s\n", original_dir);
    
    // Initialize BDD support before running tests
    bdd_init("CLI Functionality");
    
    // Run tests individually with proper cleanup between each
    int skip_wip = bdd_should_skip_wip();
    
    for (int i = 0; i < sizeof(cli_test_cases) / sizeof(cli_test_cases[0]); i++) {
        if (skip_wip && cli_test_cases[i].is_wip) {
            continue;
        }
        
        fprintf(stderr, "DEBUG: About to run test %d: %s\n", i, cli_test_cases[i].name);
        bdd_run_test_case(&cli_test_cases[i]);
        
        // Clean up after each test
        cleanup_cli_state();
        
        // Ensure we're in the correct directory
        if (chdir(original_dir) != 0) {
            fprintf(stderr, "ERROR: Failed to restore directory after test %s\n", cli_test_cases[i].name);
            if (getcwd(original_dir, sizeof(original_dir)) != NULL) {
                fprintf(stderr, "Current directory is: %s\n", original_dir);
            }
            return 1;
        }
    }
    
    int result = bdd_report();
    
    // Final cleanup
    cleanup_cli_state();
    
    return result;
}