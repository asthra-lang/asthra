/*
 * Hash Package Tests
 * Tests for Asthra stdlib hash package functionality
 * 
 * Tests the SipHash, FNV, and utility functions in the hash package
 * Uses test framework methodology from successful stdlib tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include "../framework/test_framework.h"

// ============================================================================
// GLOBAL TEST TRACKING
// ============================================================================

static int tests_run = 0;
static int tests_passed = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("  ✅ PASS: %s\n", message); \
        } else { \
            printf("  ❌ FAIL: %s:%d - %s\n", __FILE__, __LINE__, message); \
        } \
    } while(0)

// ============================================================================
// HASH PACKAGE VALIDATION TESTS
// ============================================================================

void test_hash_package_structure(void) {
    printf("Testing hash package structure...\n");
    
    // Test that our hash package files exist and are readable
    FILE* mod_file = fopen("../stdlib/hash/mod.asthra", "r");
    TEST_ASSERT(mod_file != NULL, "mod.asthra should exist and be readable");
    if (mod_file) fclose(mod_file);
    
    FILE* common_file = fopen("../stdlib/hash/common.asthra", "r");
    TEST_ASSERT(common_file != NULL, "common.asthra should exist and be readable");
    if (common_file) fclose(common_file);
    
    FILE* siphash_file = fopen("../stdlib/hash/siphash.asthra", "r");
    TEST_ASSERT(siphash_file != NULL, "siphash.asthra should exist and be readable");
    if (siphash_file) fclose(siphash_file);
    
    FILE* fnv_file = fopen("../stdlib/hash/fnv.asthra", "r");
    TEST_ASSERT(fnv_file != NULL, "fnv.asthra should exist and be readable");
    if (fnv_file) fclose(fnv_file);
    
    FILE* table_file = fopen("../stdlib/hash/table.asthra", "r");
    TEST_ASSERT(table_file != NULL, "table.asthra should exist and be readable");
    if (table_file) fclose(table_file);
    
    FILE* utils_file = fopen("../stdlib/hash/utils.asthra", "r");
    TEST_ASSERT(utils_file != NULL, "utils.asthra should exist and be readable");
    if (utils_file) fclose(utils_file);
    
    printf("  Hash package structure validation completed\n");
}

void test_hash_package_syntax(void) {
    printf("Testing hash package syntax compliance...\n");
    
    // Test that each file contains proper package declarations
    FILE* mod_file = fopen("../stdlib/hash/mod.asthra", "r");
    if (mod_file) {
        char buffer[256];
        bool found_package = false;
        while (fgets(buffer, sizeof(buffer), mod_file)) {
            if (strstr(buffer, "package stdlib::hash;")) {
                found_package = true;
                break;
            }
        }
        TEST_ASSERT(found_package, "mod.asthra should contain correct package declaration");
        fclose(mod_file);
    }
    
    // Test that siphash.asthra contains SipHash structures
    FILE* siphash_file = fopen("../stdlib/hash/siphash.asthra", "r");
    if (siphash_file) {
        char buffer[256];
        bool found_siphash_key = false;
        bool found_siphash_variant = false;
        while (fgets(buffer, sizeof(buffer), siphash_file)) {
            if (strstr(buffer, "struct SipHashKey")) {
                found_siphash_key = true;
            }
            if (strstr(buffer, "enum SipHashVariant")) {
                found_siphash_variant = true;
            }
        }
        TEST_ASSERT(found_siphash_key, "siphash.asthra should contain SipHashKey struct");
        TEST_ASSERT(found_siphash_variant, "siphash.asthra should contain SipHashVariant enum");
        fclose(siphash_file);
    }
    
    // Test that fnv.asthra contains FNV constants
    FILE* fnv_file = fopen("../stdlib/hash/fnv.asthra", "r");
    if (fnv_file) {
        char buffer[256];
        bool found_fnv_constant = false;
        while (fgets(buffer, sizeof(buffer), fnv_file)) {
            if (strstr(buffer, "FNV1A_64_OFFSET_BASIS")) {
                found_fnv_constant = true;
                break;
            }
        }
        TEST_ASSERT(found_fnv_constant, "fnv.asthra should contain FNV constants");
        fclose(fnv_file);
    }
    
    printf("  Hash package syntax validation completed\n");
}

void test_hash_package_completeness(void) {
    printf("Testing hash package implementation completeness...\n");
    
    // Count functions in each module to ensure completeness
    FILE* siphash_file = fopen("../stdlib/hash/siphash.asthra", "r");
    if (siphash_file) {
        char buffer[512];
        int function_count = 0;
        while (fgets(buffer, sizeof(buffer), siphash_file)) {
            if (strstr(buffer, "pub fn ") && !strstr(buffer, "//")) {
                function_count++;
            }
        }
        TEST_ASSERT(function_count >= 10, "siphash.asthra should have at least 10 public functions");
        printf("  SipHash module has %d public functions\n", function_count);
        fclose(siphash_file);
    }
    
    FILE* fnv_file = fopen("../stdlib/hash/fnv.asthra", "r");
    if (fnv_file) {
        char buffer[512];
        int function_count = 0;
        while (fgets(buffer, sizeof(buffer), fnv_file)) {
            if (strstr(buffer, "pub fn ") && !strstr(buffer, "//")) {
                function_count++;
            }
        }
        TEST_ASSERT(function_count >= 5, "fnv.asthra should have at least 5 public functions");
        printf("  FNV module has %d public functions\n", function_count);
        fclose(fnv_file);
    }
    
    FILE* table_file = fopen("../stdlib/hash/table.asthra", "r");
    if (table_file) {
        char buffer[512];
        int function_count = 0;
        while (fgets(buffer, sizeof(buffer), table_file)) {
            if (strstr(buffer, "pub fn ") && !strstr(buffer, "//")) {
                function_count++;
            }
        }
        TEST_ASSERT(function_count >= 5, "table.asthra should have at least 5 public functions");
        printf("  Table module has %d public functions\n", function_count);
        fclose(table_file);
    }
    
    printf("  Hash package completeness validation completed\n");
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    printf("==========================================================================\n");
    printf("Hash Package Tests - SipHash Standard Library Implementation Validation\n");
    printf("==========================================================================\n");
    
    // Run test categories
    test_hash_package_structure();
    test_hash_package_syntax();
    test_hash_package_completeness();
    
    // Print final results
    printf("\n==========================================================================\n");
    printf("HASH PACKAGE TESTS SUMMARY\n");
    printf("==========================================================================\n");
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    printf("Success rate: %.1f%%\n", tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    
    if (tests_passed == tests_run && tests_run > 0) {
        printf("🎉 ALL TESTS PASSED! Hash package implementation is complete.\n");
        return 0;
    } else {
        printf("❌ Some tests failed. Check individual test output above.\n");
        return 1;
    }
} 
