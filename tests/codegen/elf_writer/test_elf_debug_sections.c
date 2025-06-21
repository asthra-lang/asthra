/**
 * Asthra Programming Language Compiler
 * ELF Debug Sections Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test ELF debug section generation and management
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
// DEBUG SECTION TESTS
// =============================================================================

/**
 * Test debug section generation
 */
AsthraTestResult test_debug_section_generation(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_debug.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Enable debug information generation
    elf_writer_set_debug_enabled(fixture->elf_writer, true);
    
    // Create debug sections
    ELFDebugSection debug_sections[3];
    debug_sections[0] = (ELFDebugSection){.name = ".debug_info", .type = DEBUG_INFO, .size = 1024};
    debug_sections[1] = (ELFDebugSection){.name = ".debug_line", .type = DEBUG_LINE, .size = 512};
    debug_sections[2] = (ELFDebugSection){.name = ".debug_str", .type = DEBUG_STR, .size = 256};
    
    for (int i = 0; i < 3; i++) {
        bool result = elf_writer_add_debug_section(fixture->elf_writer, &debug_sections[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add debug section %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Generate debug sections
    bool result = elf_writer_generate_debug_sections(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Failed to generate debug sections")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that debug sections were created
    size_t debug_section_count = elf_writer_get_debug_section_count(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, debug_section_count == 3,
                                "Expected 3 debug sections, got %zu", debug_section_count)) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test debug information compression
 */
AsthraTestResult test_debug_info_compression(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_debug_compression.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Enable debug compression
    elf_writer_set_debug_compression(fixture->elf_writer, true);
    
    // Add substantial debug information to test compression
    for (int i = 0; i < 100; i++) {
        char name[32];
        snprintf(name, sizeof(name), "variable_%d", i);
        
        ELFVariableDebugInfo var_info = {
            .name = name,
            .type = "int",
            .location = VAR_LOCATION_STACK,
            .stack_offset = -8 * (i + 1),
            .scope_start = 0x1000 + i * 0x10,
            .scope_end = 0x1000 + (i + 1) * 0x10
        };
        
        bool result = elf_writer_add_variable_debug_info(fixture->elf_writer, &var_info);
        if (!asthra_test_assert_bool(context, result, "Failed to add variable debug info %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Generate compressed debug information
    bool result = elf_writer_generate_compressed_debug_info(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Failed to generate compressed debug info")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check compression ratio
    size_t uncompressed_size = elf_writer_get_uncompressed_debug_size(fixture->elf_writer);
    size_t compressed_size = elf_writer_get_compressed_debug_size(fixture->elf_writer);
    
    if (!asthra_test_assert_bool(context, compressed_size < uncompressed_size,
                                "Compressed size should be smaller than uncompressed")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check compression ratio is reasonable (at least 10% compression)
    double compression_ratio = (double)(uncompressed_size - compressed_size) / uncompressed_size;
    if (!asthra_test_assert_bool(context, compression_ratio > 0.1,
                                "Compression ratio should be at least 10%%, got %.2f%%", 
                                compression_ratio * 100)) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test debug information validation
 */
AsthraTestResult test_debug_info_validation(AsthraTestContext* context) {
    ELFWriterTestFixture* fixture = setup_elf_writer_fixture("test_debug_validation.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Add valid debug information
    ELFLineNumberEntry valid_line = {
        .address = 0x1000,
        .line = 1,
        .column = 1,
        .file_index = 0
    };
    
    bool result = elf_writer_add_line_number_entry(fixture->elf_writer, &valid_line);
    if (!asthra_test_assert_bool(context, result, "Failed to add valid line number entry")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Test invalid line number entry
    ELFLineNumberEntry invalid_line = {
        .address = 0x1000,
        .line = 0, // Invalid line number
        .column = 1,
        .file_index = 0
    };
    
    result = elf_writer_add_line_number_entry(fixture->elf_writer, &invalid_line);
    if (!asthra_test_assert_bool(context, !result, "Should reject invalid line number entry")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Validate all debug information
    result = elf_writer_validate_debug_info(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Debug info validation should pass")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Main test function for ELF debug sections
 */
int main(void) {
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    
    AsthraTestSuiteConfig config = {
        .name = "ELF Debug Sections Tests",
        .description = "Test ELF debug section generation, compression, and validation",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 15000000000ULL, // 15 seconds
        .max_parallel_tests = 1,
        .statistics = stats
    };
    
    AsthraTestFunction tests[] = {
        test_debug_section_generation,
        test_debug_info_compression,
        test_debug_info_validation
    };
    
    AsthraTestMetadata metadata[] = {
        {"test_debug_section_generation", __FILE__, __LINE__, "test_debug_section_generation", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_debug_info_compression", __FILE__, __LINE__, "test_debug_info_compression", ASTHRA_TEST_SEVERITY_LOW, 10000000000ULL, false, NULL},
        {"test_debug_info_validation", __FILE__, __LINE__, "test_debug_info_validation", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);
    
    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
} 
