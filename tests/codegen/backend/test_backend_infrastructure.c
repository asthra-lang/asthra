/**
 * Tests for Asthra Backend Infrastructure
 * Validates backend selection, initialization, and basic operations
 */

#include "../../../src/codegen/backend_interface.h"
#include "../../../src/compiler.h"
#include "../../framework/test_framework.h"
#include "../../framework/test_assertions.h"
#include "../../framework/test_statistics.h"
#include <string.h>

// Test function typedef
typedef AsthraTestResult (*AsthraTestFunction)(AsthraTestContext*);

// Test backend type selection from compiler options
static AsthraTestResult test_backend_selection(AsthraTestContext *context) {
    // Test default backend (C)
    {
        AsthraCompilerOptions options = asthra_compiler_default_options();
        AsthraBackend *backend = asthra_backend_create(&options);
        if (!asthra_test_assert_not_null(context, backend, "Backend should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_int_eq(context, backend->type, ASTHRA_BACKEND_C, "Backend type should be C")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        asthra_backend_destroy(backend);
    }
    
    // Test LLVM backend selection
    {
        AsthraCompilerOptions options = asthra_compiler_default_options();
        options.emit_llvm = true;
        AsthraBackend *backend = asthra_backend_create(&options);
        if (!asthra_test_assert_not_null(context, backend, "Backend should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_int_eq(context, backend->type, ASTHRA_BACKEND_LLVM_IR, "Backend type should be LLVM IR")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        asthra_backend_destroy(backend);
    }
    
    // Test Assembly backend selection
    {
        AsthraCompilerOptions options = asthra_compiler_default_options();
        options.emit_asm = true;
        AsthraBackend *backend = asthra_backend_create(&options);
        if (!asthra_test_assert_not_null(context, backend, "Backend should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_int_eq(context, backend->type, ASTHRA_BACKEND_ASSEMBLY, "Backend type should be Assembly")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        asthra_backend_destroy(backend);
    }
    
    // Test explicit backend type selection
    {
        AsthraCompilerOptions options = asthra_compiler_default_options();
        options.backend_type = ASTHRA_BACKEND_ASSEMBLY;
        AsthraBackend *backend = asthra_backend_create(&options);
        if (!asthra_test_assert_not_null(context, backend, "Backend should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_int_eq(context, backend->type, ASTHRA_BACKEND_ASSEMBLY, "Backend type should be Assembly")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        asthra_backend_destroy(backend);
    }
    
    return ASTHRA_TEST_PASS;
}

// Test backend initialization
static AsthraTestResult test_backend_initialization(AsthraTestContext *context) {
    AsthraCompilerOptions options = asthra_compiler_default_options();
    
    // Test C backend initialization
    {
        AsthraBackend *backend = asthra_backend_create_by_type(ASTHRA_BACKEND_C);
        if (!asthra_test_assert_not_null(context, backend, "Backend should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        
        int result = asthra_backend_initialize(backend, &options);
        if (!asthra_test_assert_int_eq(context, result, 0, "C backend initialization should succeed")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        
        asthra_backend_destroy(backend);
    }
    
    // Test Assembly backend initialization
    {
        AsthraBackend *backend = asthra_backend_create_by_type(ASTHRA_BACKEND_ASSEMBLY);
        if (!asthra_test_assert_not_null(context, backend, "Backend should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        
        int result = asthra_backend_initialize(backend, &options);
        if (!asthra_test_assert_int_eq(context, result, 0, "Assembly backend initialization should succeed")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        
        asthra_backend_destroy(backend);
    }
    
    // Test LLVM backend initialization (should fail as it's not implemented)
    {
        AsthraBackend *backend = asthra_backend_create_by_type(ASTHRA_BACKEND_LLVM_IR);
        if (!asthra_test_assert_not_null(context, backend, "Backend should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        
        int result = asthra_backend_initialize(backend, &options);
        if (!asthra_test_assert_bool(context, result != 0, "LLVM backend initialization should fail (not implemented)")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        
        const char *error = asthra_backend_get_last_error(backend);
        if (!asthra_test_assert_not_null(context, (void*)error, "Error message should not be null")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_bool(context, strstr(error, "not yet implemented") != NULL, "Error should mention not implemented")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        
        asthra_backend_destroy(backend);
    }
    
    return ASTHRA_TEST_PASS;
}

// Test backend feature support
static AsthraTestResult test_backend_features(AsthraTestContext *context) {
    AsthraCompilerOptions options = asthra_compiler_default_options();
    
    // Test C backend features
    {
        AsthraBackend *backend = asthra_backend_create_by_type(ASTHRA_BACKEND_C);
        if (!asthra_test_assert_not_null(context, backend, "Backend should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        asthra_backend_initialize(backend, &options);
        
        // Should support basic features
        if (!asthra_test_assert_bool(context, asthra_backend_supports_feature(backend, "functions"), 
                                    "C backend should support functions")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_bool(context, asthra_backend_supports_feature(backend, "expressions"), 
                                    "C backend should support expressions")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_bool(context, asthra_backend_supports_feature(backend, "statements"), 
                                    "C backend should support statements")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_bool(context, asthra_backend_supports_feature(backend, "string_literals"), 
                                    "C backend should support string literals")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        
        // Should not support unknown features
        if (!asthra_test_assert_bool(context, !asthra_backend_supports_feature(backend, "unknown_feature"), 
                                    "C backend should not support unknown features")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        
        asthra_backend_destroy(backend);
    }
    
    // Test Assembly backend features
    {
        AsthraBackend *backend = asthra_backend_create_by_type(ASTHRA_BACKEND_ASSEMBLY);
        if (!asthra_test_assert_not_null(context, backend, "Backend should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        asthra_backend_initialize(backend, &options);
        
        // Should support assembly-specific features
        if (!asthra_test_assert_bool(context, asthra_backend_supports_feature(backend, "register_allocation"), 
                                    "Assembly backend should support register allocation")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_bool(context, asthra_backend_supports_feature(backend, "x86_64"), 
                                    "Assembly backend should support x86_64")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_bool(context, asthra_backend_supports_feature(backend, "arm64"), 
                                    "Assembly backend should support arm64")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        
        asthra_backend_destroy(backend);
    }
    
    return ASTHRA_TEST_PASS;
}

// Test backend information functions
static AsthraTestResult test_backend_info(AsthraTestContext *context) {
    // Test C backend info
    {
        AsthraBackend *backend = asthra_backend_create_by_type(ASTHRA_BACKEND_C);
        if (!asthra_test_assert_not_null(context, backend, "Backend should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        
        const char *name = asthra_backend_get_name(backend);
        if (!asthra_test_assert_not_null(context, (void*)name, "Backend name should not be null")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_bool(context, strstr(name, "C") != NULL, "Backend name should contain 'C'")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        
        const char *version = asthra_backend_get_version(backend);
        if (!asthra_test_assert_not_null(context, (void*)version, "Backend version should not be null")) {
            asthra_backend_destroy(backend);
            return ASTHRA_TEST_FAIL;
        }
        
        asthra_backend_destroy(backend);
    }
    
    // Test backend type string function
    {
        if (!asthra_test_assert_string_eq(context, asthra_get_backend_type_string(ASTHRA_BACKEND_C), "C", 
                                         "Backend type string for C should be 'C'")) {
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_string_eq(context, asthra_get_backend_type_string(ASTHRA_BACKEND_LLVM_IR), "LLVM IR", 
                                         "Backend type string for LLVM IR should be 'LLVM IR'")) {
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_string_eq(context, asthra_get_backend_type_string(ASTHRA_BACKEND_ASSEMBLY), "Assembly", 
                                         "Backend type string for Assembly should be 'Assembly'")) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

// Test output filename generation
static AsthraTestResult test_output_filename_generation(AsthraTestContext *context) {
    // Test C backend
    {
        char *output = asthra_backend_get_output_filename(ASTHRA_BACKEND_C, 
                                                          "test.asthra", NULL);
        if (!asthra_test_assert_not_null(context, output, "Output filename should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_string_eq(context, output, "test.c", "C backend should generate .c extension")) {
            free(output);
            return ASTHRA_TEST_FAIL;
        }
        free(output);
    }
    
    // Test LLVM backend
    {
        char *output = asthra_backend_get_output_filename(ASTHRA_BACKEND_LLVM_IR, 
                                                          "test.asthra", NULL);
        if (!asthra_test_assert_not_null(context, output, "Output filename should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_string_eq(context, output, "test.ll", "LLVM backend should generate .ll extension")) {
            free(output);
            return ASTHRA_TEST_FAIL;
        }
        free(output);
    }
    
    // Test Assembly backend
    {
        char *output = asthra_backend_get_output_filename(ASTHRA_BACKEND_ASSEMBLY, 
                                                          "test.asthra", NULL);
        if (!asthra_test_assert_not_null(context, output, "Output filename should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_string_eq(context, output, "test.s", "Assembly backend should generate .s extension")) {
            free(output);
            return ASTHRA_TEST_FAIL;
        }
        free(output);
    }
    
    // Test with explicit output file
    {
        char *output = asthra_backend_get_output_filename(ASTHRA_BACKEND_C, 
                                                          "test.asthra", "output.txt");
        if (!asthra_test_assert_not_null(context, output, "Output filename should not be null")) {
            return ASTHRA_TEST_FAIL;
        }
        if (!asthra_test_assert_string_eq(context, output, "output.txt", "Should use explicit output filename")) {
            free(output);
            return ASTHRA_TEST_FAIL;
        }
        free(output);
    }
    
    return ASTHRA_TEST_PASS;
}

// Test backend file extensions
static AsthraTestResult test_backend_file_extensions(AsthraTestContext *context) {
    if (!asthra_test_assert_string_eq(context, asthra_backend_get_file_extension(ASTHRA_BACKEND_C), "c",
                                     "C backend extension should be 'c'")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_string_eq(context, asthra_backend_get_file_extension(ASTHRA_BACKEND_LLVM_IR), "ll",
                                     "LLVM backend extension should be 'll'")) {
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_string_eq(context, asthra_backend_get_file_extension(ASTHRA_BACKEND_ASSEMBLY), "s",
                                     "Assembly backend extension should be 's'")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST RUNNER
// =============================================================================

int main(void) {
    printf("=== Backend Infrastructure Tests ===\n\n");

    // Test case definitions
    struct {
        const char* test_name;
        AsthraTestFunction test_func;
    } test_cases[] = {
        {"Backend Selection from Options", test_backend_selection},
        {"Backend Initialization", test_backend_initialization},
        {"Backend Feature Support", test_backend_features},
        {"Backend Information Functions", test_backend_info},
        {"Output Filename Generation", test_output_filename_generation},
        {"Backend File Extensions", test_backend_file_extensions},
        {NULL, NULL}
    };

    // Create test statistics
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Run all tests
    for (int i = 0; test_cases[i].test_func != NULL; i++) {
        AsthraTestMetadata metadata = {
            .name = test_cases[i].test_name,
            .file = __FILE__,
            .line = __LINE__,
            .function = test_cases[i].test_name,
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
            .timeout_ns = 30000000000ULL,
            .skip = false,
            .skip_reason = NULL
        };

        AsthraTestResult result = asthra_test_run_single(test_cases[i].test_func, &metadata, stats);
        printf("[%s] %s\n", 
               result == ASTHRA_TEST_PASS ? "PASS" : "FAIL", 
               test_cases[i].test_name);
    }

    // Print results
    printf("\nTest Results: %llu/%llu passed\n", 
           (unsigned long long)stats->tests_passed, (unsigned long long)stats->tests_run);

    bool success = (stats->tests_failed == 0);
    asthra_test_statistics_destroy(stats);
    return success ? 0 : 1;
}