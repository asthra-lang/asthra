/**
 * Asthra Programming Language Compiler
 * Register Spill Slot Allocation Testing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file implements testing for spill slot allocation including
 * basic allocation, reuse optimization, and type segregation.
 */

#include "spill_handling_common.h"

// =============================================================================
// SPILL SLOT ALLOCATION TESTS
// =============================================================================

/**
 * Test basic spill slot allocation
 */
AsthraTestResult test_spill_slot_allocation(AsthraTestContext *context) {
    SpillHandlingTestFixture *fixture = setup_spill_handling_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Allocate spill slots for several virtual registers
    int slot1 = spill_manager_allocate_slot(fixture->spill_manager, 0, SPILL_TYPE_GENERAL);
    int slot2 = spill_manager_allocate_slot(fixture->spill_manager, 1, SPILL_TYPE_GENERAL);
    int slot3 = spill_manager_allocate_slot(fixture->spill_manager, 2, SPILL_TYPE_GENERAL);

    if (!asthra_test_assert_bool(context, slot1 >= 0 && slot2 >= 0 && slot3 >= 0,
                                 "Spill slot allocation should succeed")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Slots should be different
    if (!asthra_test_assert_bool(context, slot1 != slot2 && slot2 != slot3 && slot1 != slot3,
                                 "Different virtual registers should get different spill slots")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that we can retrieve the slot for a virtual register
    int retrieved_slot1 = spill_manager_get_slot(fixture->spill_manager, 0);
    if (!asthra_test_assert_int(context, retrieved_slot1, slot1,
                                "Should be able to retrieve allocated spill slot")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_spill_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test spill slot reuse for non-overlapping lifetimes
 */
AsthraTestResult test_spill_slot_reuse(AsthraTestContext *context) {
    SpillHandlingTestFixture *fixture = setup_spill_handling_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create non-overlapping live intervals
    LiveInterval interval1 = {
        .virtual_reg = 0, .start = 0, .end = 10, .physical_reg = REG_UNASSIGNED};
    LiveInterval interval2 = {
        .virtual_reg = 1, .start = 15, .end = 25, .physical_reg = REG_UNASSIGNED};

    // Allocate spill slots with lifetime information
    int slot1 = spill_manager_allocate_slot_with_lifetime(fixture->spill_manager, &interval1,
                                                          SPILL_TYPE_GENERAL);
    int slot2 = spill_manager_allocate_slot_with_lifetime(fixture->spill_manager, &interval2,
                                                          SPILL_TYPE_GENERAL);

    if (!asthra_test_assert_bool(context, slot1 >= 0 && slot2 >= 0,
                                 "Spill slot allocation with lifetime should succeed")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Non-overlapping intervals should be able to reuse the same slot
    if (!asthra_test_assert_int(context, slot1, slot2,
                                "Non-overlapping lifetimes should reuse the same spill slot")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_spill_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test spill slot type segregation
 */
AsthraTestResult test_spill_slot_type_segregation(AsthraTestContext *context) {
    SpillHandlingTestFixture *fixture = setup_spill_handling_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Allocate slots for different types
    int general_slot = spill_manager_allocate_slot(fixture->spill_manager, 0, SPILL_TYPE_GENERAL);
    int float_slot = spill_manager_allocate_slot(fixture->spill_manager, 1, SPILL_TYPE_FLOAT);
    int vector_slot = spill_manager_allocate_slot(fixture->spill_manager, 2, SPILL_TYPE_VECTOR);

    if (!asthra_test_assert_bool(context, general_slot >= 0 && float_slot >= 0 && vector_slot >= 0,
                                 "Spill slot allocation for different types should succeed")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that different types get appropriate alignment/sizing
    size_t general_offset = spill_manager_get_slot_offset(fixture->spill_manager, general_slot);
    size_t float_offset = spill_manager_get_slot_offset(fixture->spill_manager, float_slot);
    size_t vector_offset = spill_manager_get_slot_offset(fixture->spill_manager, vector_slot);

    // Vector slots should be aligned to larger boundaries
    if (!asthra_test_assert_bool(context, vector_offset % 16 == 0,
                                 "Vector spill slots should be 16-byte aligned")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Float slots should be aligned to 8-byte boundaries
    if (!asthra_test_assert_bool(context, float_offset % 8 == 0,
                                 "Float spill slots should be 8-byte aligned")) {
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
        .name = "Spill Slot Allocation Tests",
        .tests = {{"Spill Slot Allocation", test_spill_slot_allocation},
                  {"Spill Slot Reuse", test_spill_slot_reuse},
                  {"Spill Slot Type Segregation", test_spill_slot_type_segregation},
                  {NULL, NULL}}};

    return asthra_test_run_suite(&suite);
}
