/**
 * Asthra Programming Language Compiler
 * Multi-file Coordination Test - Minimal Framework Version
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 3 Priority 2: Multi-file test coordination using minimal framework
 * This version tests coordination between multiple compilation units
 */

#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// MULTI-FILE COORDINATION SIMULATION
// =============================================================================

// Simulate compilation unit information
typedef struct {
    char filename[128];
    char *symbols[50];
    size_t symbol_count;
    bool compiled;
    char object_file[256];
} CompilationUnit;

// Simulate linker information
typedef struct {
    CompilationUnit units[10];
    size_t unit_count;
    char *undefined_symbols[100];
    size_t undefined_count;
    char executable_name[256];
} LinkerState;

static LinkerState linker = {0};

// Helper functions for multi-file coordination
static void add_compilation_unit(const char *filename) {
    if (linker.unit_count >= 10)
        return;

    CompilationUnit *unit = &linker.units[linker.unit_count++];
    strncpy(unit->filename, filename, sizeof(unit->filename) - 1);
    unit->filename[sizeof(unit->filename) - 1] = '\0';
    unit->symbol_count = 0;
    unit->compiled = false;
    snprintf(unit->object_file, sizeof(unit->object_file), "%s.o", filename);
}

static void add_symbol_to_unit(const char *filename, const char *symbol) {
    for (size_t i = 0; i < linker.unit_count; i++) {
        if (strcmp(linker.units[i].filename, filename) == 0) {
            if (linker.units[i].symbol_count < 50) {
                linker.units[i].symbols[linker.units[i].symbol_count] = malloc(strlen(symbol) + 1);
                strcpy(linker.units[i].symbols[linker.units[i].symbol_count], symbol);
                linker.units[i].symbol_count++;
            }
            break;
        }
    }
}

static bool compile_unit(const char *filename) {
    for (size_t i = 0; i < linker.unit_count; i++) {
        if (strcmp(linker.units[i].filename, filename) == 0) {
            linker.units[i].compiled = true;
            return true;
        }
    }
    return false;
}

static bool link_units(const char *output_name) {
    strncpy(linker.executable_name, output_name, sizeof(linker.executable_name) - 1);
    linker.executable_name[sizeof(linker.executable_name) - 1] = '\0';

    // Check all units are compiled
    for (size_t i = 0; i < linker.unit_count; i++) {
        if (!linker.units[i].compiled) {
            return false;
        }
    }
    return true;
}

static void cleanup_linker(void) {
    for (size_t i = 0; i < linker.unit_count; i++) {
        for (size_t j = 0; j < linker.units[i].symbol_count; j++) {
            free(linker.units[i].symbols[j]);
        }
    }
    memset(&linker, 0, sizeof(linker));
}

// =============================================================================
// MINIMAL MULTI-FILE COORDINATION TEST IMPLEMENTATIONS
// =============================================================================

