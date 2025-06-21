/**
 * Asthra Programming Language Compiler
 * ELF Relocation Validation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test ELF relocation validation and error handling
 */

#include "elf_relocation_test_fixtures.h"

/**
 * Test relocation validation and error handling
 */
AsthraTestResult test_relocation_validation(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_reloc_validation.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test invalid relocation type
    ELFRelocation invalid_reloc = {
        .offset = 0x1000,
        .symbol_index = 1,
        .type = 999, // Invalid type
        .addend = 0
    };
    
    bool result = elf_relocation_manager_add_relocation(fixture->relocation_manager, &invalid_reloc);
    if (!asthra_test_assert_bool(context, !result, "Should reject invalid relocation type")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test relocation with invalid symbol index
    ELFRelocation invalid_symbol_reloc = {
        .offset = 0x1000,
        .symbol_index = UINT32_MAX, // Invalid symbol index
        .type = R_X86_64_PC32,
        .addend = 0
    };
    
    result = elf_relocation_manager_add_relocation(fixture->relocation_manager, &invalid_symbol_reloc);
    if (!asthra_test_assert_bool(context, !result, "Should reject relocation with invalid symbol index")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test valid relocation for comparison
    ELFRelocation valid_reloc = {
        .offset = 0x1000,
        .symbol_index = 1,
        .type = R_X86_64_PC32,
        .addend = -4
    };
    
    result = elf_relocation_manager_add_relocation(fixture->relocation_manager, &valid_reloc);
    if (!asthra_test_assert_bool(context, result, "Should accept valid relocation")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Validate all relocations
    result = elf_relocation_manager_validate_all(fixture->relocation_manager);
    if (!asthra_test_assert_bool(context, result, "Relocation validation should pass")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Main test function for ELF relocation validation
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = {
        .name = "ELF Relocation Validation Tests",
        .description = "Test ELF relocation validation and error handling",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 10000000000ULL, // 10 seconds
        .max_parallel_tests = 1,
        .statistics = stats
    };
    
    AsthraTestFunction tests[] = {
        test_relocation_validation
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_relocation_validation", __FILE__, __LINE__, "test_relocation_validation", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
