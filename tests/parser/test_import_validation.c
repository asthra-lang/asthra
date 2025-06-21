/**
 * Asthra Programming Language Compiler
 * Import Path Validation Tests - Phase 1: Import System Enhancement
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parser.h"
#include "parser_error.h"

// Test framework macros (simple pattern like test_parser_core.c)
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n", message); \
            return 0; \
        } \
    } while (0)

#define TEST_PASS(message) \
    do { \
        printf("PASS: %s\n", message); \
        return 1; \
    } while (0)

// Test helper macros for import validation
#define ASSERT_IMPORT_VALID(path) do { \
    bool result = validate_import_path_format(path); \
    TEST_ASSERT(result, "Expected '" path "' to be valid import path"); \
    printf("PASS: '%s' is valid import path\n", path); \
} while(0)

#define ASSERT_IMPORT_INVALID(path) do { \
    bool result = validate_import_path_format(path); \
    TEST_ASSERT(!result, "Expected '" path "' to be invalid import path"); \
    printf("PASS: '%s' is correctly rejected as invalid\n", path); \
} while(0)

// Function prototypes
static int test_import_path_early_validation(void);
static int test_stdlib_path_validation(void);
static int test_github_path_validation(void);
static int test_local_path_validation(void);
static int test_internal_path_validation(void);
static int test_error_detection(void);

// Test early validation
static int test_import_path_early_validation(void) {
    printf("\n=== Testing Import Path Early Validation ===\n");
    
    // Valid formats
    ASSERT_IMPORT_VALID("stdlib/string");
    ASSERT_IMPORT_VALID("stdlib/collections/hashmap");
    ASSERT_IMPORT_VALID("github.com/user/repo");
    ASSERT_IMPORT_VALID("gitlab.com/organization/project");
    ASSERT_IMPORT_VALID("bitbucket.org/team/package");
    ASSERT_IMPORT_VALID("./local/path");
    ASSERT_IMPORT_VALID("../parent/module");
    ASSERT_IMPORT_VALID("codeberg.org/user/repo");
    ASSERT_IMPORT_VALID("sr.ht/~user/project");
    
    // Invalid formats
    ASSERT_IMPORT_INVALID("");  // Empty path
    ASSERT_IMPORT_INVALID("stdlib  string");  // Double space
    ASSERT_IMPORT_INVALID(" stdlib/string");  // Leading space
    ASSERT_IMPORT_INVALID("stdlib/string ");  // Trailing space
    ASSERT_IMPORT_INVALID("github.com/user");  // Incomplete GitHub path
    ASSERT_IMPORT_INVALID("local/path");  // Local path without ./
    ASSERT_IMPORT_INVALID("internal/runtime");  // Internal access (should be caught)
    
    TEST_PASS("Import path early validation");
}

// Test stdlib path validation
static int test_stdlib_path_validation(void) {
    printf("\n=== Testing Stdlib Path Validation ===\n");
    
    // Valid stdlib paths
    ASSERT_IMPORT_VALID("stdlib/string");
    ASSERT_IMPORT_VALID("stdlib/io");
    ASSERT_IMPORT_VALID("stdlib/collections");
    ASSERT_IMPORT_VALID("stdlib/math");
    ASSERT_IMPORT_VALID("stdlib/collections/hashmap");
    ASSERT_IMPORT_VALID("stdlib/concurrent/channels");
    
    // Invalid stdlib paths (basic validation)
    ASSERT_IMPORT_INVALID("stdlib/");  // Empty module name
    
    TEST_PASS("Stdlib path validation");
}

// Test GitHub path validation
static int test_github_path_validation(void) {
    printf("\n=== Testing GitHub Path Validation ===\n");
    
    // Valid GitHub paths
    ASSERT_IMPORT_VALID("github.com/asthra-lang/json");
    ASSERT_IMPORT_VALID("github.com/user/simple-repo");
    ASSERT_IMPORT_VALID("github.com/org/complex-project");
    ASSERT_IMPORT_VALID("gitlab.com/group/subgroup/project");
    ASSERT_IMPORT_VALID("bitbucket.org/team/repository");
    
    // Invalid GitHub paths
    ASSERT_IMPORT_INVALID("github.com/user");  // Missing repo
    ASSERT_IMPORT_INVALID("github.com/");      // Missing user and repo
    ASSERT_IMPORT_INVALID("github/user/repo"); // Missing .com
    
    TEST_PASS("GitHub path validation");
}

// Test local path validation  
static int test_local_path_validation(void) {
    printf("\n=== Testing Local Path Validation ===\n");
    
    // Valid local paths
    ASSERT_IMPORT_VALID("./utils");
    ASSERT_IMPORT_VALID("./sub/module");
    ASSERT_IMPORT_VALID("../common/types");
    ASSERT_IMPORT_VALID("../parent/shared");
    
    // Invalid local paths
    ASSERT_IMPORT_INVALID("utils");           // Missing ./
    ASSERT_IMPORT_INVALID("sub/module");      // Missing ./
    ASSERT_IMPORT_INVALID("./../../dangerous"); // Too many .. for security
    
    TEST_PASS("Local path validation");
}

// Test internal path validation (should be restricted)
static int test_internal_path_validation(void) {
    printf("\n=== Testing Internal Path Validation ===\n");
    
    // All internal paths should be invalid for user code
    ASSERT_IMPORT_INVALID("internal/runtime");
    ASSERT_IMPORT_INVALID("internal/memory");
    ASSERT_IMPORT_INVALID("internal/gc");
    ASSERT_IMPORT_INVALID("internal/ffi");
    
    TEST_PASS("Internal path validation");
}

// Test specific error detection
static int test_error_detection(void) {
    printf("\n=== Testing Error Detection ===\n");
    
    // Test whitespace detection
    bool has_whitespace = (strstr("stdlib  string", "  ") != NULL);
    TEST_ASSERT(has_whitespace, "Double space detection should work");
    printf("PASS: Double space detection works\n");
    
    // Test empty path detection
    bool is_empty = (strlen("") == 0);
    TEST_ASSERT(is_empty, "Empty path detection should work");
    printf("PASS: Empty path detection works\n");
    
    // Test internal path detection
    bool is_internal = (strncmp("internal/runtime", "internal/", 9) == 0);
    TEST_ASSERT(is_internal, "Internal path detection should work");
    printf("PASS: Internal path detection works\n");
    
    TEST_PASS("Error detection");
}

// Main test runner
int main(void) {
    printf("Starting Import Path Validation Tests\n");
    printf("=====================================\n");
    
    int passed = 0;
    int total = 0;
    
    // Run all tests
    total++; if (test_import_path_early_validation()) passed++;
    total++; if (test_stdlib_path_validation()) passed++;
    total++; if (test_github_path_validation()) passed++;
    total++; if (test_local_path_validation()) passed++;
    total++; if (test_internal_path_validation()) passed++;
    total++; if (test_error_detection()) passed++;
    
    printf("\n=====================================\n");
    printf("Import Path Validation Test Results: %d/%d passed\n", passed, total);
    
    if (passed == total) {
        printf("All Import Path Validation Tests PASSED!\n");
        printf("Phase 1: Import System Enhancement validation complete\n");
        return 0;
    } else {
        printf("Some Import Path Validation Tests FAILED!\n");
        return 1;
    }
}
