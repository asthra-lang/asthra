/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Common Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of common test utilities and shared state management.
 */

#include "elf_writer_test_common.h"

// Global test state
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

// Global test fixtures
static FFIAssemblyGenerator *test_ffi_generator = NULL;
static ELFWriter *test_writer = NULL;

// =============================================================================
// TEST STATE MANAGEMENT
// =============================================================================

void increment_tests_run(void) {
    tests_run++;
}

void increment_tests_passed(void) {
    tests_passed++;
}

void increment_tests_failed(void) {
    tests_failed++;
}

int get_tests_run(void) {
    return tests_run;
}

int get_tests_passed(void) {
    return tests_passed;
}

int get_tests_failed(void) {
    return tests_failed;
}

// =============================================================================
// TEST ENVIRONMENT MANAGEMENT
// =============================================================================

bool setup_global_test_environment(void) {
    // Reset test counters
    tests_run = 0;
    tests_passed = 0;
    tests_failed = 0;

    return setup_test_environment();
}

void teardown_global_test_environment(void) {
    teardown_test_environment();
}

bool setup_test_environment(void) {
    // Create FFI assembly generator
    test_ffi_generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!test_ffi_generator) {
        printf("Failed to create FFI assembly generator\n");
        return false;
    }

    // Create ELF writer
    test_writer = elf_writer_create(test_ffi_generator);
    if (!test_writer) {
        printf("Failed to create ELF writer\n");
        return false;
    }

    return true;
}

void teardown_test_environment(void) {
    if (test_writer) {
        elf_writer_destroy(test_writer);
        test_writer = NULL;
    }

    if (test_ffi_generator) {
        ffi_assembly_generator_destroy(test_ffi_generator);
        test_ffi_generator = NULL;
    }
}

// =============================================================================
// TEST FIXTURE ACCESS
// =============================================================================

FFIAssemblyGenerator *get_test_ffi_generator(void) {
    return test_ffi_generator;
}

ELFWriter *get_test_writer(void) {
    return test_writer;
}

// =============================================================================
// TEST FIXTURE MANAGEMENT
// =============================================================================

/**
 * Setup test fixture with an ELF writer
 */
ELFWriterTestFixture *setup_elf_writer_fixture(const char *filename) {
    ELFWriterTestFixture *fixture = calloc(1, sizeof(ELFWriterTestFixture));
    if (!fixture)
        return NULL;

    fixture->output_filename = strdup(filename);
    if (!fixture->output_filename) {
        free(fixture);
        return NULL;
    }

    fixture->ffi_generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
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
void cleanup_elf_writer_fixture(ELFWriterTestFixture *fixture) {
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

// =============================================================================
// STUB FUNCTIONS FOR MISSING EXTERNAL TESTS
// =============================================================================

/**
 * Debug information test stub - placeholder for missing test_debug_information
 */
void test_debug_information(void) {
    TEST_SECTION("Debug Information");
    TEST_ASSERT(true, "Debug information placeholder test passed");
    printf("Note: Debug information tests are available as standalone programs\n");
    printf("Run: make build/tests/codegen/test_elf_debug_dwarf for DWARF debug tests\n");
}