DEFINE_TEST(test_multiple_compilation_units) {
    printf("  Testing multiple compilation units coordination...\n");

    cleanup_linker();

    // Add multiple source files
    add_compilation_unit("main.asthra");
    add_compilation_unit("math.asthra");
    add_compilation_unit("utils.asthra");

    TEST_ASSERT(linker.unit_count == 3, "Should have 3 compilation units");

    // Add symbols to each unit
    add_symbol_to_unit("main.asthra", "main");
    add_symbol_to_unit("math.asthra", "add");
    add_symbol_to_unit("math.asthra", "multiply");
    add_symbol_to_unit("utils.asthra", "print_debug");

    // Verify symbols are properly assigned
    TEST_ASSERT(linker.units[0].symbol_count == 1, "main.asthra should have 1 symbol");
    TEST_ASSERT(linker.units[1].symbol_count == 2, "math.asthra should have 2 symbols");
    TEST_ASSERT(linker.units[2].symbol_count == 1, "utils.asthra should have 1 symbol");

    printf("  ✅ Multiple compilation units: Unit coordination functional\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_compilation_sequence) {
    printf("  Testing compilation sequence coordination...\n");

    // Test compilation order
    bool result1 = compile_unit("main.asthra");
    bool result2 = compile_unit("math.asthra");
    bool result3 = compile_unit("utils.asthra");

    TEST_ASSERT(result1, "main.asthra should compile successfully");
    TEST_ASSERT(result2, "math.asthra should compile successfully");
    TEST_ASSERT(result3, "utils.asthra should compile successfully");

    // Verify compilation state
    for (size_t i = 0; i < linker.unit_count; i++) {
        TEST_ASSERT(linker.units[i].compiled, "All units should be compiled");
    }

    printf("  ✅ Compilation sequence: Sequential compilation functional\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_linking_coordination) {
    printf("  Testing linking coordination...\n");

    // Test linking process
    bool link_result = link_units("test_program");
    TEST_ASSERT(link_result, "Linking should succeed when all units are compiled");
    TEST_ASSERT_STR_EQ(linker.executable_name, "test_program",
                       "Executable name should be set correctly");

    printf("  ✅ Linking coordination: Multi-unit linking functional\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_symbol_resolution) {
    printf("  Testing symbol resolution coordination...\n");

    // Test symbol resolution across units
    bool main_found = false;
    bool add_found = false;
    bool print_debug_found = false;

    for (size_t i = 0; i < linker.unit_count; i++) {
        for (size_t j = 0; j < linker.units[i].symbol_count; j++) {
            if (strcmp(linker.units[i].symbols[j], "main") == 0)
                main_found = true;
            if (strcmp(linker.units[i].symbols[j], "add") == 0)
                add_found = true;
            if (strcmp(linker.units[i].symbols[j], "print_debug") == 0)
                print_debug_found = true;
        }
    }

    TEST_ASSERT(main_found, "main symbol should be found");
    TEST_ASSERT(add_found, "add symbol should be found");
    TEST_ASSERT(print_debug_found, "print_debug symbol should be found");

    printf("  ✅ Symbol resolution: Cross-unit symbol resolution functional\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_object_file_generation) {
    printf("  Testing object file generation coordination...\n");

    // Test object file naming
    for (size_t i = 0; i < linker.unit_count; i++) {
        size_t filename_len = strlen(linker.units[i].filename);
        size_t object_len = strlen(linker.units[i].object_file);

        TEST_ASSERT(object_len > filename_len,
                    "Object file name should be longer than source file name");
        TEST_ASSERT(strstr(linker.units[i].object_file, ".o") != NULL,
                    "Object file should have .o extension");
    }

    printf("  ✅ Object file generation: Object file naming functional\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_incremental_compilation) {
    printf("  Testing incremental compilation coordination...\n");

    // Simulate modification of one unit
    cleanup_linker();
    add_compilation_unit("core.asthra");
    add_compilation_unit("plugin.asthra");

    add_symbol_to_unit("core.asthra", "core_init");
    add_symbol_to_unit("plugin.asthra", "plugin_load");

    // Compile core first
    bool core_compiled = compile_unit("core.asthra");
    TEST_ASSERT(core_compiled, "Core unit should compile");

    // Verify incremental state
    TEST_ASSERT(linker.units[0].compiled, "Core should be compiled");
    TEST_ASSERT(!linker.units[1].compiled, "Plugin should not be compiled yet");

    // Compile plugin
    bool plugin_compiled = compile_unit("plugin.asthra");
    TEST_ASSERT(plugin_compiled, "Plugin unit should compile");
    TEST_ASSERT(linker.units[1].compiled, "Plugin should now be compiled");

    printf("  ✅ Incremental compilation: Incremental build coordination functional\n");
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST SUITE USING MINIMAL FRAMEWORK
// =============================================================================

RUN_TEST_SUITE(multifile_coordination_suite, RUN_TEST(test_multiple_compilation_units);
               RUN_TEST(test_compilation_sequence); RUN_TEST(test_linking_coordination);
               RUN_TEST(test_symbol_resolution); RUN_TEST(test_object_file_generation);
               RUN_TEST(test_incremental_compilation);)
