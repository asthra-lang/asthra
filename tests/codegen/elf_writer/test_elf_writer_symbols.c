/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Symbol Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for ELF symbol management and Asthra-specific symbol types.
 */

#include "elf_writer_test_common.h"

void test_symbol_management(void) {
    TEST_SECTION("Symbol Management Tests");

    ELFWriter *test_writer = get_test_writer();
    if (!test_writer)
        return;

    // Test basic symbol addition
    ELFSymbol *symbol =
        elf_add_symbol(test_writer, "test_symbol", 0x1000, 64, STT_FUNC, STB_GLOBAL, 1);
    TEST_ASSERT(symbol != NULL, "Basic symbol addition");

    if (symbol) {
        TEST_ASSERT(strcmp(symbol->name, "test_symbol") == 0, "Symbol name correct");
        TEST_ASSERT(symbol->value == 0x1000, "Symbol value correct");
        TEST_ASSERT(symbol->size == 64, "Symbol size correct");
        TEST_ASSERT(ELF64_ST_TYPE(symbol->info) == STT_FUNC, "Symbol type correct");
        TEST_ASSERT(ELF64_ST_BIND(symbol->info) == STB_GLOBAL, "Symbol binding correct");
        TEST_ASSERT(symbol->section_index == 1, "Symbol section index correct");
    }

    // Test FFI symbol addition
    ELFSymbol *ffi_symbol = elf_add_ffi_symbol(test_writer, "external_func", 0x2000, NULL);
    TEST_ASSERT(ffi_symbol != NULL, "FFI symbol addition");

    if (ffi_symbol) {
        TEST_ASSERT(ffi_symbol->is_ffi_function == true, "FFI symbol flag set");
        TEST_ASSERT(strcmp(ffi_symbol->name, "external_func") == 0, "FFI symbol name correct");
    }

    // Test pattern matching symbol addition
    ELFSymbol *pattern_symbol =
        elf_add_pattern_match_symbol(test_writer, "match_label", 0x3000, NULL);
    TEST_ASSERT(pattern_symbol != NULL, "Pattern matching symbol addition");

    if (pattern_symbol) {
        TEST_ASSERT(pattern_symbol->is_pattern_match == true, "Pattern match symbol flag set");
    }

    // Test string operation symbol addition
    ELFSymbol *string_symbol = elf_add_string_op_symbol(test_writer, "string_concat", 0x4000, NULL);
    TEST_ASSERT(string_symbol != NULL, "String operation symbol addition");

    if (string_symbol) {
        TEST_ASSERT(string_symbol->is_string_operation == true, "String operation symbol flag set");
    }

    // Test slice operation symbol addition
    ELFSymbol *slice_symbol = elf_add_slice_op_symbol(test_writer, "slice_access", 0x5000, NULL);
    TEST_ASSERT(slice_symbol != NULL, "Slice operation symbol addition");

    if (slice_symbol) {
        TEST_ASSERT(slice_symbol->is_slice_operation == true, "Slice operation symbol flag set");
    }

    // Test spawn symbol addition
    ELFSymbol *spawn_symbol = elf_add_spawn_symbol(test_writer, "spawn_task", 0x6000, NULL);
    TEST_ASSERT(spawn_symbol != NULL, "Spawn symbol addition");

    if (spawn_symbol) {
        TEST_ASSERT(spawn_symbol->is_spawn_point == true, "Spawn symbol flag set");
    }

    // Test GC root symbol addition
    ELFSymbol *gc_symbol = elf_add_gc_root_symbol(test_writer, "gc_data", 0x7000, 128, 0);
    TEST_ASSERT(gc_symbol != NULL, "GC root symbol addition");

    if (gc_symbol) {
        TEST_ASSERT(gc_symbol->is_gc_root == true, "GC root symbol flag set");
        TEST_ASSERT(gc_symbol->size == 128, "GC root symbol size correct");
    }
}
