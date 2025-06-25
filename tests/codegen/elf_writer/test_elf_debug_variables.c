/**
 * Asthra Programming Language Compiler
 * ELF Debug Variables Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test ELF variable debug information generation
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "ast.h"
#include "codegen_test_stubs.h"
#include "elf_compat.h"
#include "elf_relocation_manager.h"
#include "elf_symbol_manager.h"
#include "elf_writer.h"
#include "elf_writer_core.h"
#include "elf_writer_test_common.h"
#include "ffi_assembly_generator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// VARIABLE DEBUG TESTS
// =============================================================================

/**
 * Test variable debug information
 */
AsthraTestResult test_variable_debug_info(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture("test_var_debug.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create variable debug information
    ELFVariableDebugInfo var_info[3];
    var_info[0] = (ELFVariableDebugInfo){.name = "x",
                                         .type = "int",
                                         .location = VAR_LOCATION_REGISTER,
                                         .reg = 0,
                                         .scope_start = 0x1000,
                                         .scope_end = 0x1020};
    var_info[1] = (ELFVariableDebugInfo){.name = "y",
                                         .type = "float",
                                         .location = VAR_LOCATION_STACK,
                                         .stack_offset = -8,
                                         .scope_start = 0x1008,
                                         .scope_end = 0x1030};
    var_info[2] = (ELFVariableDebugInfo){.name = "ptr",
                                         .type = "*int",
                                         .location = VAR_LOCATION_MEMORY,
                                         .memory_address = 0x2000,
                                         .scope_start = 0x1010,
                                         .scope_end = 0x1040};

    for (int i = 0; i < 3; i++) {
        bool result = elf_writer_add_variable_debug_info(fixture->elf_writer, &var_info[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add variable debug info %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Generate variable debug information
    bool result = elf_writer_generate_variable_debug_info(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Failed to generate variable debug info")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that debug info was generated
    size_t debug_info_size = elf_writer_get_debug_info_size(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, debug_info_size > 0,
                                 "Debug info should have non-zero size")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test variable scope tracking
 */
AsthraTestResult test_variable_scope_tracking(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture("test_var_scope.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create variables with overlapping scopes
    ELFVariableDebugInfo var_info[] = {{.name = "global_var",
                                        .type = "int",
                                        .location = VAR_LOCATION_MEMORY,
                                        .memory_address = 0x3000,
                                        .scope_start = 0x1000,
                                        .scope_end = 0x2000},
                                       {.name = "local_var1",
                                        .type = "int",
                                        .location = VAR_LOCATION_STACK,
                                        .stack_offset = -4,
                                        .scope_start = 0x1100,
                                        .scope_end = 0x1200},
                                       {.name = "local_var2",
                                        .type = "float",
                                        .location = VAR_LOCATION_STACK,
                                        .stack_offset = -8,
                                        .scope_start = 0x1150,
                                        .scope_end = 0x1250},
                                       {.name = "temp_var",
                                        .type = "int",
                                        .location = VAR_LOCATION_REGISTER,
                                        .reg = 1,
                                        .scope_start = 0x1180,
                                        .scope_end = 0x1190}};

    for (int i = 0; i < 4; i++) {
        bool result = elf_writer_add_variable_debug_info(fixture->elf_writer, &var_info[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add variable debug info %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test variable lookup at different addresses
    ELFVariableDebugInfo *found_vars = NULL;
    size_t var_count = 0;

    // At address 0x1180, should find global_var, local_var1, local_var2, and temp_var
    bool result = elf_writer_lookup_variables_at_address(fixture->elf_writer, 0x1180, &found_vars,
                                                         &var_count);
    if (!asthra_test_assert_bool(context, result, "Failed to lookup variables at address 0x1180")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, var_count == 4,
                                 "Expected 4 variables at address 0x1180, got %zu", var_count)) {
        free(found_vars);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    free(found_vars);

    // At address 0x1050, should only find global_var
    result = elf_writer_lookup_variables_at_address(fixture->elf_writer, 0x1050, &found_vars,
                                                    &var_count);
    if (!asthra_test_assert_bool(context, result, "Failed to lookup variables at address 0x1050")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(context, var_count == 1,
                                 "Expected 1 variable at address 0x1050, got %zu", var_count)) {
        free(found_vars);
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    free(found_vars);

    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test variable location encoding
 */
AsthraTestResult test_variable_location_encoding(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture("test_var_location.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test different variable location types
    ELFVariableDebugInfo var_info[] = {// Register variable
                                       {.name = "reg_var",
                                        .type = "int",
                                        .location = VAR_LOCATION_REGISTER,
                                        .reg = 5,
                                        .scope_start = 0x1000,
                                        .scope_end = 0x1020},
                                       // Stack variable with positive offset
                                       {.name = "stack_pos",
                                        .type = "int",
                                        .location = VAR_LOCATION_STACK,
                                        .stack_offset = 8,
                                        .scope_start = 0x1000,
                                        .scope_end = 0x1020},
                                       // Stack variable with negative offset
                                       {.name = "stack_neg",
                                        .type = "int",
                                        .location = VAR_LOCATION_STACK,
                                        .stack_offset = -16,
                                        .scope_start = 0x1000,
                                        .scope_end = 0x1020},
                                       // Memory variable
                                       {.name = "mem_var",
                                        .type = "int",
                                        .location = VAR_LOCATION_MEMORY,
                                        .memory_address = 0x4000,
                                        .scope_start = 0x1000,
                                        .scope_end = 0x1020},
                                       // Complex location (register + offset)
                                       {.name = "complex_var",
                                        .type = "int",
                                        .location = VAR_LOCATION_COMPLEX,
                                        .reg = 6,
                                        .stack_offset = 4,
                                        .scope_start = 0x1000,
                                        .scope_end = 0x1020}};

    for (int i = 0; i < 5; i++) {
        bool result = elf_writer_add_variable_debug_info(fixture->elf_writer, &var_info[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add variable debug info %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Generate and validate location expressions
    bool result = elf_writer_generate_variable_locations(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Failed to generate variable locations")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Verify location encodings
    result = elf_writer_validate_variable_locations(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Variable location validation failed")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_elf_writer_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test variable lifetime tracking
 */
AsthraTestResult test_variable_lifetime_tracking(AsthraTestContext *context) {
    ELFWriterTestFixture *fixture = setup_elf_writer_fixture("test_var_lifetime.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create variable with changing locations during its lifetime
    ELFVariableDebugInfo var_info[] = {// Variable starts in register
                                       {.name = "var_x",
                                        .type = "int",
                                        .location = VAR_LOCATION_REGISTER,
                                        .reg = 0,
                                        .scope_start = 0x1000,
                                        .scope_end = 0x1010},
                                       // Same variable moves to stack
                                       {.name = "var_x",
                                        .type = "int",
                                        .location = VAR_LOCATION_STACK,
                                        .stack_offset = -4,
                                        .scope_start = 0x1010,
                                        .scope_end = 0x1020},
                                       // Same variable moves to different register
                                       {.name = "var_x",
                                        .type = "int",
                                        .location = VAR_LOCATION_REGISTER,
                                        .reg = 2,
                                        .scope_start = 0x1020,
                                        .scope_end = 0x1030}};

    for (int i = 0; i < 3; i++) {
        bool result = elf_writer_add_variable_debug_info(fixture->elf_writer, &var_info[i]);
        if (!asthra_test_assert_bool(context, result, "Failed to add variable debug info %d", i)) {
            cleanup_elf_writer_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Generate variable lifetime information
    bool result = elf_writer_generate_variable_lifetimes(fixture->elf_writer);
    if (!asthra_test_assert_bool(context, result, "Failed to generate variable lifetimes")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Test variable lookup at different points in its lifetime
    ELFVariableDebugInfo *found_var = NULL;

    // At 0x1005, should be in register 0
    result = elf_writer_lookup_variable_by_name_and_address(fixture->elf_writer, "var_x", 0x1005,
                                                            &found_var);
    if (!asthra_test_assert_bool(context, result, "Failed to lookup var_x at 0x1005")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(
            context, found_var->location == VAR_LOCATION_REGISTER && found_var->reg == 0,
            "Expected var_x in register 0 at 0x1005")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // At 0x1015, should be on stack
    result = elf_writer_lookup_variable_by_name_and_address(fixture->elf_writer, "var_x", 0x1015,
                                                            &found_var);
    if (!asthra_test_assert_bool(context, result, "Failed to lookup var_x at 0x1015")) {
        cleanup_elf_writer_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    if (!asthra_test_assert_bool(
            context, found_var->location == VAR_LOCATION_STACK && found_var->stack_offset == -4,
            "Expected var_x on stack at offset -4 at 0x1015")) {
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
 * Main test function for ELF debug variables
 */
int main(void) {
    AsthraTestStatistics *stats = asthra_test_statistics_create();

    AsthraTestSuiteConfig config = {
        .name = "ELF Debug Variables Tests",
        .description = "Test ELF variable debug information generation and tracking",
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 15000000000ULL, // 15 seconds
        .max_parallel_tests = 1,
        .statistics = stats};

    AsthraTestFunction tests[] = {test_variable_debug_info, test_variable_scope_tracking,
                                  test_variable_location_encoding, test_variable_lifetime_tracking};

    AsthraTestMetadata metadata[] = {
        {"test_variable_debug_info", __FILE__, __LINE__, "test_variable_debug_info",
         ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_variable_scope_tracking", __FILE__, __LINE__, "test_variable_scope_tracking",
         ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_variable_location_encoding", __FILE__, __LINE__, "test_variable_location_encoding",
         ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
        {"test_variable_lifetime_tracking", __FILE__, __LINE__, "test_variable_lifetime_tracking",
         ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    AsthraTestResult result = asthra_test_run_suite(tests, metadata, test_count, &config);

    asthra_test_statistics_print(stats, false);
    asthra_test_statistics_destroy(stats);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
