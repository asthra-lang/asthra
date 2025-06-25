/**
 * Asthra Programming Language Compiler
 * ELF Symbol Lookup and Resolution Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test symbol lookup by name and resolution by index
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
 * Test symbol lookup and resolution
 */
AsthraTestResult test_symbol_lookup_resolution(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture_local("test_lookup.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Add symbols with different scopes
    ELFSymbol symbols[5];
    symbols[0] =
        (ELFSymbol){.name = "main", .value = 0x1000, .binding = STB_GLOBAL, .type = STT_FUNC};
    symbols[1] =
        (ELFSymbol){.name = "helper", .value = 0x1100, .binding = STB_LOCAL, .type = STT_FUNC};
    symbols[2] = (ELFSymbol){
        .name = "global_var", .value = 0x2000, .binding = STB_GLOBAL, .type = STT_OBJECT};
    symbols[3] = (ELFSymbol){
        .name = "static_var", .value = 0x2100, .binding = STB_LOCAL, .type = STT_OBJECT};
    symbols[4] = (ELFSymbol){
        .name = "extern_var", .value = 0, .binding = STB_GLOBAL, .section_index = SHN_UNDEF};

    for (int i = 0; i < 5; i++) {
        bool result = elf_symbol_manager_add_symbol(fixture->symbol_manager, &symbols[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add symbol %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test symbol lookup
    ELFSymbol *found_symbol = elf_symbol_manager_lookup_symbol(fixture->symbol_manager, "main");
    if (!asthra_test_assert_pointer(context, found_symbol, "Failed to lookup 'main' symbol")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_int_eq(context, found_symbol->value, 0x1000,
                                   "Main symbol value mismatch")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Test lookup of non-existent symbol
    found_symbol = elf_symbol_manager_lookup_symbol(fixture->symbol_manager, "nonexistent");
    if (!asthra_test_assert_pointer_null(context, found_symbol,
                                         "Lookup of non-existent symbol should return NULL")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Test symbol resolution by index
    size_t symbol_index =
        elf_symbol_manager_get_symbol_index(fixture->symbol_manager, "global_var");
    if (!asthra_test_assert_bool(context, symbol_index != SIZE_MAX,
                                 "Failed to get symbol index for 'global_var'")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Main test function
 */
int main(void) {
    AsthraTestStatistics *stats = asthra_test_statistics_create();

    AsthraTestSuiteConfig config = {.name = "ELF Symbol Lookup and Resolution Test",
                                    .description =
                                        "Test symbol lookup by name and resolution by index",
                                    .parallel_execution = false,
                                    .stop_on_failure = false,
                                    .verbose_output = true,
                                    .json_output = false,
                                    .default_timeout_ns = 10000000000ULL, // 10 seconds
                                    .max_parallel_tests = 1,
                                    .statistics = stats};

    AsthraTestFunction tests[] = {test_symbol_lookup_resolution};

    AsthraTestMetadata metadata[] = {{"test_symbol_lookup_resolution", __FILE__, __LINE__,
                                      "test_symbol_lookup_resolution", ASTHRA_TEST_SEVERITY_HIGH,
                                      5000000000ULL, false, NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);

    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}