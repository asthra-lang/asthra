/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Lifecycle Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for ELF writer lifecycle management (creation, initialization, destruction).
 */

#include "elf_writer_test_common.h"

void test_elf_writer_lifecycle(void) {
    TEST_SECTION("ELF Writer Lifecycle Tests");
    
    // Test creation
    FFIAssemblyGenerator *ffi_gen = ffi_assembly_generator_create(TARGET_ARCH_X86_64, 
                                                                 CALLING_CONV_SYSTEM_V_AMD64);
    TEST_ASSERT(ffi_gen != NULL, "FFI assembly generator creation");
    
    ELFWriter *writer = elf_writer_create(ffi_gen);
    TEST_ASSERT(writer != NULL, "ELF writer creation");
    
    if (writer) {
        // Test initial state
        TEST_ASSERT(writer->section_count == 0, "Initial section count is zero");
        TEST_ASSERT(writer->symbol_count == 0, "Initial symbol count is zero");
        TEST_ASSERT(writer->relocation_count == 0, "Initial relocation count is zero");
        TEST_ASSERT(writer->ffi_generator == ffi_gen, "FFI generator reference is correct");
        
        // Test configuration defaults
        TEST_ASSERT(writer->config.generate_debug_info == true, "Debug info generation enabled by default");
        TEST_ASSERT(writer->config.optimize_metadata == true, "Metadata optimization enabled by default");
        TEST_ASSERT(writer->config.validate_elf_structure == true, "ELF validation enabled by default");
        TEST_ASSERT(writer->config.enable_all_asthra_sections == true, "All Asthra sections enabled by default");
        
        // Test destruction
        elf_writer_destroy(writer);
        TEST_ASSERT(true, "ELF writer destruction");
    }
    
    if (ffi_gen) {
        ffi_assembly_generator_destroy(ffi_gen);
    }
    
    // Test null handling
    elf_writer_destroy(NULL);
    TEST_ASSERT(true, "ELF writer destruction with NULL pointer");
} 
