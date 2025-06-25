/**
 * Asthra Programming Language Compiler
 * Register Spill Handling Integration Testing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file implements integration testing for complete spill handling
 * workflows including end-to-end register allocation with spilling.
 */

#include "spill_handling_common.h"

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

/**
 * Test complete spill handling workflow
 */
AsthraTestResult test_complete_spill_workflow(AsthraTestContext *context) {
    SpillHandlingTestFixture *fixture = setup_spill_handling_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create more intervals than available registers to force spilling
    const int num_intervals = 8; // More than 4 physical registers
    LiveInterval intervals[8];

    for (int i = 0; i < num_intervals; i++) {
        intervals[i] = (LiveInterval){.virtual_reg = i,
                                      .start = i * 2,
                                      .end = i * 2 + 20, // All overlap significantly
                                      .physical_reg = REG_UNASSIGNED,
                                      .use_count = 5 + (i % 3),
                                      .def_count = 1};
    }

    // Perform register allocation with spilling
    RegisterAllocationResult *result = register_allocator_allocate_with_spilling(
        fixture->allocator, fixture->spill_manager, intervals, num_intervals);

    if (!asthra_test_assert_pointer(context, result,
                                    "Register allocation with spilling should succeed")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Should have assigned some registers and spilled others
    if (!asthra_test_assert_bool(context, result->num_assigned > 0 && result->num_spilled > 0,
                                 "Should have both assigned and spilled registers")) {
        register_allocation_result_destroy(result);
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Total should equal input
    if (!asthra_test_assert_int(context, result->num_assigned + result->num_spilled, num_intervals,
                                "Total assigned + spilled should equal input intervals")) {
        register_allocation_result_destroy(result);
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Should not exceed available physical registers
    if (!asthra_test_assert_bool(context,
                                 result->num_assigned <= (int)fixture->num_physical_registers,
                                 "Should not assign more than available physical registers")) {
        register_allocation_result_destroy(result);
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    register_allocation_result_destroy(result);
    cleanup_spill_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite suite = {
        .name = "Spill Handling Integration Tests",
        .tests = {{"Complete Spill Workflow", test_complete_spill_workflow}, {NULL, NULL}}};

    return asthra_test_run_suite(&suite);
}
