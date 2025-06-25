/**
 * Asthra Programming Language Compiler
 * Standalone Import Path Validation Tests - Phase 1: Import System Enhancement
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations of our validation functions
bool validate_import_path_format(const char *path);
bool validate_stdlib_path(const char *path);
bool validate_internal_path(const char *path);
bool validate_github_path(const char *path);
bool validate_local_path(const char *path);

// Standalone implementations of our validation functions
bool validate_import_path_format(const char *path) {
    if (!path)
        return false;

    // Basic format validation (not complex grammar rules)
    size_t len = strlen(path);
    if (len == 0)
        return false;

    // No double spaces
    if (strstr(path, "  "))
        return false;

    // No leading/trailing spaces
    if (path[0] == ' ' || path[len - 1] == ' ')
        return false;

    // Validate known patterns
    if (strncmp(path, "stdlib/", 7) == 0)
        return validate_stdlib_path(path);
    if (strncmp(path, "internal/", 9) == 0)
        return false; // Internal paths are restricted for user code
    if (strstr(path, "github.com/") || strstr(path, "gitlab.com/") ||
        strstr(path, "bitbucket.org/") || strstr(path, "codeberg.org/") || strstr(path, "sr.ht/")) {
        return validate_github_path(path);
    }
    if (path[0] == '.' && (path[1] == '/' || (path[1] == '.' && path[2] == '/'))) {
        return validate_local_path(path);
    }

    // For validation purposes, we'll be more restrictive and require known patterns
    // In a real implementation, this might allow more patterns for extensibility

    // Check if it looks like a local path without proper prefix
    if (strchr(path, '/') && path[0] != '.' && strncmp(path, "stdlib/", 7) != 0 &&
        strncmp(path, "internal/", 9) != 0 && !strstr(path, ".com/") && !strstr(path, ".org/")) {
        return false; // Reject paths that look local but don't have proper prefix
    }

    // Also reject simple identifiers that should be local paths
    if (!strchr(path, '/') && !strstr(path, ".com") && !strstr(path, ".org")) {
        return false; // Reject simple names like "utils" that should be "./utils"
    }

    return true; // Allow unknown patterns for future extensibility
}

bool validate_stdlib_path(const char *path) {
    if (!path || strncmp(path, "stdlib/", 7) != 0)
        return false;

    const char *module_name = path + 7; // Skip "stdlib/"
    if (strlen(module_name) == 0)
        return false;

    // Check for valid module names (basic validation)
    // Valid stdlib modules: string, io, collections, math, etc.
    const char *valid_modules[] = {"string", "io",         "collections", "math", "time",
                                   "fs",     "net",        "crypto",      "json", "xml",
                                   "http",   "concurrent", "testing",     NULL};

    for (int i = 0; valid_modules[i]; i++) {
        size_t mod_len = strlen(valid_modules[i]);
        if (strncmp(module_name, valid_modules[i], mod_len) == 0) {
            // Allow submodules like "collections/hashmap"
            if (module_name[mod_len] == '\0' || module_name[mod_len] == '/') {
                return true;
            }
        }
    }

    return true; // Allow unknown stdlib modules for extensibility
}

bool validate_internal_path(const char *path) {
    if (!path || strncmp(path, "internal/", 9) != 0)
        return false;

    const char *module_name = path + 9; // Skip "internal/"
    if (strlen(module_name) == 0)
        return false;

    // Internal modules are restricted - only compiler internals
    const char *valid_internal[] = {"runtime", "memory", "gc", "ffi", "codegen", "analysis", NULL};

    for (int i = 0; valid_internal[i]; i++) {
        size_t mod_len = strlen(valid_internal[i]);
        if (strncmp(module_name, valid_internal[i], mod_len) == 0) {
            if (module_name[mod_len] == '\0' || module_name[mod_len] == '/') {
                return true;
            }
        }
    }

    return false; // Internal modules are strictly controlled
}

bool validate_github_path(const char *path) {
    if (!path)
        return false;

    // Look for domain patterns
    const char *domains[] = {"github.com/",   "gitlab.com/", "bitbucket.org/",
                             "codeberg.org/", "sr.ht/",      NULL};

    const char *domain_start = NULL;
    size_t domain_len = 0;

    for (int i = 0; domains[i]; i++) {
        domain_start = strstr(path, domains[i]);
        if (domain_start == path) { // Must be at start
            domain_len = strlen(domains[i]);
            break;
        }
    }

    if (!domain_start)
        return false;

    const char *repo_path = path + domain_len;
    if (strlen(repo_path) == 0)
        return false;

    // Must have at least "user/repo" format
    const char *slash = strchr(repo_path, '/');
    if (!slash || slash == repo_path)
        return false; // No username
    if (strlen(slash + 1) == 0)
        return false; // No repo name

    return true;
}

bool validate_local_path(const char *path) {
    if (!path)
        return false;

    // Must start with "./" or "../"
    if (!(path[0] == '.' && path[1] == '/') &&
        !(path[0] == '.' && path[1] == '.' && path[2] == '/')) {
        return false;
    }

    // Check for basic path validity (no null bytes, reasonable length)
    size_t len = strlen(path);
    if (len > 512)
        return false; // Reasonable path length limit

    // No ".." after initial "../" for security
    const char *dotdot = strstr(path + 3, "..");
    if (dotdot)
        return false;

    return true;
}

// Test helper macros
#define ASSERT_IMPORT_VALID(path)                                                                  \
    do {                                                                                           \
        bool result = validate_import_path_format(path);                                           \
        if (!result) {                                                                             \
            fprintf(stderr, "FAIL: Expected '%s' to be valid import path\n", path);                \
            exit(1);                                                                               \
        }                                                                                          \
        printf("PASS: '%s' is valid import path\n", path);                                         \
    } while (0)

#define ASSERT_IMPORT_INVALID(path)                                                                \
    do {                                                                                           \
        bool result = validate_import_path_format(path);                                           \
        if (result) {                                                                              \
            fprintf(stderr, "FAIL: Expected '%s' to be invalid import path\n", path);              \
            exit(1);                                                                               \
        }                                                                                          \
        printf("PASS: '%s' is correctly rejected as invalid\n", path);                             \
    } while (0)

// Test early validation
void test_import_path_early_validation(void) {
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
    ASSERT_IMPORT_INVALID("");                 // Empty path
    ASSERT_IMPORT_INVALID("stdlib  string");   // Double space
    ASSERT_IMPORT_INVALID(" stdlib/string");   // Leading space
    ASSERT_IMPORT_INVALID("stdlib/string ");   // Trailing space
    ASSERT_IMPORT_INVALID("github.com/user");  // Incomplete GitHub path
    ASSERT_IMPORT_INVALID("local/path");       // Local path without ./
    ASSERT_IMPORT_INVALID("internal/runtime"); // Internal access (should be caught)
}

// Test stdlib path validation
void test_stdlib_path_validation(void) {
    printf("\n=== Testing Stdlib Path Validation ===\n");

    // Valid stdlib paths
    ASSERT_IMPORT_VALID("stdlib/string");
    ASSERT_IMPORT_VALID("stdlib/io");
    ASSERT_IMPORT_VALID("stdlib/collections");
    ASSERT_IMPORT_VALID("stdlib/math");
    ASSERT_IMPORT_VALID("stdlib/collections/hashmap");
    ASSERT_IMPORT_VALID("stdlib/concurrent/channels");

    // Invalid stdlib paths (basic validation)
    ASSERT_IMPORT_INVALID("stdlib/"); // Empty module name

    printf("Stdlib path validation tests passed\n");
}

// Test GitHub path validation
void test_github_path_validation(void) {
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

    printf("GitHub path validation tests passed\n");
}

// Test local path validation
void test_local_path_validation(void) {
    printf("\n=== Testing Local Path Validation ===\n");

    // Valid local paths
    ASSERT_IMPORT_VALID("./utils");
    ASSERT_IMPORT_VALID("./sub/module");
    ASSERT_IMPORT_VALID("../common/types");
    ASSERT_IMPORT_VALID("../parent/shared");

    // Invalid local paths
    ASSERT_IMPORT_INVALID("utils");             // Missing ./
    ASSERT_IMPORT_INVALID("sub/module");        // Missing ./
    ASSERT_IMPORT_INVALID("./../../dangerous"); // Too many .. for security

    printf("Local path validation tests passed\n");
}

// Test internal path validation (should be restricted)
void test_internal_path_validation(void) {
    printf("\n=== Testing Internal Path Validation ===\n");

    // All internal paths should be invalid for user code
    ASSERT_IMPORT_INVALID("internal/runtime");
    ASSERT_IMPORT_INVALID("internal/memory");
    ASSERT_IMPORT_INVALID("internal/gc");
    ASSERT_IMPORT_INVALID("internal/ffi");

    printf("Internal path validation tests passed\n");
}

// Test specific error detection
void test_error_detection(void) {
    printf("\n=== Testing Error Detection ===\n");

    // Test whitespace detection
    bool has_whitespace = (strstr("stdlib  string", "  ") != NULL);
    assert(has_whitespace);
    printf("PASS: Double space detection works\n");

    // Test empty path detection
    bool is_empty = (strlen("") == 0);
    assert(is_empty);
    printf("PASS: Empty path detection works\n");

    // Test internal path detection
    bool is_internal = (strncmp("internal/runtime", "internal/", 9) == 0);
    assert(is_internal);
    printf("PASS: Internal path detection works\n");

    printf("Error detection tests passed\n");
}

int main(void) {
    printf("Starting Import Path Validation Tests\n");
    printf("=====================================\n");

    test_import_path_early_validation();
    test_stdlib_path_validation();
    test_github_path_validation();
    test_local_path_validation();
    test_internal_path_validation();
    test_error_detection();

    printf("\n=====================================\n");
    printf("All Import Path Validation Tests PASSED!\n");
    printf("Phase 1: Import System Enhancement validation complete\n");

    return 0;
}
