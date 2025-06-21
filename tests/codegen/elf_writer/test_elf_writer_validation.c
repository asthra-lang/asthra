/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Validation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for ELF structure validation and optimization.
 */

#include "elf_writer_test_common.h"

void test_elf_validation(void) {
    TEST_SECTION("ELF Validation Tests");
    
    ELFWriter *test_writer = get_test_writer();
    if (!test_writer) return;
    
    // Set up a complete ELF structure
    elf_create_standard_sections(test_writer);
    elf_create_asthra_sections(test_writer);
    elf_populate_text_section(test_writer);
    elf_populate_ffi_section(test_writer);
    elf_populate_gc_section(test_writer);
    elf_generate_runtime_init(test_writer);
    
    // Test ELF structure validation
    bool success = elf_validate_structure(test_writer);
    TEST_ASSERT(success, "ELF structure validation");
    
    // Test C compatibility validation
    success = elf_validate_c_compatibility(test_writer);
    TEST_ASSERT(success, "C compatibility validation");
    
    // Test debug symbol validation
    success = elf_validate_debug_symbols(test_writer);
    TEST_ASSERT(success, "Debug symbol validation");
    
    // Test metadata optimization
    success = elf_optimize_metadata(test_writer);
    TEST_ASSERT(success, "Metadata optimization");
} 
