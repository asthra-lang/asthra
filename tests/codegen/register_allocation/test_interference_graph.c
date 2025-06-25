/**
 * Asthra Programming Language Compiler
 * Interference Graph Testing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file implements testing for interference graph construction
 * including edge management and degree calculation.
 */

#include "graph_coloring_common.h"

// =============================================================================
// INTERFERENCE GRAPH CONSTRUCTION TESTS
// =============================================================================

/**
 * Test basic interference graph construction
 */
AsthraTestResult test_interference_graph_construction(AsthraTestContext *context) {
    GraphColoringTestFixture *fixture = setup_graph_coloring_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Add some interference edges
    bool result1 = interference_graph_add_edge(fixture->interference_graph, 0, 1);
    bool result2 = interference_graph_add_edge(fixture->interference_graph, 1, 2);
    bool result3 = interference_graph_add_edge(fixture->interference_graph, 0, 2);

    if (!asthra_test_assert_bool(context, result1 && result2 && result3,
                                 "Failed to add interference edges")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that edges exist
    if (!asthra_test_assert_bool(context,
                                 interference_graph_has_edge(fixture->interference_graph, 0, 1),
                                 "Interference edge (0,1) should exist")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context,
                                 interference_graph_has_edge(fixture->interference_graph, 1, 2),
                                 "Interference edge (1,2) should exist")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_bool(context,
                                 interference_graph_has_edge(fixture->interference_graph, 0, 2),
                                 "Interference edge (0,2) should exist")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check symmetry (undirected graph)
    if (!asthra_test_assert_bool(context,
                                 interference_graph_has_edge(fixture->interference_graph, 1, 0),
                                 "Interference edge should be symmetric (1,0)")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_graph_coloring_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test interference graph degree calculation
 */
AsthraTestResult test_interference_graph_degree(AsthraTestContext *context) {
    GraphColoringTestFixture *fixture = setup_graph_coloring_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a star graph: node 0 connected to nodes 1, 2, 3, 4
    interference_graph_add_edge(fixture->interference_graph, 0, 1);
    interference_graph_add_edge(fixture->interference_graph, 0, 2);
    interference_graph_add_edge(fixture->interference_graph, 0, 3);
    interference_graph_add_edge(fixture->interference_graph, 0, 4);

    // Node 0 should have degree 4
    int degree_0 = interference_graph_get_degree(fixture->interference_graph, 0);
    if (!asthra_test_assert_int(context, degree_0, 4, "Node 0 should have degree 4")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Nodes 1-4 should each have degree 1
    for (int i = 1; i <= 4; i++) {
        int degree = interference_graph_get_degree(fixture->interference_graph, i);
        if (!asthra_test_assert_int(context, degree, 1, "Node %d should have degree 1", i)) {
            cleanup_graph_coloring_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    // Node 5 (unconnected) should have degree 0
    int degree_5 = interference_graph_get_degree(fixture->interference_graph, 5);
    if (!asthra_test_assert_int(context, degree_5, 0, "Node 5 should have degree 0")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_graph_coloring_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite suite = {
        .name = "Interference Graph Tests",
        .tests = {{"Interference Graph Construction", test_interference_graph_construction},
                  {"Interference Graph Degree", test_interference_graph_degree},
                  {NULL, NULL}}};

    return asthra_test_run_suite(&suite);
}
