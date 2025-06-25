/**
 * Asthra Programming Language Compiler
 * Register Spill Code Generation Testing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file implements testing for spill code generation including
 * spill code generation, reload code generation, and optimization.
 */

#include "spill_handling_common.h"

// =============================================================================
// SPILL CODE GENERATION TESTS
// =============================================================================

/**
 * Test basic spill code generation
 */
AsthraTestResult test_spill_code_generation(AsthraTestContext *context) {
    SpillHandlingTestFixture *fixture = setup_spill_handling_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Allocate a spill slot
    int spill_slot = spill_manager_allocate_slot(fixture->spill_manager, 0, SPILL_TYPE_GENERAL);
    if (!asthra_test_assert_bool(context, spill_slot >= 0,
                                 "Spill slot allocation should succeed")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Generate spill code (store virtual register to memory)
    size_t initial_instruction_count = instruction_buffer_get_count(fixture->instruction_buffer);
    bool spill_result =
        spill_manager_generate_spill_code(fixture->spill_manager, fixture->instruction_buffer, 0,
                                          spill_slot, 1); // vreg 0, slot, at position 1

    if (!asthra_test_assert_bool(context, spill_result, "Spill code generation should succeed")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Should have added at least one instruction
    size_t final_instruction_count = instruction_buffer_get_count(fixture->instruction_buffer);
    if (!asthra_test_assert_bool(context, final_instruction_count > initial_instruction_count,
                                 "Spill code generation should add instructions")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_spill_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test reload code generation
 */
AsthraTestResult test_reload_code_generation(AsthraTestContext *context) {
    SpillHandlingTestFixture *fixture = setup_spill_handling_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Allocate a spill slot
    int spill_slot = spill_manager_allocate_slot(fixture->spill_manager, 0, SPILL_TYPE_GENERAL);
    if (!asthra_test_assert_bool(context, spill_slot >= 0,
                                 "Spill slot allocation should succeed")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Generate reload code (load from memory to physical register)
    size_t initial_instruction_count = instruction_buffer_get_count(fixture->instruction_buffer);
    bool reload_result = spill_manager_generate_reload_code(
        fixture->spill_manager, fixture->instruction_buffer, 0, spill_slot, 2,
        1); // vreg 0, slot, at position 2, to physical reg 1

    if (!asthra_test_assert_bool(context, reload_result, "Reload code generation should succeed")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Should have added at least one instruction
    size_t final_instruction_count = instruction_buffer_get_count(fixture->instruction_buffer);
    if (!asthra_test_assert_bool(context, final_instruction_count > initial_instruction_count,
                                 "Reload code generation should add instructions")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_spill_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test spill/reload pair optimization
 */
AsthraTestResult test_spill_reload_optimization(AsthraTestContext *context) {
    SpillHandlingTestFixture *fixture = setup_spill_handling_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a scenario with adjacent spill and reload
    int spill_slot = spill_manager_allocate_slot(fixture->spill_manager, 0, SPILL_TYPE_GENERAL);

    // Generate spill followed immediately by reload
    spill_manager_generate_spill_code(fixture->spill_manager, fixture->instruction_buffer, 0,
                                      spill_slot, 1);
    spill_manager_generate_reload_code(fixture->spill_manager, fixture->instruction_buffer, 0,
                                       spill_slot, 2, 1);

    size_t instruction_count_before_opt = instruction_buffer_get_count(fixture->instruction_buffer);

    // Run spill/reload optimization
    bool opt_result = spill_manager_optimize_spill_reload_pairs(fixture->spill_manager,
                                                                fixture->instruction_buffer);

    if (!asthra_test_assert_bool(context, opt_result, "Spill/reload optimization should succeed")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    size_t instruction_count_after_opt = instruction_buffer_get_count(fixture->instruction_buffer);

    // Should have eliminated redundant spill/reload pairs
    if (!asthra_test_assert_bool(context,
                                 instruction_count_after_opt <= instruction_count_before_opt,
                                 "Optimization should not increase instruction count")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_spill_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite suite = {
        .name = "Spill Code Generation Tests",
        .tests = {{"Spill Code Generation", test_spill_code_generation},
                  {"Reload Code Generation", test_reload_code_generation},
                  {"Spill/Reload Optimization", test_spill_reload_optimization},
                  {NULL, NULL}}};

    return asthra_test_run_suite(&suite);
}
