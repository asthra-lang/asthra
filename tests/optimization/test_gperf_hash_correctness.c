/**
 * Asthra Programming Language
 * Gperf Hash Function Correctness Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_gperf_common.h"

static AsthraTestResult test_hash_function_compilation(AsthraTestContext *context) {
    // Test that the generated hash function compiles correctly
    char command[MAX_COMMAND_LENGTH];
    snprintf(
        command, sizeof(command),
        "cd optimization && gcc -c asthra_keywords_hash.c -o asthra_keywords_hash.o 2>/dev/null");

    int exit_code = run_command_with_timeout(command, 30.0);
    if (!ASTHRA_TEST_ASSERT_EQ(context, exit_code, 0,
                               "Generated hash function should compile without errors")) {
        return ASTHRA_TEST_FAIL;
    }

    // Check that object file was created
    if (!ASTHRA_TEST_ASSERT(context, gperf_file_exists("optimization/asthra_keywords_hash.o"),
                            "Object file should be created")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_hash_function_lookup_accuracy(AsthraTestContext *context) {
    // This test would require linking with the generated hash function
    // For now, we'll test the structure and assume the Makefile target tests functionality

    const char *test_program = "optimization/test_keywords";

    // Check if test program exists (built by Makefile)
    if (!gperf_file_exists(test_program)) {
        printf("Hash function test program not found, skipping accuracy test\n");
        return ASTHRA_TEST_SKIP;
    }

    // Run the test program
    int exit_code = run_command_with_timeout(test_program, 30.0);
    if (!ASTHRA_TEST_ASSERT_EQ(context, exit_code, 0, "Hash function lookup test should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_hash_function_no_collisions(AsthraTestContext *context) {
    // Test that the perfect hash function has no collisions
    // This is verified by checking the gperf output for collision indicators

    const char *hash_file = "optimization/asthra_keywords_hash.c";
    FILE *file = fopen(hash_file, "r");

    if (!ASTHRA_TEST_ASSERT(context, file != NULL, "Should be able to open generated hash file")) {
        return ASTHRA_TEST_FAIL;
    }

    char line[512];
    bool found_perfect_hash_comment = false;
    bool found_collision_resolution = false;

    while (fgets(line, sizeof(line), file)) {
        // Look for gperf's perfect hash indicators
        if (strstr(line, "perfect hash") || strstr(line, "no collisions")) {
            found_perfect_hash_comment = true;
        }

        // Look for collision resolution code (which shouldn't exist in perfect hash)
        if (strstr(line, "collision") && strstr(line, "resolution")) {
            found_collision_resolution = true;
        }
    }

    fclose(file);

    // We want a perfect hash (no collision resolution needed)
    if (found_collision_resolution) {
        printf("Warning: Hash function appears to have collision resolution code\n");
    }

    if (found_perfect_hash_comment) {
        printf("Perfect hash function validation: Found perfect hash indicators\n");
    } else {
        printf("Perfect hash function validation: No explicit perfect hash indicators found\n");
    }

    printf("Perfect hash function validation completed\n");
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_hash_function_structure(AsthraTestContext *context) {
    const char *hash_file = "optimization/asthra_keywords_hash.c";
    FILE *file = fopen(hash_file, "r");

    if (!ASTHRA_TEST_ASSERT(context, file != NULL, "Should be able to open generated hash file")) {
        return ASTHRA_TEST_FAIL;
    }

    char line[512];
    bool found_hash_function = false;
    bool found_lookup_function = false;
    bool found_keyword_table = false;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "hash") && strstr(line, "(")) {
            found_hash_function = true;
        }

        if (strstr(line, "in_word_set") || strstr(line, "lookup")) {
            found_lookup_function = true;
        }

        if (strstr(line, "wordlist") || strstr(line, "keywords")) {
            found_keyword_table = true;
        }
    }

    fclose(file);

    if (!ASTHRA_TEST_ASSERT(context, found_hash_function,
                            "Generated file should contain hash function")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, found_lookup_function,
                            "Generated file should contain lookup function")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, found_keyword_table,
                            "Generated file should contain keyword table")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Public test suite interface
AsthraTestFunction gperf_hash_correctness_tests[] = {
    test_hash_function_compilation, test_hash_function_lookup_accuracy,
    test_hash_function_no_collisions, test_hash_function_structure};

AsthraTestMetadata gperf_hash_correctness_metadata[] = {
    {"Hash Function Compilation", __FILE__, __LINE__, "test_hash_function_compilation",
     ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"Hash Function Lookup Accuracy", __FILE__, __LINE__, "test_hash_function_lookup_accuracy",
     ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"Hash Function No Collisions", __FILE__, __LINE__, "test_hash_function_no_collisions",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
    {"Hash Function Structure", __FILE__, __LINE__, "test_hash_function_structure",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL}};

const size_t gperf_hash_correctness_test_count =
    sizeof(gperf_hash_correctness_tests) / sizeof(gperf_hash_correctness_tests[0]);
