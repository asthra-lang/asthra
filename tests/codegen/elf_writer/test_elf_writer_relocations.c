/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Relocation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for ELF relocation management and Asthra-specific relocations.
 */

#include "elf_writer_test_common.h"

void test_relocations(void) {
    TEST_SECTION("Relocation Tests");
    
    ELFWriter *test_writer = get_test_writer();
    if (!test_writer) return;
    
    // Test basic relocation addition
    ELFRelocation *reloc = elf_add_relocation(test_writer, 0x1000, R_X86_64_PC32, 1, -4);
    TEST_ASSERT(reloc != NULL, "Basic relocation addition");
    
    if (reloc) {
        TEST_ASSERT(reloc->offset == 0x1000, "Relocation offset correct");
        TEST_ASSERT(ELF64_R_TYPE(reloc->info) == R_X86_64_PC32, "Relocation type correct");
        TEST_ASSERT(ELF64_R_SYM(reloc->info) == 1, "Relocation symbol correct");
        TEST_ASSERT(reloc->addend == -4, "Relocation addend correct");
    }
    
    // Test FFI call relocation
    bool success = elf_add_ffi_call_relocation(test_writer, 0x2000, "external_function");
    TEST_ASSERT(success, "FFI call relocation addition");
    
    // Test pattern match relocation
    success = elf_add_pattern_match_relocation(test_writer, 0x3000, "match_target");
    TEST_ASSERT(success, "Pattern match relocation addition");
    
    // Test string operation relocation
    success = elf_add_string_op_relocation(test_writer, 0x4000, "asthra_string_concat");
    TEST_ASSERT(success, "String operation relocation addition");
    
    // Test slice bounds relocation
    success = elf_add_slice_bounds_relocation(test_writer, 0x5000, "asthra_slice_bounds_check");
    TEST_ASSERT(success, "Slice bounds relocation addition");
    
    // Test spawn relocation
    success = elf_add_spawn_relocation(test_writer, 0x6000, "asthra_scheduler_spawn");
    TEST_ASSERT(success, "Spawn relocation addition");
} 
