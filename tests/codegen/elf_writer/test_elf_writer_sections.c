/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Section Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for ELF section creation and management.
 */

#include "elf_writer_test_common.h"

void test_section_creation(void) {
    TEST_SECTION("Section Creation Tests");
    
    ELFWriter *test_writer = get_test_writer();
    if (!test_writer) return;
    
    // Test standard section creation
    bool success = elf_create_standard_sections(test_writer);
    TEST_ASSERT(success, "Standard sections creation");
    
    if (success) {
        TEST_ASSERT(test_writer->section_count >= 8, "Minimum standard sections created");
        
        // Check for required sections
        bool has_text = false, has_data = false, has_bss = false;
        bool has_symtab = false, has_strtab = false, has_shstrtab = false;
        
        for (size_t i = 0; i < test_writer->section_count; i++) {
            ELFSection *section = test_writer->sections[i];
            if (strcmp(section->name, ".text") == 0) has_text = true;
            if (strcmp(section->name, ".data") == 0) has_data = true;
            if (strcmp(section->name, ".bss") == 0) has_bss = true;
            if (strcmp(section->name, ".symtab") == 0) has_symtab = true;
            if (strcmp(section->name, ".strtab") == 0) has_strtab = true;
            if (strcmp(section->name, ".shstrtab") == 0) has_shstrtab = true;
        }
        
        TEST_ASSERT(has_text, ".text section created");
        TEST_ASSERT(has_data, ".data section created");
        TEST_ASSERT(has_bss, ".bss section created");
        TEST_ASSERT(has_symtab, ".symtab section created");
        TEST_ASSERT(has_strtab, ".strtab section created");
        TEST_ASSERT(has_shstrtab, ".shstrtab section created");
    }
    
    // Test Asthra section creation
    success = elf_create_asthra_sections(test_writer);
    TEST_ASSERT(success, "Asthra sections creation");
    
    if (success) {
        // Check for Asthra-specific sections
        bool has_ffi = false, has_gc = false, has_security = false;
        bool has_pattern = false, has_string = false, has_slice = false, has_concurrency = false;
        
        for (size_t i = 0; i < test_writer->section_count; i++) {
            ELFSection *section = test_writer->sections[i];
            if (strcmp(section->name, ".Asthra.ffi") == 0) has_ffi = true;
            if (strcmp(section->name, ".Asthra.gc") == 0) has_gc = true;
            if (strcmp(section->name, ".Asthra.security_meta") == 0) has_security = true;
            if (strcmp(section->name, ".Asthra.pattern_matching") == 0) has_pattern = true;
            if (strcmp(section->name, ".Asthra.string_ops") == 0) has_string = true;
            if (strcmp(section->name, ".Asthra.slice_meta") == 0) has_slice = true;
            if (strcmp(section->name, ".Asthra.concurrency") == 0) has_concurrency = true;
        }
        
        TEST_ASSERT(has_ffi, ".Asthra.ffi section created");
        TEST_ASSERT(has_gc, ".Asthra.gc section created");
        TEST_ASSERT(has_security, ".Asthra.security_meta section created");
        TEST_ASSERT(has_pattern, ".Asthra.pattern_matching section created");
        TEST_ASSERT(has_string, ".Asthra.string_ops section created");
        TEST_ASSERT(has_slice, ".Asthra.slice_meta section created");
        TEST_ASSERT(has_concurrency, ".Asthra.concurrency section created");
    }
    
    // Test individual section addition
    ELFSection *custom_section = elf_add_section(test_writer, ".custom", SHT_PROGBITS, SHF_ALLOC);
    TEST_ASSERT(custom_section != NULL, "Custom section addition");
    
    if (custom_section) {
        TEST_ASSERT(strcmp(custom_section->name, ".custom") == 0, "Custom section name correct");
        TEST_ASSERT(custom_section->type == SHT_PROGBITS, "Custom section type correct");
        TEST_ASSERT(custom_section->flags == SHF_ALLOC, "Custom section flags correct");
    }
} 
