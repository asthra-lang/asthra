/**
 * Asthra Programming Language Compiler
 * ELF Dynamic Relocation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test ELF dynamic relocation support and dynamic linking
 */

#include "elf_relocation_test_fixtures.h"

/**
 * Test dynamic relocation support
 */
AsthraTestResult test_dynamic_relocation_support(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_dynamic_reloc.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create dynamic relocations
    ELFRelocation dynamic_relocations[] = {
        {.offset = 0x1000, .symbol_index = 1, .type = R_X86_64_JUMP_SLOT, .addend = 0},
        {.offset = 0x1008, .symbol_index = 2, .type = R_X86_64_GLOB_DAT, .addend = 0},
        {.offset = 0x1010, .symbol_index = 0, .type = R_X86_64_RELATIVE, .addend = 0x400000},
    };
    
    for (size_t i = 0; i < 3; i++) {
        bool result = elf_relocation_manager_add_relocation(fixture->relocation_manager, &dynamic_relocations[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add dynamic relocation %zu", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Generate dynamic relocation table
    Elf64_Rela* dyn_reloc_table;
    size_t dyn_reloc_count;
    bool result = elf_relocation_manager_generate_dynamic_table(fixture->relocation_manager,
                                                               &dyn_reloc_table, &dyn_reloc_count);
    if (!asthra_test_assert_bool(context, result, "Failed to generate dynamic relocation table")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify dynamic relocation count and types
    if (!asthra_test_assert_size_eq(context, dyn_reloc_count, 3, "Expected 3 dynamic relocations")) {
        free(dyn_reloc_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check JUMP_SLOT relocation
    if (!asthra_test_assert_bool(context, ELF64_R_TYPE(dyn_reloc_table[0].r_info) == R_X86_64_JUMP_SLOT,
                                "First dynamic relocation should be JUMP_SLOT")) {
        free(dyn_reloc_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check GLOB_DAT relocation
    if (!asthra_test_assert_bool(context, ELF64_R_TYPE(dyn_reloc_table[1].r_info) == R_X86_64_GLOB_DAT,
                                "Second dynamic relocation should be GLOB_DAT")) {
        free(dyn_reloc_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check RELATIVE relocation
    if (!asthra_test_assert_bool(context, ELF64_R_TYPE(dyn_reloc_table[2].r_info) == R_X86_64_RELATIVE,
                                "Third dynamic relocation should be RELATIVE")) {
        free(dyn_reloc_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    free(dyn_reloc_table);
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Main test function for ELF dynamic relocations
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = {
        .name = "ELF Dynamic Relocation Tests",
        .description = "Test ELF dynamic relocation support and dynamic linking",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 10000000000ULL, // 10 seconds
        .max_parallel_tests = 1,
        .statistics = stats
    };
    
    AsthraTestFunction tests[] = {
        test_dynamic_relocation_support
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_dynamic_relocation_support", __FILE__, __LINE__, "test_dynamic_relocation_support", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
