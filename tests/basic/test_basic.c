/**
 * Simple Basic Compiler Test
 * Tests basic compiler initialization and version info
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "compiler.h"

/**
 * Test compiler version string
 */
static bool test_compiler_version(void) {
    printf("Testing compiler version...\n");
    
    const char* version = asthra_compiler_get_version();
    if (!version) {
        printf("❌ Failed to get compiler version\n");
        return false;
    }
    
    printf("  Compiler version: %s\n", version);
    
    // Check version format (should be like "1.0.0")
    if (strlen(version) < 5) {
        printf("❌ Invalid version format\n");
        return false;
    }
    
    printf("  ✓ Compiler version test passed\n");
    return true;
}

/**
 * Test compiler build info
 */
static bool test_compiler_build_info(void) {
    printf("Testing compiler build info...\n");
    
    const char* build_info = asthra_compiler_get_build_info();
    if (!build_info) {
        printf("❌ Failed to get compiler build info\n");
        return false;
    }
    
    printf("  Build info: %s\n", build_info);
    printf("  ✓ Compiler build info test passed\n");
    return true;
}

/**
 * Test compiler context creation and destruction
 */
static bool test_compiler_context_lifecycle(void) {
    printf("Testing compiler context lifecycle...\n");
    
    AsthraCompilerOptions options = {
        .input_file = "test.asthra",
        .output_file = "test.out",
        .opt_level = ASTHRA_OPT_NONE,
        .target_arch = ASTHRA_TARGET_X86_64,
        .debug_info = false,
        .verbose = false,
        .emit_llvm = false,
        .emit_asm = false,
        .no_stdlib = false,
        .include_paths = NULL,
        .library_paths = NULL,
        .libraries = NULL
    };
    
    AsthraCompilerContext* ctx = asthra_compiler_create(&options);
    if (!ctx) {
        printf("❌ Failed to create compiler context\n");
        return false;
    }
    
    printf("  ✓ Compiler context created successfully\n");
    
    asthra_compiler_destroy(ctx);
    printf("  ✓ Compiler context destroyed successfully\n");
    
    return true;
}

/**
 * Run all basic compiler tests
 */
static bool run_basic_compiler_tests(void) {
    printf("\n=== Basic Compiler Tests ===\n");
    
    bool all_passed = true;
    
    all_passed &= test_compiler_version();
    all_passed &= test_compiler_build_info();
    all_passed &= test_compiler_context_lifecycle();
    
    printf("\n=== Test Results ===\n");
    if (all_passed) {
        printf("✅ All basic compiler tests passed!\n");
    } else {
        printf("❌ Some basic compiler tests failed!\n");
    }
    
    return all_passed;
}

/**
 * Main entry point
 */
int main(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused
    
    bool success = run_basic_compiler_tests();
    return success ? 0 : 1;
}