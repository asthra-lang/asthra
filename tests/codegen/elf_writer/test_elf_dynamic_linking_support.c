/**
 * Asthra Programming Language Compiler
 * ELF Dynamic Linking Support Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test dynamic symbol table generation for external library functions
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
 * Test dynamic linking support
 */
AsthraTestResult test_dynamic_linking_support(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture_local("test_dynamic.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create dynamic symbols
    ELFSymbol dynamic_symbols[2];
    dynamic_symbols[0] = (ELFSymbol){
        .name = "printf", .binding = STB_GLOBAL, .section_index = SHN_UNDEF, .is_dynamic = true};
    dynamic_symbols[1] = (ELFSymbol){
        .name = "malloc", .binding = STB_GLOBAL, .section_index = SHN_UNDEF, .is_dynamic = true};

    for (int i = 0; i < 2; i++) {
        bool result = elf_symbol_manager_add_symbol(fixture->symbol_manager, &dynamic_symbols[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add dynamic symbol %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Generate dynamic symbol table
    Elf64_Sym *dynsym_table;
    size_t dynsym_count;
    bool result = elf_symbol_manager_generate_dynamic_table(fixture->symbol_manager, &dynsym_table,
                                                            &dynsym_count);
    if (!asthra_test_assert_bool(context, result, "Failed to generate dynamic symbol table")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check dynamic symbol count
    if (!asthra_test_assert_bool(context, dynsym_count >= 2,
                                 "Expected at least 2 dynamic symbols, got %zu", dynsym_count)) {
        free(dynsym_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that dynamic symbols are undefined
    for (size_t i = 1; i < dynsym_count; i++) { // Skip null symbol
        if (!asthra_test_assert_bool(context, dynsym_table[i].st_shndx == SHN_UNDEF,
                                     "Dynamic symbol %zu should be undefined", i)) {
            free(dynsym_table);
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    free(dynsym_table);
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Main test function
 */
int main(void) {
    AsthraTestStatistics *stats = asthra_test_statistics_create();

    AsthraTestSuiteConfig config = {
        .name = "ELF Dynamic Linking Support Test",
        .description = "Test dynamic symbol table generation",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 10000000000ULL, // 10 seconds
        .max_parallel_tests = 1,
        .statistics = stats};

    AsthraTestFunction tests[] = {test_dynamic_linking_support};

    AsthraTestMetadata metadata[] = {
        {"test_dynamic_linking_support", __FILE__, __LINE__, "test_dynamic_linking_support",
         ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);

    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}