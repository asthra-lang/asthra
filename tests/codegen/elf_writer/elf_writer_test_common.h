/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Common Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common definitions, macros, and utilities for ELF writer tests.
 */

#ifndef ELF_WRITER_TEST_COMMON_H
#define ELF_WRITER_TEST_COMMON_H

#include "ast.h"
#include "dwarf_constants.h"
#include "elf_relocation_manager.h"
#include "elf_symbol_manager.h"
#include "elf_writer.h"
#include "elf_writer_core.h"
#include "elf_writer_relocations.h"
#include "elf_writer_symbols.h"
#include "ffi_assembly_generator.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// =============================================================================
// TEST FIXTURE DEFINITIONS
// =============================================================================

/**
 * Test fixture for ELF writer testing
 */
typedef struct {
    ELFWriter *elf_writer;
    FFIAssemblyGenerator *ffi_generator;
    ELFSymbolManager *symbol_manager;
    ELFRelocationManager *relocation_manager;
    ASTNode *test_ast;
    char *output_filename;
    FILE *output_file;
} ELFWriterTestFixture;

// =============================================================================
// TEST MACROS
// =============================================================================

// Test macros
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        increment_tests_run();                                                                     \
        if (condition) {                                                                           \
            increment_tests_passed();                                                              \
            printf("✓ %s\n", message);                                                             \
        } else {                                                                                   \
            increment_tests_failed();                                                              \
            printf("✗ %s\n", message);                                                             \
        }                                                                                          \
    } while (0)

#define TEST_SECTION(name) printf("\n=== %s ===\n", name)

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Global test state management
void increment_tests_run(void);
void increment_tests_passed(void);
void increment_tests_failed(void);
int get_tests_run(void);
int get_tests_passed(void);
int get_tests_failed(void);

// Test environment management
bool setup_global_test_environment(void);
void teardown_global_test_environment(void);
bool setup_test_environment(void);
void teardown_test_environment(void);

// Global test fixtures access
FFIAssemblyGenerator *get_test_ffi_generator(void);
ELFWriter *get_test_writer(void);

// Test fixture management
ELFWriterTestFixture *setup_elf_writer_fixture(const char *filename);
void cleanup_elf_writer_fixture(ELFWriterTestFixture *fixture);

#endif // ELF_WRITER_TEST_COMMON_H
