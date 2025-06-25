/**
 * Asthra Programming Language Compiler
 * ELF Symbol Table Creation Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test symbol table creation functionality
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "ast.h"
#include "codegen_test_stubs.h"
#include "elf_compat.h"
#include "elf_relocation_manager.h"
#include "elf_symbol_manager.h"
#include "elf_writer.h"
#include "elf_writer_core.h"
#include "elf_writer_test_common.h"
#include "ffi_assembly_generator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Setup test fixture with an ELF writer (local implementation)
 */
static ELFWriterTestFixture *setup_elf_writer_fixture_local(const char *filename) {
    ELFWriterTestFixture *fixture = calloc(1, sizeof(ELFWriterTestFixture));
    if (!fixture)
        return NULL;

    fixture->output_filename = strdup(filename);
    if (!fixture->output_filename) {
        free(fixture);
        return NULL;
    }

    fixture->ffi_generator = ffi_assembly_generator_create();
    if (!fixture->ffi_generator) {
        free(fixture->output_filename);
        free(fixture);
        return NULL;
    }

    fixture->elf_writer = elf_writer_create(fixture->ffi_generator);
    if (!fixture->elf_writer) {
        ffi_assembly_generator_destroy(fixture->ffi_generator);
        free(fixture->output_filename);
        free(fixture);
        return NULL;
    }

    fixture->symbol_manager = elf_symbol_manager_create();
    if (!fixture->symbol_manager) {
        elf_writer_destroy(fixture->elf_writer);
        ffi_assembly_generator_destroy(fixture->ffi_generator);
        free(fixture->output_filename);
        free(fixture);
        return NULL;
    }

    fixture->relocation_manager = elf_relocation_manager_create();
    if (!fixture->relocation_manager) {
        elf_symbol_manager_destroy(fixture->symbol_manager);
        elf_writer_destroy(fixture->elf_writer);
        ffi_assembly_generator_destroy(fixture->ffi_generator);
        free(fixture->output_filename);
        free(fixture);
        return NULL;
    }

    return fixture;
}

/**
 * Cleanup test fixture
 */
static void cleanup_elf_writer_fixture(ELFWriterTestFixture *fixture) {
    if (!fixture)
        return;

    if (fixture->output_file) {
        fclose(fixture->output_file);
    }
    if (fixture->output_filename) {
        remove(fixture->output_filename);
        free(fixture->output_filename);
    }
    if (fixture->test_ast) {
        ast_free_node(fixture->test_ast);
    }
    if (fixture->relocation_manager) {
        elf_relocation_manager_destroy(fixture->relocation_manager);
    }
    if (fixture->symbol_manager) {
        elf_symbol_manager_destroy(fixture->symbol_manager);
    }
    if (fixture->elf_writer) {
        elf_writer_destroy(fixture->elf_writer);
    }
    if (fixture->ffi_generator) {
        ffi_assembly_generator_destroy(fixture->ffi_generator);
    }
    free(fixture);
}

/**
 * Test symbol table creation
 */
AsthraTestResult test_symbol_table_creation(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture_local("test_symbols.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Add symbols to symbol manager
    ELFSymbol symbols[4];
    symbols[0] = (ELFSymbol){.name = "main",
                             .value = 0x1000,
                             .size = 64,
                             .type = STT_FUNC,
                             .binding = STB_GLOBAL,
                             .section_index = 1};
    symbols[1] = (ELFSymbol){.name = "data_var",
                             .value = 0x2000,
                             .size = 8,
                             .type = STT_OBJECT,
                             .binding = STB_GLOBAL,
                             .section_index = 2};
    symbols[2] = (ELFSymbol){.name = "local_func",
                             .value = 0x1040,
                             .size = 32,
                             .type = STT_FUNC,
                             .binding = STB_LOCAL,
                             .section_index = 1};
    symbols[3] = (ELFSymbol){.name = "extern_func",
                             .value = 0,
                             .size = 0,
                             .type = STT_FUNC,
                             .binding = STB_GLOBAL,
                             .section_index = SHN_UNDEF};

    for (int i = 0; i < 4; i++) {
        bool result = elf_symbol_manager_add_symbol(fixture->symbol_manager, &symbols[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add symbol %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Generate symbol table
    Elf64_Sym *symbol_table;
    size_t symbol_count;
    bool result =
        elf_symbol_manager_generate_table(fixture->symbol_manager, &symbol_table, &symbol_count);
    if (!asthra_test_assert_bool(context, result, "Failed to generate symbol table")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check symbol count (including null symbol)
    if (!asthra_test_assert_bool(context, symbol_count == 5,
                                 "Expected 5 symbols (including null), got %zu", symbol_count)) {
        free(symbol_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check null symbol (first entry)
    if (!asthra_test_assert_bool(context, symbol_table[0].st_name == 0,
                                 "First symbol should be null symbol")) {
        free(symbol_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check main function symbol
    if (!asthra_test_assert_bool(context, ELF64_ST_TYPE(symbol_table[1].st_info) == STT_FUNC,
                                 "main symbol should be function type")) {
        free(symbol_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, ELF64_ST_BIND(symbol_table[1].st_info) == STB_GLOBAL,
                                 "main symbol should be global binding")) {
        free(symbol_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    free(symbol_table);
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Main test function
 */
int main(void) {
    AsthraTestStatistics *stats = asthra_test_statistics_create();

    AsthraTestSuiteConfig config = {.name = "ELF Symbol Table Creation Test",
                                    .description = "Test symbol table creation functionality",
                                    .parallel_execution = false,
                                    .stop_on_failure = false,
                                    .verbose_output = true,
                                    .json_output = false,
                                    .default_timeout_ns = 10000000000ULL, // 10 seconds
                                    .max_parallel_tests = 1,
                                    .statistics = stats};

    AsthraTestFunction tests[] = {test_symbol_table_creation};

    AsthraTestMetadata metadata[] = {{"test_symbol_table_creation", __FILE__, __LINE__,
                                      "test_symbol_table_creation", ASTHRA_TEST_SEVERITY_HIGH,
                                      5000000000ULL, false, NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);

    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}