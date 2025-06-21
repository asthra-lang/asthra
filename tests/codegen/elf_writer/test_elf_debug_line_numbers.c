/**
 * Asthra Programming Language Compiler
 * ELF Debug Line Numbers Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test ELF line number debug information generation
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "elf_writer.h"
#include "elf_writer_core.h"
#include "elf_symbol_manager.h"
#include "elf_relocation_manager.h"
#include "ffi_assembly_generator.h"
#include "ast.h"
#include "elf_writer_test_common.h"
#include "codegen_test_stubs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "elf_compat.h"

// =============================================================================
// LINE NUMBER DEBUG TESTS
// =============================================================================

/**
 * Test line number information
 */
AsthraTestResult test_line_number_info(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_line_numbers.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create line number entries
    ELFLineNumberEntry line_entries[4];
    line_entries[0] = (ELFLineNumberEntry){.address = 0x1000, .line = 1, .column = 1, .file_index = 0};
    line_entries[1] = (ELFLineNumberEntry){.address = 0x1008, .line = 2, .column = 5, .file_index = 0};
    line_entries[2] = (ELFLineNumberEntry){.address = 0x1010, .line = 3, .column = 1, .file_index = 0};
    line_entries[3] = (ELFLineNumberEntry){.address = 0x1018, .line = 4, .column = 1, .file_index = 0};
    
    for (int i = 0; i < 4; i++) {
        bool result = elf_writer_add_line_number_entry(fixture->elf_writer, &line_entries[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add line number entry %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Generate line number table
    bool result = elf_writer_generate_line_number_table(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Failed to generate line number table")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check line number table size
    size_t line_table_size = elf_writer_get_line_table_size(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, line_table_size > 0,
                                "Line number table should have non-zero size")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test line number sorting and ordering
 */
AsthraTestResult test_line_number_ordering(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_line_ordering.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Add line entries in non-sequential order
    ELFLineNumberEntry line_entries[] = {
        {.address = 0x1010, .line = 3, .column = 1, .file_index = 0},
        {.address = 0x1000, .line = 1, .column = 1, .file_index = 0},
        {.address = 0x1020, .line = 5, .column = 1, .file_index = 0},
        {.address = 0x1008, .line = 2, .column = 1, .file_index = 0}
    };
    
    for (int i = 0; i < 4; i++) {
        bool result = elf_writer_add_line_number_entry(fixture->elf_writer, &line_entries[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add line number entry %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Sort line entries by address
    bool result = elf_writer_sort_line_entries(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Failed to sort line entries")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify ordering
    result = elf_writer_validate_line_entry_ordering(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Line entries should be properly ordered")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test line number lookup functionality
 */
AsthraTestResult test_line_number_lookup(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_line_lookup.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Add line entries
    ELFLineNumberEntry line_entries[] = {
        {.address = 0x1000, .line = 10, .column = 1, .file_index = 0},
        {.address = 0x1010, .line = 20, .column = 1, .file_index = 0},
        {.address = 0x1020, .line = 30, .column = 1, .file_index = 0},
        {.address = 0x1030, .line = 40, .column = 1, .file_index = 0}
    };
    
    for (int i = 0; i < 4; i++) {
        bool result = elf_writer_add_line_number_entry(fixture->elf_writer, &line_entries[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add line number entry %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Test exact address lookup
    uint32_t line = 0;
    bool result = elf_writer_lookup_line_number(fixture->elf_writer, 0x1010, &line);
    if (!asthra_test_assert_bool(context, result, "Failed to lookup line number")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, line == 20, "Expected line 20, got %u", line)) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test approximate address lookup (should find closest lower address)
    result = elf_writer_lookup_line_number(fixture->elf_writer, 0x1015, &line);
    if (!asthra_test_assert_bool(context, result, "Failed to lookup approximate line number")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, line == 20, "Expected line 20 for approximate lookup, got %u", line)) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test multiple file support in line numbers
 */
AsthraTestResult test_multi_file_line_numbers(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_multi_file_lines.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Add file entries
    bool result = elf_writer_add_source_file(fixture->elf_writer, "main.asthra");
    if (!asthra_test_assert_bool(context, result, "Failed to add main.asthra")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    result = elf_writer_add_source_file(fixture->elf_writer, "helper.asthra");
    if (!asthra_test_assert_bool(context, result, "Failed to add helper.asthra")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Add line entries from different files
    ELFLineNumberEntry line_entries[] = {
        {.address = 0x1000, .line = 1, .column = 1, .file_index = 0}, // main.asthra
        {.address = 0x1010, .line = 2, .column = 1, .file_index = 0}, // main.asthra
        {.address = 0x1020, .line = 1, .column = 1, .file_index = 1}, // helper.asthra
        {.address = 0x1030, .line = 2, .column = 1, .file_index = 1}  // helper.asthra
    };
    
    for (int i = 0; i < 4; i++) {
        result = elf_writer_add_line_number_entry(fixture->elf_writer, &line_entries[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add line number entry %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Test file lookup
    uint32_t file_index = 0;
    result = elf_writer_lookup_source_file(fixture->elf_writer, 0x1020, &file_index);
    if (!asthra_test_assert_bool(context, result, "Failed to lookup source file")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, file_index == 1, "Expected file index 1, got %u", file_index)) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Main test function for ELF debug line numbers
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = {
        .name = "ELF Debug Line Numbers Tests",
        .description = "Test ELF line number debug information generation and lookup",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 15000000000ULL, // 15 seconds
        .max_parallel_tests = 1,
        .statistics = stats
    };
    
    AsthraTestFunction tests[] = {
        test_line_number_info,
        test_line_number_ordering,
        test_line_number_lookup,
        test_multi_file_line_numbers
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_line_number_info", __FILE__, __LINE__, "test_line_number_info", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_line_number_ordering", __FILE__, __LINE__, "test_line_number_ordering", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_line_number_lookup", __FILE__, __LINE__, "test_line_number_lookup", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_multi_file_line_numbers", __FILE__, __LINE__, "test_multi_file_line_numbers", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
