/**
 * Asthra Programming Language Compiler
 * Register Spill Cost Analysis Testing
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file implements testing for spill cost analysis including
 * cost calculation, candidate selection, and loop-aware optimization.
 */

#include "spill_handling_common.h"

// =============================================================================
// SPILL COST ANALYSIS TESTS
// =============================================================================

/**
 * Test spill cost calculation
 */
AsthraTestResult test_spill_cost_calculation(AsthraTestContext* context) {
    SpillHandlingTestFixture* fixture = setup_spill_handling_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create live intervals with different characteristics
    LiveInterval high_use_interval = {
        .virtual_reg = 0, .start = 0, .end = 100, .physical_reg = REG_UNASSIGNED,
        .use_count = 50, .def_count = 5
    };
    
    LiveInterval low_use_interval = {
        .virtual_reg = 1, .start = 0, .end = 100, .physical_reg = REG_UNASSIGNED,
        .use_count = 2, .def_count = 1
    };
    
    // Calculate spill costs
    double high_use_cost = spill_manager_calculate_spill_cost(fixture->spill_manager, &high_use_interval);
    double low_use_cost = spill_manager_calculate_spill_cost(fixture->spill_manager, &low_use_interval);
    
    // High-use interval should have higher spill cost
    if (!asthra_test_assert_bool(context, high_use_cost > low_use_cost,
                                "High-use interval should have higher spill cost")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Both costs should be positive
    if (!asthra_test_assert_bool(context, high_use_cost > 0.0 && low_use_cost > 0.0,
                                "Spill costs should be positive")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_spill_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test spill candidate selection
 */
AsthraTestResult test_spill_candidate_selection(AsthraTestContext* context) {
    SpillHandlingTestFixture* fixture = setup_spill_handling_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create multiple live intervals
    LiveInterval intervals[5];
    intervals[0] = (LiveInterval){.virtual_reg = 0, .start = 0, .end = 50, .use_count = 10, .def_count = 2};
    intervals[1] = (LiveInterval){.virtual_reg = 1, .start = 10, .end = 60, .use_count = 2, .def_count = 1};  // Low use - good candidate
    intervals[2] = (LiveInterval){.virtual_reg = 2, .start = 20, .end = 70, .use_count = 25, .def_count = 3};
    intervals[3] = (LiveInterval){.virtual_reg = 3, .start = 30, .end = 80, .use_count = 1, .def_count = 1};  // Very low use - best candidate
    intervals[4] = (LiveInterval){.virtual_reg = 4, .start = 40, .end = 90, .use_count = 15, .def_count = 2};
    
    // Select best spill candidate
    int best_candidate = spill_manager_select_spill_candidate(fixture->spill_manager, intervals, 5);
    
    // Should select the interval with lowest spill cost (virtual register 3)
    if (!asthra_test_assert_int(context, best_candidate, 3,
                               "Should select the interval with lowest spill cost")) {
        cleanup_spill_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    cleanup_spill_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test loop-aware spill cost adjustment
 */
AsthraTestResult test_loop_aware_spill_cost(AsthraTestContext* context) {
    SpillHandlingTestFixture* fixture = setup_spill_handling_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create intervals inside and outside loops
    LiveInterval outside_loop = {
        .virtual_reg = 0, .start = 0, .end = 50, .physical_reg = REG_UNASSIGNED,
        .use_count = 5, .def_count = 1, .loop_depth = 0
    };
    
    LiveInterval inside_loop = {
        .virtual_reg = 1, .start = 0, .end = 50, .physical_reg = REG_UNASSIGNED,
        .use_count = 5, .def_count = 1, .loop_depth = 2  // Nested 2 levels deep
    };
    
    // Calculate spill costs with loop awareness
    double outside_cost = spill_manager_calculate_spill_cost_with_loops(fixture->spill_manager, &outside_loop);
    double inside_cost = spill_manager_calculate_spill_cost_with_loops(fixture->spill_manager, &inside_loop);
    
    // Interval inside loop should have much higher spill cost
    if (!asthra_test_assert_bool(context, inside_cost > outside_cost * 4,  // Loop penalty should be significant
                                "Interval inside loop should have much higher spill cost")) {
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
        .name = "Spill Cost Analysis Tests",
        .tests = {
            { "Spill Cost Calculation", test_spill_cost_calculation },
            { "Spill Candidate Selection", test_spill_candidate_selection },
            { "Loop-Aware Spill Cost", test_loop_aware_spill_cost },
            { NULL, NULL }
        }
    };
    
    return asthra_test_run_suite(&suite);
} 
