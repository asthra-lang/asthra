/**
 * Simplified basic compiler test
 * Tests only the high-level compiler API without full pipeline
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "compiler.h"

// Simple test functions
static bool test_compiler_version(void) {
    printf("Testing compiler version...\n");
    const char* version = asthra_compiler_get_version();
    if (!version) {
        printf("❌ Failed to get compiler version\n");
        return false;
    }
    printf("  ✓ Compiler version: %s\n", version);
    return true;
}

static bool test_compiler_build_info(void) {
    printf("Testing compiler build info...\n");
    const char* build_info = asthra_compiler_get_build_info();
    if (!build_info) {
        printf("❌ Failed to get compiler build info\n");
        return false;
    }
    printf("  ✓ Build info: %s\n", build_info);
    return true;
}

static bool test_compiler_options(void) {
    printf("Testing compiler options...\n");
    
    AsthraCompilerOptions options = asthra_compiler_options_create("test.as", "test.out");
    
    // Test default options
    printf("  ✓ Created compiler options\n");
    printf("    - Optimization: %s\n", 
           asthra_get_optimization_level_string(options.opt_level));
    printf("    - Target: %s\n", 
           asthra_get_target_triple(options.target_arch));
    
    // Validate options
    bool valid = asthra_compiler_validate_options(&options);
    if (!valid) {
        printf("❌ Options validation failed\n");
        return false;
    }
    printf("  ✓ Options validated successfully\n");
    
    return true;
}

static bool test_argument_list(void) {
    printf("Testing argument list...\n");
    
    struct AsthraArgumentList* args = asthra_argument_list_create(10);
    if (!args) {
        printf("❌ Failed to create argument list\n");
        return false;
    }
    
    // Test adding arguments
    asthra_argument_list_add(&args, "arg1");
    asthra_argument_list_add(&args, "arg2");
    asthra_argument_list_add(&args, "arg3");
    
    size_t count = asthra_argument_list_count(args);
    if (count != 3) {
        printf("❌ Expected 3 arguments, got %zu\n", count);
        asthra_argument_list_destroy(args);
        return false;
    }
    printf("  ✓ Added 3 arguments\n");
    
    // Test getting arguments
    for (size_t i = 0; i < count; i++) {
        const char* arg = asthra_argument_list_get(args, i);
        printf("    - args[%zu] = %s\n", i, arg);
    }
    
    asthra_argument_list_destroy(args);
    printf("  ✓ Argument list operations successful\n");
    return true;
}

int main(void) {
    printf("\n=== Basic Compiler API Tests ===\n\n");
    
    int passed = 0;
    int failed = 0;
    
    // Run tests
    if (test_compiler_version()) passed++; else failed++;
    printf("\n");
    
    if (test_compiler_build_info()) passed++; else failed++;
    printf("\n");
    
    if (test_compiler_options()) passed++; else failed++;
    printf("\n");
    
    if (test_argument_list()) passed++; else failed++;
    printf("\n");
    
    // Summary
    printf("=== Test Results ===\n");
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);
    
    if (failed == 0) {
        printf("\n✅ All tests passed!\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed!\n");
        return 1;
    }
}