/**
 * Asthra Programming Language Compiler
 * Linear Scan Register Allocation Testing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file implements testing for linear scan register allocation algorithms
 * including basic allocation, overlapping lifetimes, and register pressure handling.
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "ast.h"
#include "code_generator_core.h"
#include "code_generator_instructions.h"
#include "code_generator_labels.h"
#include "register_allocator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST FIXTURE AND UTILITIES
// =============================================================================

/**
 * Test fixture for linear scan allocation testing
 */
typedef struct {
    RegisterAllocator *allocator;
    InstructionBuffer *instruction_buffer;
    LabelManager *label_manager;
    LivenessAnalysis *liveness;
    size_t num_virtual_registers;
    size_t num_physical_registers;
} LinearScanTestFixture;

/**
 * Setup test fixture with a register allocator
 */
static LinearScanTestFixture *setup_linear_scan_fixture(void) {
    LinearScanTestFixture *fixture = calloc(1, sizeof(LinearScanTestFixture));
    if (!fixture)
        return NULL;

    fixture->num_virtual_registers = 16;
    fixture->num_physical_registers = 8;

    fixture->allocator = register_allocator_create();
    if (!fixture->allocator) {
        free(fixture);
        return NULL;
    }

    fixture->instruction_buffer = instruction_buffer_create(1024);
    if (!fixture->instruction_buffer) {
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    fixture->label_manager = label_manager_create(16);
    if (!fixture->label_manager) {
        instruction_buffer_destroy(fixture->instruction_buffer);
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    fixture->liveness = liveness_analysis_create();
    if (!fixture->liveness) {
        label_manager_destroy(fixture->label_manager);
        instruction_buffer_destroy(fixture->instruction_buffer);
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    return fixture;
}

/**
 * Cleanup test fixture
 */
static void cleanup_linear_scan_fixture(LinearScanTestFixture *fixture) {
    if (!fixture)
        return;

    if (fixture->liveness) {
        liveness_analysis_destroy(fixture->liveness);
    }
    if (fixture->label_manager) {
        label_manager_destroy(fixture->label_manager);
    }
    if (fixture->instruction_buffer) {
        instruction_buffer_destroy(fixture->instruction_buffer);
    }
    if (fixture->allocator) {
        register_allocator_destroy(fixture->allocator);
    }
    free(fixture);
}

// =============================================================================
// LINEAR SCAN ALLOCATION TESTS
// =============================================================================

/**
 * Test basic linear scan allocation
 */
AsthraTestResult test_linear_scan_basic_allocation(AsthraTestContext *context) {
    LinearScanTestFixture *fixture = setup_linear_scan_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create simple live intervals for testing
    LiveInterval intervals[4];
    intervals[0] =
        (LiveInterval){.virtual_reg = 0, .start = 0, .end = 10, .physical_reg = REG_UNASSIGNED};
    intervals[1] =
        (LiveInterval){.virtual_reg = 1, .start = 5, .end = 15, .physical_reg = REG_UNASSIGNED};
    intervals[2] =
        (LiveInterval){.virtual_reg = 2, .start = 12, .end = 20, .physical_reg = REG_UNASSIGNED};
    intervals[3] =
        (LiveInterval){.virtual_reg = 3, .start = 18, .end = 25, .physical_reg = REG_UNASSIGNED};

    // Perform linear scan allocation
    bool result = register_allocator_linear_scan(fixture->allocator, intervals, 4);
    if (!asthra_test_assert_bool(context, result, "Linear scan allocation failed")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that registers were assigned
    if (!asthra_test_assert_bool(context, intervals[0].physical_reg != REG_UNASSIGNED,
                                 "Virtual register 0 should be assigned a physical register")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, intervals[1].physical_reg != REG_UNASSIGNED,
                                 "Virtual register 1 should be assigned a physical register")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that non-overlapping intervals can share registers
    if (!asthra_test_assert_bool(
            context, intervals[0].physical_reg == intervals[2].physical_reg,
            "Non-overlapping intervals should share the same physical register")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_linear_scan_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test linear scan allocation with overlapping lifetimes
 */
AsthraTestResult test_linear_scan_overlapping_lifetimes(AsthraTestContext *context) {
    LinearScanTestFixture *fixture = setup_linear_scan_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create overlapping live intervals
    LiveInterval intervals[6];
    intervals[0] =
        (LiveInterval){.virtual_reg = 0, .start = 0, .end = 20, .physical_reg = REG_UNASSIGNED};
    intervals[1] =
        (LiveInterval){.virtual_reg = 1, .start = 5, .end = 25, .physical_reg = REG_UNASSIGNED};
    intervals[2] =
        (LiveInterval){.virtual_reg = 2, .start = 10, .end = 30, .physical_reg = REG_UNASSIGNED};
    intervals[3] =
        (LiveInterval){.virtual_reg = 3, .start = 15, .end = 35, .physical_reg = REG_UNASSIGNED};
    intervals[4] =
        (LiveInterval){.virtual_reg = 4, .start = 20, .end = 40, .physical_reg = REG_UNASSIGNED};
    intervals[5] =
        (LiveInterval){.virtual_reg = 5, .start = 25, .end = 45, .physical_reg = REG_UNASSIGNED};

    // Perform linear scan allocation
    bool result = register_allocator_linear_scan(fixture->allocator, intervals, 6);
    if (!asthra_test_assert_bool(context, result,
                                 "Linear scan allocation with overlapping lifetimes failed")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that overlapping intervals get different registers
    if (!asthra_test_assert_bool(context, intervals[0].physical_reg != intervals[1].physical_reg,
                                 "Overlapping intervals should get different physical registers")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, intervals[1].physical_reg != intervals[2].physical_reg,
                                 "Overlapping intervals should get different physical registers")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that all intervals got assigned
    for (int i = 0; i < 6; i++) {
        if (!asthra_test_assert_bool(context, intervals[i].physical_reg != REG_UNASSIGNED,
                                     "Virtual register %d should be assigned a physical register",
                                     i)) {
            cleanup_linear_scan_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    cleanup_linear_scan_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test linear scan allocation with register pressure
 */
AsthraTestResult test_linear_scan_register_pressure(AsthraTestContext *context) {
    LinearScanTestFixture *fixture = setup_linear_scan_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create more intervals than available physical registers (8 physical, 10 virtual)
    LiveInterval intervals[10];
    for (int i = 0; i < 10; i++) {
        intervals[i] = (LiveInterval){.virtual_reg = i,
                                      .start = i * 2,
                                      .end = i * 2 + 15, // All overlap significantly
                                      .physical_reg = REG_UNASSIGNED};
    }

    // Perform linear scan allocation - should handle spilling
    bool result = register_allocator_linear_scan(fixture->allocator, intervals, 10);
    if (!asthra_test_assert_bool(context, result,
                                 "Linear scan allocation under register pressure failed")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Count how many got physical registers vs spilled
    int assigned_count = 0;
    int spilled_count = 0;

    for (int i = 0; i < 10; i++) {
        if (intervals[i].physical_reg != REG_UNASSIGNED) {
            assigned_count++;
        } else {
            spilled_count++;
        }
    }

    // Should have assigned up to the number of physical registers
    if (!asthra_test_assert_bool(context, assigned_count <= (int)fixture->num_physical_registers,
                                 "Should not assign more than available physical registers")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Should have spilled some registers due to pressure
    if (!asthra_test_assert_bool(context, spilled_count > 0,
                                 "Should have spilled some registers due to register pressure")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_linear_scan_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test linear scan allocation with holes in lifetimes
 */
AsthraTestResult test_linear_scan_lifetime_holes(AsthraTestContext *context) {
    LinearScanTestFixture *fixture = setup_linear_scan_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create intervals with gaps that could be exploited
    LiveInterval intervals[4];
    intervals[0] = (LiveInterval){
        .virtual_reg = 0, .start = 0, .end = 10, .physical_reg = REG_UNASSIGNED}; // 0-10
    intervals[1] = (LiveInterval){.virtual_reg = 1,
                                  .start = 15,
                                  .end = 25,
                                  .physical_reg = REG_UNASSIGNED}; // 15-25 (gap from 10-15)
    intervals[2] = (LiveInterval){.virtual_reg = 2,
                                  .start = 5,
                                  .end = 20,
                                  .physical_reg = REG_UNASSIGNED}; // 5-20 (overlaps both)
    intervals[3] = (LiveInterval){.virtual_reg = 3,
                                  .start = 12,
                                  .end = 18,
                                  .physical_reg = REG_UNASSIGNED}; // 12-18 (fits in gap)

    // Perform linear scan allocation
    bool result = register_allocator_linear_scan(fixture->allocator, intervals, 4);
    if (!asthra_test_assert_bool(context, result,
                                 "Linear scan allocation with lifetime holes failed")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that all intervals got assigned
    for (int i = 0; i < 4; i++) {
        if (!asthra_test_assert_bool(context, intervals[i].physical_reg != REG_UNASSIGNED,
                                     "Virtual register %d should be assigned a physical register",
                                     i)) {
            cleanup_linear_scan_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // intervals[0] and intervals[1] should be able to share a register (no overlap)
    if (!asthra_test_assert_bool(
            context, intervals[0].physical_reg == intervals[1].physical_reg,
            "Non-overlapping intervals should share the same physical register")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_linear_scan_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test linear scan allocation with precolored registers
 */
AsthraTestResult test_linear_scan_precolored_registers(AsthraTestContext *context) {
    LinearScanTestFixture *fixture = setup_linear_scan_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create intervals with some precolored (e.g., function parameters, return values)
    LiveInterval intervals[5];
    intervals[0] = (LiveInterval){
        .virtual_reg = 0, .start = 0, .end = 10, .physical_reg = 0}; // Precolored to reg 0
    intervals[1] =
        (LiveInterval){.virtual_reg = 1, .start = 5, .end = 15, .physical_reg = REG_UNASSIGNED};
    intervals[2] = (LiveInterval){
        .virtual_reg = 2, .start = 8, .end = 18, .physical_reg = 1}; // Precolored to reg 1
    intervals[3] =
        (LiveInterval){.virtual_reg = 3, .start = 12, .end = 22, .physical_reg = REG_UNASSIGNED};
    intervals[4] =
        (LiveInterval){.virtual_reg = 4, .start = 16, .end = 26, .physical_reg = REG_UNASSIGNED};

    // Perform linear scan allocation
    bool result = register_allocator_linear_scan(fixture->allocator, intervals, 5);
    if (!asthra_test_assert_bool(context, result,
                                 "Linear scan allocation with precolored registers failed")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that precolored registers maintained their assignments
    if (!asthra_test_assert_bool(context, intervals[0].physical_reg == 0,
                                 "Precolored register should maintain its assignment")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context, intervals[2].physical_reg == 1,
                                 "Precolored register should maintain its assignment")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that other intervals don't conflict with precolored ones
    if (!asthra_test_assert_bool(context,
                                 intervals[1].physical_reg != 0 && intervals[1].physical_reg != 1,
                                 "Overlapping interval should not use precolored registers")) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_linear_scan_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test linear scan allocation performance characteristics
 */
AsthraTestResult test_linear_scan_performance(AsthraTestContext *context) {
    LinearScanTestFixture *fixture = setup_linear_scan_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a large number of intervals to test performance
    const int num_intervals = 100;
    LiveInterval *intervals = malloc(num_intervals * sizeof(LiveInterval));
    if (!intervals) {
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Generate intervals with varying lifetimes
    for (int i = 0; i < num_intervals; i++) {
        intervals[i] = (LiveInterval){.virtual_reg = i,
                                      .start = i * 3,
                                      .end = i * 3 + (i % 10) + 5, // Varying lengths
                                      .physical_reg = REG_UNASSIGNED};
    }

    // Measure allocation time (basic performance test)
    clock_t start_time = clock();
    bool result = register_allocator_linear_scan(fixture->allocator, intervals, num_intervals);
    clock_t end_time = clock();

    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    if (!asthra_test_assert_bool(context, result,
                                 "Linear scan allocation of large interval set failed")) {
        free(intervals);
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Should complete in reasonable time (less than 1 second for 100 intervals)
    if (!asthra_test_assert_bool(
            context, elapsed_time < 1.0,
            "Linear scan allocation should complete in reasonable time, took %f seconds",
            elapsed_time)) {
        free(intervals);
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that allocation was successful for most intervals
    int assigned_count = 0;
    for (int i = 0; i < num_intervals; i++) {
        if (intervals[i].physical_reg != REG_UNASSIGNED) {
            assigned_count++;
        }
    }

    // Should assign a reasonable percentage (at least 50% given register reuse)
    if (!asthra_test_assert_bool(context, assigned_count >= num_intervals / 2,
                                 "Should assign at least 50%% of intervals, assigned %d/%d",
                                 assigned_count, num_intervals)) {
        free(intervals);
        cleanup_linear_scan_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    free(intervals);
    cleanup_linear_scan_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite suite = {
        .name = "Linear Scan Register Allocation Tests",
        .tests = {{"Basic Linear Scan Allocation", test_linear_scan_basic_allocation},
                  {"Overlapping Lifetimes", test_linear_scan_overlapping_lifetimes},
                  {"Register Pressure Handling", test_linear_scan_register_pressure},
                  {"Lifetime Holes", test_linear_scan_lifetime_holes},
                  {"Precolored Registers", test_linear_scan_precolored_registers},
                  {"Performance Characteristics", test_linear_scan_performance},
                  {NULL, NULL}}};

    return asthra_test_run_suite(&suite);
}
