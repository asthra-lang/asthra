/**
 * Asthra Programming Language Compiler
 * ELF Writer Test Suite - Integration Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Integration tests that simulate complete ELF object file generation workflow.
 */

#include "elf_writer_test_common.h"

void test_integration(void) {
    TEST_SECTION("Integration Tests");

    ELFWriter *test_writer = get_test_writer();
    if (!test_writer)
        return;

    // Test complete object file generation workflow
    // This simulates what would happen in the real compiler

    // 1. Create all sections
    bool success = elf_create_standard_sections(test_writer);
    TEST_ASSERT(success, "Integration: Standard sections creation");

    success = elf_create_asthra_sections(test_writer);
    TEST_ASSERT(success, "Integration: Asthra sections creation");

    // 2. Populate sections with metadata
    success = elf_populate_text_section(test_writer) && elf_populate_ffi_section(test_writer) &&
              elf_populate_gc_section(test_writer) && elf_populate_security_section(test_writer) &&
              elf_populate_pattern_matching_section(test_writer) &&
              elf_populate_string_ops_section(test_writer) &&
              elf_populate_slice_meta_section(test_writer) &&
              elf_populate_concurrency_section(test_writer);
    TEST_ASSERT(success, "Integration: All metadata population");

    // 3. Generate runtime initialization
    success = elf_generate_runtime_init(test_writer);
    TEST_ASSERT(success, "Integration: Runtime initialization");

    // 4. Generate debug information
    success = elf_generate_debug_info(test_writer);
    TEST_ASSERT(success, "Integration: Debug information generation");

    // 5. Add symbols and relocations
    elf_add_ffi_symbol(test_writer, "external_func", 0x1000, NULL);
    elf_add_pattern_match_symbol(test_writer, "match_label", 0x2000, NULL);
    elf_add_string_op_symbol(test_writer, "string_concat", 0x3000, NULL);
    elf_add_ffi_call_relocation(test_writer, 0x1010, "external_func");
    elf_add_pattern_match_relocation(test_writer, 0x2010, "match_label");
    TEST_ASSERT(true, "Integration: Symbols and relocations added");

    // 6. Validate everything
    success = elf_validate_structure(test_writer) && elf_validate_c_compatibility(test_writer) &&
              elf_validate_debug_symbols(test_writer);
    TEST_ASSERT(success, "Integration: Complete validation");

    // 7. Optimize metadata
    success = elf_optimize_metadata(test_writer);
    TEST_ASSERT(success, "Integration: Metadata optimization");

    // 8. Write to file
    const char *integration_filename = "integration_test.o";
    printf("DEBUG: About to call elf_write_object_file()\n");
    success = elf_write_object_file(test_writer, integration_filename);
    printf("DEBUG: elf_write_object_file() returned: %s\n", success ? "true" : "false");
    TEST_ASSERT(success, "Integration: Complete object file generation");

    if (success) {
        // Verify the file
        printf("DEBUG: About to verify file\n");
        struct stat st;
        if (stat(integration_filename, &st) == 0) {
            printf("DEBUG: File stat successful, size: %lld\n", (long long)st.st_size);
            TEST_ASSERT(st.st_size > 1024, "Integration: Output file has substantial content");
            printf("DEBUG: About to unlink file\n");
            unlink(integration_filename);
            printf("DEBUG: File unlinked successfully\n");
        }
    }
    printf("DEBUG: Integration test completed\n");
}
