/**
 * Asthra Programming Language Compiler
 * ELF Relocation Section Management Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test ELF relocation section management and section-specific relocations
 */

#include "elf_relocation_test_fixtures.h"

/**
 * Test relocation section management
 */
AsthraTestResult test_relocation_section_management(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_reloc_sections.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create relocations for different sections
    ELFRelocation text_relocations[] = {
        {.offset = 0x10, .symbol_index = 1, .type = R_X86_64_PC32, .addend = -4, .section = ".text"},
        {.offset = 0x20, .symbol_index = 2, .type = R_X86_64_PLT32, .addend = -4, .section = ".text"},
    };
    
    ELFRelocation data_relocations[] = {
        {.offset = 0x00, .symbol_index = 3, .type = R_X86_64_64, .addend = 0, .section = ".data"},
        {.offset = 0x08, .symbol_index = 4, .type = R_X86_64_32, .addend = 0, .section = ".data"},
    };
    
    // Add text relocations
    for (size_t i = 0; i < sizeof(text_relocations) / sizeof(text_relocations[0]); i++) {
        bool result = elf_relocation_manager_add_relocation(fixture->relocation_manager, &text_relocations[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add text relocation %zu", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Add data relocations
    for (size_t i = 0; i < sizeof(data_relocations) / sizeof(data_relocations[0]); i++) {
        bool result = elf_relocation_manager_add_relocation(fixture->relocation_manager, &data_relocations[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add data relocation %zu", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Generate section-specific relocation tables
    Elf64_Rela* text_reloc_table;
    size_t text_reloc_count;
    bool result = elf_relocation_manager_generate_section_table(fixture->relocation_manager, ".text", 
                                                               &text_reloc_table, &text_reloc_count);
    if (!asthra_test_assert_bool(context, result, "Failed to generate .text relocation table")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    Elf64_Rela* data_reloc_table;
    size_t data_reloc_count;
    result = elf_relocation_manager_generate_section_table(fixture->relocation_manager, ".data",
                                                          &data_reloc_table, &data_reloc_count);
    if (!asthra_test_assert_bool(context, result, "Failed to generate .data relocation table")) {
        free(text_reloc_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify section-specific counts
    if (!asthra_test_assert_size_eq(context, text_reloc_count, 2, ".text should have 2 relocations")) {
        free(text_reloc_table);
        free(data_reloc_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_size_eq(context, data_reloc_count, 2, ".data should have 2 relocations")) {
        free(text_reloc_table);
        free(data_reloc_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    free(text_reloc_table);
    free(data_reloc_table);
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Main test function for ELF relocation section management
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = {
        .name = "ELF Relocation Section Management Tests",
        .description = "Test ELF relocation section management and section-specific relocations",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 10000000000ULL, // 10 seconds
        .max_parallel_tests = 1,
        .statistics = stats
    };
    
    AsthraTestFunction tests[] = {
        test_relocation_section_management
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_relocation_section_management", __FILE__, __LINE__, "test_relocation_section_management", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
