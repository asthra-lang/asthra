/**
 * Asthra Programming Language Compiler
 * ELF Symbol Table Ordering Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test symbol table sorting and ordering (local symbols before global/weak)
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
 * Test symbol table sorting and ordering
 */
AsthraTestResult test_symbol_table_ordering(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture_local("test_ordering.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Add symbols in random order to test sorting
    ELFSymbol symbols[6];
    symbols[0] = (ELFSymbol){.name = "z_global", .binding = STB_GLOBAL, .type = STT_FUNC};
    symbols[1] = (ELFSymbol){.name = "a_local", .binding = STB_LOCAL, .type = STT_FUNC};
    symbols[2] = (ELFSymbol){.name = "m_global", .binding = STB_GLOBAL, .type = STT_OBJECT};
    symbols[3] = (ELFSymbol){.name = "b_local", .binding = STB_LOCAL, .type = STT_OBJECT};
    symbols[4] = (ELFSymbol){.name = "weak_symbol", .binding = STB_WEAK, .type = STT_FUNC};
    symbols[5] = (ELFSymbol){.name = "c_local", .binding = STB_LOCAL, .type = STT_NOTYPE};

    for (int i = 0; i < 6; i++) {
        bool result = elf_symbol_manager_add_symbol(fixture->symbol_manager, &symbols[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add symbol %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Generate sorted symbol table
    Elf64_Sym *symbol_table;
    size_t symbol_count;
    bool result = elf_symbol_manager_generate_sorted_table(fixture->symbol_manager, &symbol_table,
                                                           &symbol_count);
    if (!asthra_test_assert_bool(context, result, "Failed to generate sorted symbol table")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Verify ordering: local symbols come first, then global/weak
    size_t local_end_index = 0;
    for (size_t i = 1; i < symbol_count; i++) { // Skip null symbol
        if (ELF64_ST_BIND(symbol_table[i].st_info) == STB_LOCAL) {
            local_end_index = i;
        } else {
            // Once we hit a non-local, all remaining should be non-local
            break;
        }
    }

    // Check that all symbols after local_end_index are global or weak
    for (size_t i = local_end_index + 1; i < symbol_count; i++) {
        unsigned char binding = ELF64_ST_BIND(symbol_table[i].st_info);
        if (!asthra_test_assert_bool(context, binding == STB_GLOBAL || binding == STB_WEAK,
                                     "Symbol %zu should be global or weak, got binding %d", i,
                                     binding)) {
            free(symbol_table);
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
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

    AsthraTestSuiteConfig config = {.name = "ELF Symbol Table Ordering Test",
                                    .description = "Test symbol table sorting and ordering",
                                    .parallel_execution = false,
                                    .stop_on_failure = false,
                                    .verbose_output = true,
                                    .json_output = false,
                                    .default_timeout_ns = 10000000000ULL, // 10 seconds
                                    .max_parallel_tests = 1,
                                    .statistics = stats};

    AsthraTestFunction tests[] = {test_symbol_table_ordering};

    AsthraTestMetadata metadata[] = {{"test_symbol_table_ordering", __FILE__, __LINE__,
                                      "test_symbol_table_ordering", ASTHRA_TEST_SEVERITY_MEDIUM,
                                      5000000000ULL, false, NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);

    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}