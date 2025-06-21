/**
 * Asthra Programming Language Compiler
 * ELF Relocation Test Suite
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Comprehensive test suite for all ELF relocation tests
 */

#include "elf_relocation_test_fixtures.h"

// Forward declarations of all test functions
AsthraTestResult test_relocation_entry_generation(AsthraTestContext* context);
AsthraTestResult test_relocation_processing(AsthraTestContext* context);
AsthraTestResult test_relocation_types(AsthraTestContext* context);
AsthraTestResult test_relocation_section_management(AsthraTestContext* context);
AsthraTestResult test_dynamic_relocation_support(AsthraTestContext* context);
AsthraTestResult test_relocation_validation(AsthraTestContext* context);

// Include test implementations
#include "test_elf_relocation_basic.c"
#include "test_elf_relocation_types.c"
#include "test_elf_relocation_sections.c"
#include "test_elf_relocation_dynamic.c"
#include "test_elf_relocation_validation.c"

/**
 * Main test function for complete ELF relocations suite
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = {
        .name = "ELF Relocation Test Suite",
        .description = "Comprehensive test suite for ELF relocation entry generation, processing, and dynamic linking",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 10000000000ULL, // 10 seconds
        .max_parallel_tests = 1,
        .statistics = stats
    };
    
    AsthraTestFunction tests[] = {
        test_relocation_entry_generation,
        test_relocation_processing,
        test_relocation_types,
        test_relocation_section_management,
        test_dynamic_relocation_support,
        test_relocation_validation
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_relocation_entry_generation", __FILE__, __LINE__, "test_relocation_entry_generation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_relocation_processing", __FILE__, __LINE__, "test_relocation_processing", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_relocation_types", __FILE__, __LINE__, "test_relocation_types", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_relocation_section_management", __FILE__, __LINE__, "test_relocation_section_management", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_dynamic_relocation_support", __FILE__, __LINE__, "test_dynamic_relocation_support", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_relocation_validation", __FILE__, __LINE__, "test_relocation_validation", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
