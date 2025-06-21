/**
 * Asthra Programming Language Compiler
 * ELF Structure Generation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test ELF file structure generation including headers, sections, and program segments
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
// TEST FIXTURE AND UTILITIES
// =============================================================================

// ELFWriterTestFixture is defined in elf_writer_test_common.h

/**
 * Setup test fixture with an ELF writer (local implementation)
 */
static ELFWriterTestFixture* setup_elf_writer_fixture_local(const char* filename) {
    ELFWriterTestFixture* fixture = calloc(1, sizeof(ELFWriterTestFixture));
    if (!fixture) return NULL;
    
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
static void cleanup_elf_writer_fixture(ELFWriterTestFixture* fixture) {
    if (!fixture) return;
    
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
// ELF STRUCTURE GENERATION TESTS
// =============================================================================

/**
 * Test ELF header generation
 */
AsthraTestResult test_elf_header_generation(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_header.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Generate ELF header
    Elf64_Ehdr header;
    bool result = elf_writer_generate_header(fixture->elf_writer, &header);
    if (!asthra_test_assert_bool(context, result, "Failed to generate ELF header")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check ELF magic number
    if (!asthra_test_assert_bool(context, header.e_ident[EI_MAG0] == ELFMAG0,
                                "ELF magic byte 0 incorrect")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, header.e_ident[EI_MAG1] == ELFMAG1,
                                "ELF magic byte 1 incorrect")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check ELF class (64-bit)
    if (!asthra_test_assert_bool(context, header.e_ident[EI_CLASS] == ELFCLASS64,
                                "ELF class should be 64-bit")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check machine type (x86_64)
    if (!asthra_test_assert_bool(context, header.e_machine == EM_X86_64,
                                "ELF machine type should be x86_64")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check object file type
    if (!asthra_test_assert_bool(context, header.e_type == ET_REL,
                                "ELF type should be relocatable object")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test section header generation
 */
AsthraTestResult test_section_header_generation(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_sections.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create sections
    ELFSection sections[4];
    sections[0] = (ELFSection){.name = ".text", .type = SHT_PROGBITS, .flags = SHF_ALLOC | SHF_EXECINSTR, .size = 1024};
    sections[1] = (ELFSection){.name = ".data", .type = SHT_PROGBITS, .flags = SHF_ALLOC | SHF_WRITE, .size = 512};
    sections[2] = (ELFSection){.name = ".bss", .type = SHT_NOBITS, .flags = SHF_ALLOC | SHF_WRITE, .size = 256};
    sections[3] = (ELFSection){.name = ".symtab", .type = SHT_SYMTAB, .flags = 0, .size = 2048};
    
    // Generate section headers
    Elf64_Shdr headers[4];
    for (int i = 0; i < 4; i++) {
        bool result = elf_writer_generate_section_header(fixture->elf_writer, &sections[i], &headers[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to generate section header %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Check .text section header
    if (!asthra_test_assert_bool(context, headers[0].sh_type == SHT_PROGBITS,
                                ".text section should be PROGBITS")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, headers[0].sh_flags == (SHF_ALLOC | SHF_EXECINSTR),
                                ".text section should be allocatable and executable")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check .bss section header
    if (!asthra_test_assert_bool(context, headers[2].sh_type == SHT_NOBITS,
                                ".bss section should be NOBITS")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check .symtab section header
    if (!asthra_test_assert_bool(context, headers[3].sh_type == SHT_SYMTAB,
                                ".symtab section should be SYMTAB")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test program header generation
 */
AsthraTestResult test_program_header_generation(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_program.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create program segments
    ELFSegment segments[2];
    segments[0] = (ELFSegment){.type = PT_LOAD, .flags = PF_R | PF_X, .vaddr = 0x400000, .paddr = 0x400000, .filesz = 1024, .memsz = 1024};
    segments[1] = (ELFSegment){.type = PT_LOAD, .flags = PF_R | PF_W, .vaddr = 0x600000, .paddr = 0x600000, .filesz = 512, .memsz = 768};
    
    // Generate program headers
    Elf64_Phdr headers[2];
    for (int i = 0; i < 2; i++) {
        bool result = elf_writer_generate_program_header(fixture->elf_writer, &segments[i], &headers[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to generate program header %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Check first segment (executable)
    if (!asthra_test_assert_bool(context, headers[0].p_type == PT_LOAD,
                                "First segment should be LOAD type")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, headers[0].p_flags == (PF_R | PF_X),
                                "First segment should be readable and executable")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check second segment (data)
    if (!asthra_test_assert_bool(context, headers[1].p_flags == (PF_R | PF_W),
                                "Second segment should be readable and writable")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check memory size >= file size
    if (!asthra_test_assert_bool(context, headers[1].p_memsz >= headers[1].p_filesz,
                                "Memory size should be >= file size")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test ELF file layout validation
 */
AsthraTestResult test_elf_file_layout_validation(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_layout.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Generate a complete ELF layout
    bool result = elf_writer_generate_complete_layout(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Failed to generate complete ELF layout")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Validate file layout
    result = elf_writer_validate_layout(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "ELF layout validation failed")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check section alignment
    size_t section_count = elf_writer_get_section_count(fixture->elf_writer);
    for (size_t i = 0; i < section_count; i++) {
        uint64_t section_offset = elf_writer_get_section_offset(fixture->elf_writer, i);
        uint64_t section_align = elf_writer_get_section_alignment(fixture->elf_writer, i);
        
        if (section_align > 0) {
            if (!asthra_test_assert_bool(context, (section_offset % section_align) == 0,
                                        "Section %zu offset not aligned to %lu bytes", i, section_align)) {
                cleanup_elf_writer_fixture(fixture);
                return ASTHRA_TEST_FAIL;
            }
        }
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test string table generation
 */
AsthraTestResult test_string_table_generation(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_strings.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Add strings to string table
    const char* test_strings[] = {
        ".text",
        ".data", 
        ".bss",
        ".symtab",
        ".strtab",
        "main",
        "printf",
        "malloc",
        NULL
    };
    
    uint32_t string_offsets[8];
    for (int i = 0; test_strings[i] != NULL; i++) {
        string_offsets[i] = elf_writer_add_string(fixture->elf_writer, test_strings[i]);
        if (!asthra_test_assert_bool(context, string_offsets[i] != 0 || i == 0,
                                    "Failed to add string '%s' to string table", test_strings[i])) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Generate string table
    char* string_table;
    size_t string_table_size;
    bool result = elf_writer_generate_string_table(fixture->elf_writer, &string_table, &string_table_size);
    if (!asthra_test_assert_bool(context, result, "Failed to generate string table")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify string table contents
    if (!asthra_test_assert_bool(context, string_table_size > 0,
                                "String table should have non-zero size")) {
        free(string_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that strings are properly stored
    for (int i = 0; test_strings[i] != NULL; i++) {
        const char* stored_string = string_table + string_offsets[i];
        if (!asthra_test_assert_string_eq(context, stored_string, test_strings[i],
                                         "String %d mismatch in string table", i)) {
            free(string_table);
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    free(string_table);
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Main test function for ELF structure generation
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = {
        .name = "ELF Structure Generation Tests",
        .description = "Test ELF file structure generation including headers, sections, and layout",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 10000000000ULL, // 10 seconds
        .max_parallel_tests = 1,
        .statistics = stats
    };
    
    AsthraTestFunction tests[] = {
        test_elf_header_generation,
        test_section_header_generation,
        test_program_header_generation,
        test_elf_file_layout_validation,
        test_string_table_generation
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_elf_header_generation", __FILE__, __LINE__, "test_elf_header_generation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_section_header_generation", __FILE__, __LINE__, "test_section_header_generation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_program_header_generation", __FILE__, __LINE__, "test_program_header_generation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_elf_file_layout_validation", __FILE__, __LINE__, "test_elf_file_layout_validation", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_string_table_generation", __FILE__, __LINE__, "test_string_table_generation", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
