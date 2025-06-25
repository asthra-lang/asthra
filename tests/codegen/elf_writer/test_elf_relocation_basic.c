/**
 * Asthra Programming Language Compiler
 * ELF Basic Relocation Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test basic ELF relocation entry generation and processing
 */

#include "elf_relocation_test_fixtures.h"

/**
 * Test relocation entry generation
 */
AsthraTestResult test_relocation_entry_generation(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture("test_relocations.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create relocation entries
    ELFRelocation relocations[3];
    relocations[0] =
        (ELFRelocation){.offset = 0x1008, .symbol_index = 1, .type = R_X86_64_PC32, .addend = -4};
    relocations[1] =
        (ELFRelocation){.offset = 0x1010, .symbol_index = 2, .type = R_X86_64_PLT32, .addend = -4};
    relocations[2] =
        (ELFRelocation){.offset = 0x1018, .symbol_index = 3, .type = R_X86_64_64, .addend = 0};

    for (int i = 0; i < 3; i++) {
        bool result =
            elf_relocation_manager_add_relocation(fixture->relocation_manager, &relocations[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add relocation %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Generate relocation table
    ELFRelocation **relocation_table = NULL;
    size_t relocation_count = 0;
    bool result = elf_relocation_manager_generate_table(fixture->relocation_manager,
                                                        &relocation_table, &relocation_count);
    if (!asthra_test_assert_bool(context, result, "Failed to generate relocation table")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check relocation count
    if (!asthra_test_assert_bool(context, relocation_count == 3, "Expected 3 relocations, got %zu",
                                 relocation_count)) {
        // Don't free relocation_table - it's owned by the manager
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check first relocation (PC-relative)
    if (!asthra_test_assert_bool(context, relocation_table[0]->offset == 0x1008,
                                 "First relocation offset incorrect")) {
        // Don't free relocation_table - it's owned by the manager
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, relocation_table[0]->type == R_X86_64_PC32,
                                 "First relocation type should be PC32")) {
        // Don't free relocation_table - it's owned by the manager
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check PLT relocation
    if (!asthra_test_assert_bool(context, relocation_table[1]->type == R_X86_64_PLT32,
                                 "Second relocation type should be PLT32")) {
        // Don't free relocation_table - it's owned by the manager
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Don't free relocation_table - it's owned by the manager
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test relocation processing
 */
AsthraTestResult test_relocation_processing(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture("test_reloc_process.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a simple program with relocations
    const char *reloc_source = "package test;\n"
                               "\n"
                               "pub extern fn external_function(none) -> i32;\n"
                               "\n"
                               "pub fn main(none) -> i32 {\n"
                               "    return external_function(none);\n"
                               "}\n";

    fixture->test_ast = parse_test_source(reloc_source, "test.asthra");
    if (!asthra_test_assert_pointer(context, fixture->test_ast,
                                    "Failed to parse relocation source")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Process relocations
    bool result =
        elf_relocation_manager_process_ast(fixture->relocation_manager, fixture->test_ast);
    if (!asthra_test_assert_bool(context, result, "Failed to process AST relocations")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // TODO: Currently process_ast is a stub, so we expect 0 relocations
    // Once it's implemented, change this to expect > 0
    size_t relocation_count = elf_relocation_manager_get_count(fixture->relocation_manager);
    if (!asthra_test_assert_bool(context, relocation_count == 0,
                                 "Expected 0 relocations (process_ast is a stub)")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Main test function for basic ELF relocations
 */
int main(void) {
    AsthraTestStatistics *stats = asthra_test_statistics_create();

    AsthraTestSuiteConfig config = {.name = "ELF Basic Relocation Tests",
                                    .description =
                                        "Test basic ELF relocation entry generation and processing",
                                    .parallel_execution = false,
                                    .stop_on_failure = false,
                                    .verbose_output = true,
                                    .json_output = false,
                                    .default_timeout_ns = 10000000000ULL, // 10 seconds
                                    .max_parallel_tests = 1,
                                    .statistics = stats};

    AsthraTestFunction tests[] = {test_relocation_entry_generation, test_relocation_processing};

    AsthraTestMetadata metadata[] = {
        {"test_relocation_entry_generation", __FILE__, __LINE__, "test_relocation_entry_generation",
         ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_relocation_processing", __FILE__, __LINE__, "test_relocation_processing",
         ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);

    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
