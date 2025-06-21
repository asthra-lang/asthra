/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Runtime Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for runtime initialization and setup.
 */

#include "elf_writer_test_common.h"

void test_runtime_initialization(void) {
    TEST_SECTION("Runtime Initialization Tests");
    
    ELFWriter *test_writer = get_test_writer();
    if (!test_writer) return;
    
    // Ensure sections exist
    elf_create_standard_sections(test_writer);
    
    // Test runtime initialization generation
    bool success = elf_generate_runtime_init(test_writer);
    TEST_ASSERT(success, "Runtime initialization generation");
    
    // Test runtime metadata addition
    success = elf_add_runtime_init_metadata(test_writer);
    TEST_ASSERT(success, "Runtime initialization metadata addition");
    
    // Verify _Asthra_init symbol was created
    bool has_init_symbol = false;
    for (size_t i = 0; i < test_writer->symbol_count; i++) {
        if (strcmp(test_writer->symbols[i]->name, "_Asthra_init") == 0) {
            has_init_symbol = true;
            break;
        }
    }
    TEST_ASSERT(has_init_symbol, "_Asthra_init symbol created");
} 
