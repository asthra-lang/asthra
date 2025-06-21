/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Metadata Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for metadata population in Asthra-specific ELF sections.
 */

#include "elf_writer_test_common.h"

void test_metadata_population(void) {
    TEST_SECTION("Metadata Population Tests");
    
    ELFWriter *test_writer = get_test_writer();
    if (!test_writer) return;
    
    // Ensure sections exist
    elf_create_standard_sections(test_writer);
    elf_create_asthra_sections(test_writer);
    
    // Test text section population
    bool success = elf_populate_text_section(test_writer);
    TEST_ASSERT(success, "Text section population");
    
    // Test FFI metadata population
    success = elf_populate_ffi_section(test_writer);
    TEST_ASSERT(success, "FFI metadata population");
    
    // Test GC metadata population
    success = elf_populate_gc_section(test_writer);
    TEST_ASSERT(success, "GC metadata population");
    
    // Test security metadata population
    success = elf_populate_security_section(test_writer);
    TEST_ASSERT(success, "Security metadata population");
    
    // Test pattern matching metadata population
    success = elf_populate_pattern_matching_section(test_writer);
    TEST_ASSERT(success, "Pattern matching metadata population");
    
    // Test string operations metadata population
    success = elf_populate_string_ops_section(test_writer);
    TEST_ASSERT(success, "String operations metadata population");
    
    // Test slice metadata population
    success = elf_populate_slice_meta_section(test_writer);
    TEST_ASSERT(success, "Slice metadata population");
    
    // Test concurrency metadata population
    success = elf_populate_concurrency_section(test_writer);
    TEST_ASSERT(success, "Concurrency metadata population");
    
    // Verify metadata was actually populated
    if (test_writer->asthra_metadata) {
        TEST_ASSERT(test_writer->asthra_metadata->ffi_metadata != NULL, "FFI metadata structure created");
        TEST_ASSERT(test_writer->asthra_metadata->gc_metadata != NULL, "GC metadata structure created");
        TEST_ASSERT(test_writer->asthra_metadata->security_metadata != NULL, "Security metadata structure created");
        TEST_ASSERT(test_writer->asthra_metadata->pattern_metadata != NULL, "Pattern metadata structure created");
        TEST_ASSERT(test_writer->asthra_metadata->string_metadata != NULL, "String metadata structure created");
        TEST_ASSERT(test_writer->asthra_metadata->slice_metadata != NULL, "Slice metadata structure created");
        TEST_ASSERT(test_writer->asthra_metadata->concurrency_metadata != NULL, "Concurrency metadata structure created");
    }
} 
