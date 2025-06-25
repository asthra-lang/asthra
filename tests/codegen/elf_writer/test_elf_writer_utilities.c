/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Utilities Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for ELF writer utility functions and statistics.
 */

#include "elf_writer_test_common.h"

void test_statistics_and_utilities(void) {
    TEST_SECTION("Statistics and Utility Tests");

    ELFWriter *test_writer = get_test_writer();
    if (!test_writer)
        return;

    printf("DEBUG: Setting up content for statistics test\n");
    // Set up some content
    elf_create_standard_sections(test_writer);
    elf_create_asthra_sections(test_writer);
    elf_add_symbol(test_writer, "test_sym", 0x1000, 64, STT_FUNC, STB_GLOBAL, 1);
    elf_add_relocation(test_writer, 0x1000, R_X86_64_PC32, 1, -4);

    printf("DEBUG: About to call elf_get_statistics\n");
    // Test statistics retrieval
    size_t sections, symbols, relocations, metadata_entries;
    elf_get_statistics(test_writer, &sections, &symbols, &relocations, &metadata_entries);
    printf("DEBUG: elf_get_statistics completed\n");

    TEST_ASSERT(sections > 0, "Statistics: sections count");
    TEST_ASSERT(symbols > 0, "Statistics: symbols count");
    TEST_ASSERT(relocations > 0, "Statistics: relocations count");

    printf("Statistics: %zu sections, %zu symbols, %zu relocations, %zu metadata entries\n",
           sections, symbols, relocations, metadata_entries);

    printf("DEBUG: About to call elf_print_structure\n");
    // Test structure printing (visual verification)
    printf("\nELF Structure:\n");
    elf_print_structure(test_writer);
    printf("DEBUG: elf_print_structure completed\n");
    TEST_ASSERT(true, "Structure printing completed");
}
