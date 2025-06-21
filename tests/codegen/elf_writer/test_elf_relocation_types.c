/**
 * Asthra Programming Language Compiler
 * ELF Relocation Types Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test different x86_64 ELF relocation types
 */

#include "elf_relocation_test_fixtures.h"

/**
 * Test different relocation types
 */
AsthraTestResult test_relocation_types(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_reloc_types.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test various x86_64 relocation types
    struct {
        uint64_t offset;
        uint32_t symbol_index;
        uint32_t type;
        int64_t addend;
        const char* description;
    } test_relocations[] = {
        {0x1000, 1, R_X86_64_64, 0, "64-bit absolute"},
        {0x1008, 2, R_X86_64_PC32, -4, "32-bit PC-relative"},
        {0x1010, 3, R_X86_64_PLT32, -4, "32-bit PLT-relative"},
        {0x1018, 4, R_X86_64_GOT32, 0, "32-bit GOT-relative"},
        {0x1020, 5, R_X86_64_GOTPCREL, -4, "32-bit GOT PC-relative"},
        {0x1028, 6, R_X86_64_32, 0, "32-bit zero-extended"},
        {0x1030, 7, R_X86_64_32S, 0, "32-bit sign-extended"},
        {0x1038, 8, R_X86_64_16, 0, "16-bit zero-extended"},
        {0x1040, 9, R_X86_64_PC16, -2, "16-bit PC-relative"},
        {0x1048, 10, R_X86_64_8, 0, "8-bit zero-extended"}
    };
    
    size_t test_count = sizeof(test_relocations) / sizeof(test_relocations[0]);
    
    // Add all relocation types
    for (size_t i = 0; i < test_count; i++) {
        ELFRelocation reloc = {
            .offset = test_relocations[i].offset,
            .symbol_index = test_relocations[i].symbol_index,
            .type = test_relocations[i].type,
            .addend = test_relocations[i].addend
        };
        
        bool result = elf_relocation_manager_add_relocation(fixture->relocation_manager, &reloc);
        if (!asthra_test_assert_bool(context, result, "Failed to add %s relocation", test_relocations[i].description)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Generate and validate relocation table
    Elf64_Rela* relocation_table;
    size_t relocation_count;
    bool result = elf_relocation_manager_generate_table(fixture->relocation_manager, &relocation_table, &relocation_count);
    if (!asthra_test_assert_bool(context, result, "Failed to generate relocation table")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify relocation count
    if (!asthra_test_assert_size_eq(context, relocation_count, test_count,
                                   "Relocation count mismatch")) {
        free(relocation_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify each relocation
    for (size_t i = 0; i < relocation_count; i++) {
        if (!asthra_test_assert_bool(context, relocation_table[i].r_offset == test_relocations[i].offset,
                                    "Relocation %zu offset mismatch: expected 0x%lx, got 0x%lx",
                                    i, test_relocations[i].offset, relocation_table[i].r_offset)) {
            free(relocation_table);
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
        
        if (!asthra_test_assert_bool(context, ELF64_R_TYPE(relocation_table[i].r_info) == test_relocations[i].type,
                                    "Relocation %zu type mismatch: expected %u, got %lu",
                                    i, test_relocations[i].type, ELF64_R_TYPE(relocation_table[i].r_info))) {
            free(relocation_table);
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
        
        if (!asthra_test_assert_bool(context, relocation_table[i].r_addend == test_relocations[i].addend,
                                    "Relocation %zu addend mismatch: expected %ld, got %ld",
                                    i, test_relocations[i].addend, relocation_table[i].r_addend)) {
            free(relocation_table);
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    free(relocation_table);
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Main test function for ELF relocation types
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = {
        .name = "ELF Relocation Types Tests",
        .description = "Test different x86_64 ELF relocation types",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 10000000000ULL, // 10 seconds
        .max_parallel_tests = 1,
        .statistics = stats
    };
    
    AsthraTestFunction tests[] = {
        test_relocation_types
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_relocation_types", __FILE__, __LINE__, "test_relocation_types", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
