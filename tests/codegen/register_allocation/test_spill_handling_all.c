/**
 * Asthra Programming Language Compiler
 * Register Spill Handling - Master Test Runner
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file provides a master test runner that executes all spill handling
 * test suites including slot allocation, code generation, cost analysis,
 * and integration tests.
 */

#include "spill_handling_common.h"

// =============================================================================
// EXTERNAL TEST FUNCTION DECLARATIONS
// =============================================================================

// From test_spill_slot_allocation.c
extern AsthraTestResult test_spill_slot_allocation(AsthraTestContext *context);
extern AsthraTestResult test_spill_slot_reuse(AsthraTestContext *context);
extern AsthraTestResult test_spill_slot_type_segregation(AsthraTestContext *context);

// From test_spill_code_generation.c
extern AsthraTestResult test_spill_code_generation(AsthraTestContext *context);
extern AsthraTestResult test_reload_code_generation(AsthraTestContext *context);
extern AsthraTestResult test_spill_reload_optimization(AsthraTestContext *context);

// From test_spill_cost_analysis.c
extern AsthraTestResult test_spill_cost_calculation(AsthraTestContext *context);
extern AsthraTestResult test_spill_candidate_selection(AsthraTestContext *context);
extern AsthraTestResult test_loop_aware_spill_cost(AsthraTestContext *context);

// From test_spill_integration.c
extern AsthraTestResult test_complete_spill_workflow(AsthraTestContext *context);

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite suite = {
        .name = "Register Spill Handling - All Tests",
        .tests = {// Spill Slot Allocation Tests
                  {"Spill Slot Allocation", test_spill_slot_allocation},
                  {"Spill Slot Reuse", test_spill_slot_reuse},
                  {"Spill Slot Type Segregation", test_spill_slot_type_segregation},

                  // Spill Code Generation Tests
                  {"Spill Code Generation", test_spill_code_generation},
                  {"Reload Code Generation", test_reload_code_generation},
                  {"Spill/Reload Optimization", test_spill_reload_optimization},

                  // Spill Cost Analysis Tests
                  {"Spill Cost Calculation", test_spill_cost_calculation},
                  {"Spill Candidate Selection", test_spill_candidate_selection},
                  {"Loop-Aware Spill Cost", test_loop_aware_spill_cost},

                  // Integration Tests
                  {"Complete Spill Workflow", test_complete_spill_workflow},

                  {NULL, NULL}}};

    return asthra_test_run_suite(&suite);
}
