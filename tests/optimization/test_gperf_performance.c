/**
 * Asthra Programming Language
 * Gperf Hash Function Performance Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_gperf_common.h"

static AsthraTestResult test_hash_function_performance(AsthraTestContext *context) {
    // Test the performance of the generated hash function
    
    const char *benchmark_program = "optimization/benchmark_keywords";
    
    // Check if benchmark program exists
    if (!gperf_file_exists(benchmark_program)) {
        printf("Hash function benchmark program not found, skipping performance test\n");
        return ASTHRA_TEST_SKIP;
    }
    
    // Run benchmark
    clock_t start = clock();
    int exit_code = run_command_with_timeout(benchmark_program, 60.0);
    clock_t end = clock();
    
    double execution_time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
    
    if (!ASTHRA_TEST_ASSERT_EQ(context, exit_code, 0, 
                               "Hash function benchmark should complete successfully")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Performance should be reasonable (less than 100ms for benchmark)
    if (!ASTHRA_TEST_ASSERT(context, execution_time_ms < 100.0, 
                           "Hash function benchmark should complete quickly (%.2f ms)", 
                           execution_time_ms)) {
        return ASTHRA_TEST_FAIL;
    }
    
    printf("Hash function performance: %.2f ms\n", execution_time_ms);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_hash_function_memory_efficiency(AsthraTestContext *context) {
    // Test that the generated hash function is memory efficient
    
    const char *hash_file = "optimization/asthra_keywords_hash.c";
    struct stat st;
    
    if (stat(hash_file, &st) != 0) {
        return ASTHRA_TEST_SKIP;
    }
    
    // Generated file should be reasonable size (not too large)
    if (!ASTHRA_TEST_ASSERT(context, st.st_size < 50000, 
                           "Generated hash file should be reasonably sized (%lld bytes)", 
                           (long long)st.st_size)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // But should be substantial enough to contain the hash function
    if (!ASTHRA_TEST_ASSERT(context, st.st_size > 1000, 
                           "Generated hash file should be substantial (%lld bytes)", 
                           (long long)st.st_size)) {
        return ASTHRA_TEST_FAIL;
    }
    
    printf("Generated hash function size: %lld bytes\n", (long long)st.st_size);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_hash_function_lookup_speed(AsthraTestContext *context) {
    // Test individual keyword lookup speed
    const char *speed_test_program = "optimization/speed_test_keywords";
    
    if (!gperf_file_exists(speed_test_program)) {
        printf("Hash function speed test program not found, skipping speed test\n");
        return ASTHRA_TEST_SKIP;
    }
    
    // Time multiple runs to get average
    double total_time = 0.0;
    int successful_runs = 0;
    const int test_runs = 5;
    
    for (int i = 0; i < test_runs; i++) {
        clock_t start = clock();
        int exit_code = run_command_with_timeout(speed_test_program, 30.0);
        clock_t end = clock();
        
        if (exit_code == 0) {
            double run_time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
            total_time += run_time;
            successful_runs++;
        }
    }
    
    if (successful_runs == 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    double avg_time = total_time / successful_runs;
    
    // Average lookup time should be very fast (< 10ms per run)
    if (!ASTHRA_TEST_ASSERT(context, avg_time < 10.0, 
                           "Hash function lookup should be fast (%.2f ms avg)", avg_time)) {
        return ASTHRA_TEST_FAIL;
    }
    
    printf("Hash function lookup speed: %.2f ms average\n", avg_time);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_hash_function_scalability(AsthraTestContext *context) {
    // Test that the hash function scales well with keyword count
    
    const char *hash_file = "optimization/asthra_keywords_hash.c";
    FILE *file = fopen(hash_file, "r");
    
    if (!ASTHRA_TEST_ASSERT(context, file != NULL, 
                           "Should be able to open generated hash file")) {
        return ASTHRA_TEST_FAIL;
    }
    
    char line[512];
    int keyword_count = 0;
    bool in_table = false;
    
    // Count keywords in the generated table
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "wordlist") || strstr(line, "keywords")) {
            in_table = true;
            continue;
        }
        
        if (in_table && strstr(line, "}")) {
            break;
        }
        
        if (in_table && strstr(line, ",")) {
            keyword_count++;
        }
    }
    
    fclose(file);
    
    // Should handle a reasonable number of keywords efficiently
    if (!ASTHRA_TEST_ASSERT(context, keyword_count >= 20, 
                           "Hash function should handle at least 20 keywords (%d found)", 
                           keyword_count)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, keyword_count <= 200, 
                           "Hash function should not be bloated (%d keywords)", 
                           keyword_count)) {
        return ASTHRA_TEST_FAIL;
    }
    
    printf("Hash function scalability: %d keywords handled\n", keyword_count);
    return ASTHRA_TEST_PASS;
}

// Public test suite interface
AsthraTestFunction gperf_performance_tests[] = {
    test_hash_function_performance,
    test_hash_function_memory_efficiency,
    test_hash_function_lookup_speed,
    test_hash_function_scalability
};

AsthraTestMetadata gperf_performance_metadata[] = {
    {"Hash Function Performance", __FILE__, __LINE__, "test_hash_function_performance", ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
    {"Hash Function Memory Efficiency", __FILE__, __LINE__, "test_hash_function_memory_efficiency", ASTHRA_TEST_SEVERITY_LOW, 0, false, NULL},
    {"Hash Function Lookup Speed", __FILE__, __LINE__, "test_hash_function_lookup_speed", ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
    {"Hash Function Scalability", __FILE__, __LINE__, "test_hash_function_scalability", ASTHRA_TEST_SEVERITY_LOW, 0, false, NULL}
};

const size_t gperf_performance_test_count = sizeof(gperf_performance_tests) / sizeof(gperf_performance_tests[0]); 
