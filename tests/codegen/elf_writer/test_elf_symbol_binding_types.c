/**
 * Asthra Programming Language Compiler
 * ELF Symbol Binding Types Test
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test symbol binding types (local, global, weak)
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
 * Test symbol binding types
 */
AsthraTestResult test_symbol_binding_types(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture_local("test_binding.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test different binding types
    ELFSymbol symbols[3];
    symbols[0] = (ELFSymbol){.name = "global_func", .binding = STB_GLOBAL, .type = STT_FUNC};
    symbols[1] = (ELFSymbol){.name = "local_var", .binding = STB_LOCAL, .type = STT_OBJECT};
    symbols[2] = (ELFSymbol){.name = "weak_func", .binding = STB_WEAK, .type = STT_FUNC};

    for (int i = 0; i < 3; i++) {
        bool result = elf_symbol_manager_add_symbol(fixture->symbol_manager, &symbols[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add symbol with binding %d",
                                     symbols[i].binding)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Check binding validation
    if (!asthra_test_assert_bool(context,
                                 elf_symbol_manager_validate_bindings(fixture->symbol_manager),
                                 "Symbol binding validation failed")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check global symbol count
    size_t global_count = elf_symbol_manager_count_global_symbols(fixture->symbol_manager);
    if (!asthra_test_assert_bool(context, global_count == 2,
                                 "Expected 2 global symbols (including weak), got %zu",
                                 global_count)) {
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

    AsthraTestSuiteConfig config = {.name = "ELF Symbol Binding Types Test",
                                    .description =
                                        "Test symbol binding types (local, global, weak)",
                                    .parallel_execution = false,
                                    .stop_on_failure = false,
                                    .verbose_output = true,
                                    .json_output = false,
                                    .default_timeout_ns = 10000000000ULL, // 10 seconds
                                    .max_parallel_tests = 1,
                                    .statistics = stats};

    AsthraTestFunction tests[] = {test_symbol_binding_types};

    AsthraTestMetadata metadata[] = {{"test_symbol_binding_types", __FILE__, __LINE__,
                                      "test_symbol_binding_types", ASTHRA_TEST_SEVERITY_HIGH,
                                      5000000000ULL, false, NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);

    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}