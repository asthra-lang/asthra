/**
 * Asthra Programming Language Compiler
 * ELF Symbol Table Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test ELF symbol table generation, symbol binding types, and visibility
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
// SYMBOL TABLE GENERATION TESTS
// =============================================================================

/**
 * Test symbol table creation
 */
AsthraTestResult test_symbol_table_creation(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_symbols.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Add symbols to symbol manager
    ELFSymbol symbols[4];
    symbols[0] = (ELFSymbol){.name = "main", .value = 0x1000, .size = 64, .type = STT_FUNC, .binding = STB_GLOBAL, .section_index = 1};
    symbols[1] = (ELFSymbol){.name = "data_var", .value = 0x2000, .size = 8, .type = STT_OBJECT, .binding = STB_GLOBAL, .section_index = 2};
    symbols[2] = (ELFSymbol){.name = "local_func", .value = 0x1040, .size = 32, .type = STT_FUNC, .binding = STB_LOCAL, .section_index = 1};
    symbols[3] = (ELFSymbol){.name = "extern_func", .value = 0, .size = 0, .type = STT_FUNC, .binding = STB_GLOBAL, .section_index = SHN_UNDEF};
    
    for (int i = 0; i < 4; i++) {
        bool result = elf_symbol_manager_add_symbol(fixture->symbol_manager, &symbols[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add symbol %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Generate symbol table
    Elf64_Sym* symbol_table;
    size_t symbol_count;
    bool result = elf_symbol_manager_generate_table(fixture->symbol_manager, &symbol_table, &symbol_count);
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
 * Test symbol binding types
 */
AsthraTestResult test_symbol_binding_types(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_binding.o");
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
        if (!asthra_test_assert_bool(context, result, "Failed to add symbol with binding %d", symbols[i].binding)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Check binding validation
    if (!asthra_test_assert_bool(context, elf_symbol_manager_validate_bindings(fixture->symbol_manager),
                                "Symbol binding validation failed")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check global symbol count
    size_t global_count = elf_symbol_manager_count_global_symbols(fixture->symbol_manager);
    if (!asthra_test_assert_bool(context, global_count == 2,
                                "Expected 2 global symbols (including weak), got %zu", global_count)) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test symbol visibility
 */
AsthraTestResult test_symbol_visibility(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_visibility.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test different visibility types
    ELFSymbol symbols[4];
    symbols[0] = (ELFSymbol){.name = "default_func", .visibility = STV_DEFAULT, .binding = STB_GLOBAL};
    symbols[1] = (ELFSymbol){.name = "hidden_func", .visibility = STV_HIDDEN, .binding = STB_GLOBAL};
    symbols[2] = (ELFSymbol){.name = "protected_func", .visibility = STV_PROTECTED, .binding = STB_GLOBAL};
    symbols[3] = (ELFSymbol){.name = "internal_func", .visibility = STV_INTERNAL, .binding = STB_LOCAL};
    
    for (int i = 0; i < 4; i++) {
        bool result = elf_symbol_manager_add_symbol(fixture->symbol_manager, &symbols[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add symbol with visibility %d", symbols[i].visibility)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Generate symbol table and check visibility encoding
    Elf64_Sym* symbol_table;
    size_t symbol_count;
    bool result = elf_symbol_manager_generate_table(fixture->symbol_manager, &symbol_table, &symbol_count);
    if (!asthra_test_assert_bool(context, result, "Failed to generate symbol table for visibility test")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check visibility encoding in st_other field
    if (!asthra_test_assert_bool(context, ELF64_ST_VISIBILITY(symbol_table[1].st_other) == STV_DEFAULT,
                                "Default visibility not encoded correctly")) {
        free(symbol_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, ELF64_ST_VISIBILITY(symbol_table[2].st_other) == STV_HIDDEN,
                                "Hidden visibility not encoded correctly")) {
        free(symbol_table);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    free(symbol_table);
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test dynamic linking support
 */
AsthraTestResult test_dynamic_linking_support(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_dynamic.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create dynamic symbols
    ELFSymbol dynamic_symbols[2];
    dynamic_symbols[0] = (ELFSymbol){.name = "printf", .binding = STB_GLOBAL, .section_index = SHN_UNDEF, .is_dynamic = true};
    dynamic_symbols[1] = (ELFSymbol){.name = "malloc", .binding = STB_GLOBAL, .section_index = SHN_UNDEF, .is_dynamic = true};
    
    for (int i = 0; i < 2; i++) {
        bool result = elf_symbol_manager_add_symbol(fixture->symbol_manager, &dynamic_symbols[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add dynamic symbol %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Generate dynamic symbol table
    Elf64_Sym* dynsym_table;
    size_t dynsym_count;
    bool result = elf_symbol_manager_generate_dynamic_table(fixture->symbol_manager, &dynsym_table, &dynsym_count);
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
 * Test symbol lookup and resolution
 */
AsthraTestResult test_symbol_lookup_resolution(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_lookup.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Add symbols with different scopes
    ELFSymbol symbols[5];
    symbols[0] = (ELFSymbol){.name = "main", .value = 0x1000, .binding = STB_GLOBAL, .type = STT_FUNC};
    symbols[1] = (ELFSymbol){.name = "helper", .value = 0x1100, .binding = STB_LOCAL, .type = STT_FUNC};
    symbols[2] = (ELFSymbol){.name = "global_var", .value = 0x2000, .binding = STB_GLOBAL, .type = STT_OBJECT};
    symbols[3] = (ELFSymbol){.name = "static_var", .value = 0x2100, .binding = STB_LOCAL, .type = STT_OBJECT};
    symbols[4] = (ELFSymbol){.name = "extern_var", .value = 0, .binding = STB_GLOBAL, .section_index = SHN_UNDEF};
    
    for (int i = 0; i < 5; i++) {
        bool result = elf_symbol_manager_add_symbol(fixture->symbol_manager, &symbols[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add symbol %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Test symbol lookup
    ELFSymbol* found_symbol = elf_symbol_manager_lookup_symbol(fixture->symbol_manager, "main");
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
    size_t symbol_index = elf_symbol_manager_get_symbol_index(fixture->symbol_manager, "global_var");
    if (!asthra_test_assert_bool(context, symbol_index != SIZE_MAX,
                                "Failed to get symbol index for 'global_var'")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test symbol table sorting and ordering
 */
AsthraTestResult test_symbol_table_ordering(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture_local("test_ordering.o");
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
    Elf64_Sym* symbol_table;
    size_t symbol_count;
    bool result = elf_symbol_manager_generate_sorted_table(fixture->symbol_manager, &symbol_table, &symbol_count);
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
                                    "Symbol %zu should be global or weak, got binding %d", i, binding)) {
            free(symbol_table);
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    free(symbol_table);
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Main test function for ELF symbol tables
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = {
        .name = "ELF Symbol Table Tests",
        .description = "Test ELF symbol table generation, binding types, and visibility",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 10000000000ULL, // 10 seconds
        .max_parallel_tests = 1,
        .statistics = stats
    };
    
    AsthraTestFunction tests[] = {
        test_symbol_table_creation,
        test_symbol_binding_types,
        test_symbol_visibility,
        test_dynamic_linking_support,
        test_symbol_lookup_resolution,
        test_symbol_table_ordering
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_symbol_table_creation", __FILE__, __LINE__, "test_symbol_table_creation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_symbol_binding_types", __FILE__, __LINE__, "test_symbol_binding_types", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_symbol_visibility", __FILE__, __LINE__, "test_symbol_visibility", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_dynamic_linking_support", __FILE__, __LINE__, "test_dynamic_linking_support", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_symbol_lookup_resolution", __FILE__, __LINE__, "test_symbol_lookup_resolution", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
        {"test_symbol_table_ordering", __FILE__, __LINE__, "test_symbol_table_ordering", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
