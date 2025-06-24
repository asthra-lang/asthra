#include "bdd_test_framework.h"
#include "bdd_utilities.h"
#include <sys/stat.h>
#include <unistd.h>

// Test framework implementation
int bdd_run_test_suite(const char* feature_name, 
                       BddTestCase* test_cases,
                       int test_count,
                       void (*cleanup_function)(void)) {
    bdd_init(feature_name);
    
    // Check if @wip scenarios should be skipped
    int skip_wip = bdd_should_skip_wip();
    
    for (int i = 0; i < test_count; i++) {
        if (skip_wip && test_cases[i].is_wip) {
            // Skip @wip scenarios
            continue;
        }
        
        bdd_run_test_case(&test_cases[i]);
    }
    
    // Run cleanup if provided
    if (cleanup_function) {
        cleanup_function();
    }
    
    return bdd_report();
}

// Result utilities
void bdd_init_result(BddResult* result) {
    if (!result) return;
    
    result->success = 1;
    result->error_message = NULL;
    result->error_count = 0;
    result->specific_data = NULL;
}

void bdd_cleanup_result(BddResult* result) {
    if (!result) return;
    
    bdd_cleanup_string(&result->error_message);
    result->specific_data = NULL;
}

// Test case utilities
void bdd_run_test_case(BddTestCase* test_case) {
    if (!test_case || !test_case->function) return;
    
    test_case->function();
}

int bdd_should_skip_test_case(BddTestCase* test_case) {
    if (!test_case) return 1;
    
    return bdd_should_skip_wip() && test_case->is_wip;
}

// Common scenario patterns
void bdd_run_compilation_scenario(const char* scenario_name,
                                  const char* filename,
                                  const char* source_code,
                                  int should_succeed,
                                  const char* expected_output_or_error) {
    bdd_scenario(scenario_name);
    
    // Given: Compiler is available
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    // Given: Source file with content
    char desc[256];
    snprintf(desc, sizeof(desc), "I have a file \"%s\" with content", filename);
    bdd_given(desc);
    bdd_create_temp_source_file(filename, source_code);
    
    // When: Compile the file
    bdd_when("I compile the file");
    char* executable = strdup(bdd_get_temp_source_file());
    char* dot = strrchr(executable, '.');
    if (dot) *dot = '\0';
    
    int exit_code = bdd_compile_source_file(bdd_get_temp_source_file(), executable, NULL);
    
    // Then: Check compilation result
    if (should_succeed) {
        bdd_then("the compilation should succeed");
        BDD_ASSERT_EQ(exit_code, 0);
        
        bdd_then("an executable should be created");
        struct stat st;
        int exists = (stat(executable, &st) == 0);
        BDD_ASSERT_TRUE(exists);
        if (exists) {
            BDD_ASSERT_TRUE(st.st_mode & S_IXUSR);
        }
    } else {
        bdd_then("the compilation should fail");
        BDD_ASSERT_NE(exit_code, 0);
        
        if (expected_output_or_error) {
            char desc[256];
            snprintf(desc, sizeof(desc), "the error message should contain \"%s\"", expected_output_or_error);
            bdd_then(desc);
            // Note: Error checking would need access to compiler output
            // This could be enhanced with additional utilities
        }
    }
    
    free(executable);
}

void bdd_run_execution_scenario(const char* scenario_name,
                                const char* filename,
                                const char* source_code,
                                const char* expected_output,
                                int expected_exit_code) {
    bdd_scenario(scenario_name);
    
    // Given: Compiler is available
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    // Given: Source file with content
    char desc[256];
    snprintf(desc, sizeof(desc), "I have a file \"%s\" with content", filename);
    bdd_given(desc);
    bdd_create_temp_source_file(filename, source_code);
    
    // When: Compile the file
    bdd_when("I compile the file");
    char* executable = strdup(bdd_get_temp_source_file());
    char* dot = strrchr(executable, '.');
    if (dot) *dot = '\0';
    
    int compile_exit_code = bdd_compile_source_file(bdd_get_temp_source_file(), executable, NULL);
    
    // Then: Compilation should succeed
    bdd_then("the compilation should succeed");
    BDD_ASSERT_EQ(compile_exit_code, 0);
    
    bdd_then("an executable should be created");
    struct stat st;
    int exists = (stat(executable, &st) == 0);
    BDD_ASSERT_TRUE(exists);
    
    // When: Run the executable
    bdd_when("I run the executable");
    char command[512];
    snprintf(command, sizeof(command), "./%s 2>&1", executable);
    
    int execution_exit_code;
    char* execution_output = bdd_execute_command(command, &execution_exit_code);
    
    // Then: Check output and exit code
    if (expected_output) {
        snprintf(desc, sizeof(desc), "the output should contain \"%s\"", expected_output);
        bdd_then(desc);
        bdd_assert_output_contains(execution_output, expected_output);
    }
    
    snprintf(desc, sizeof(desc), "the exit code should be %d", expected_exit_code);
    bdd_then(desc);
    BDD_ASSERT_EQ(execution_exit_code, expected_exit_code);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}