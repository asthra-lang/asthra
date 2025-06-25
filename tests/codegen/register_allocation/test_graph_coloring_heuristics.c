/**
 * Asthra Programming Language Compiler
 * Graph Coloring Heuristics Testing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file implements testing for graph coloring heuristics including
 * elimination ordering, minimum degree, and largest degree first algorithms.
 */

#include "graph_coloring_common.h"

// =============================================================================
// HEURISTIC TESTS
// =============================================================================

/**
 * Test simplicial elimination ordering heuristic
 */
AsthraTestResult test_simplicial_elimination_ordering(AsthraTestContext *context) {
    GraphColoringTestFixture *fixture = setup_graph_coloring_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a graph with a simplicial node (node connected to a clique)
    // Graph: 0-1, 0-2, 1-2 (triangle), 3-0, 3-1, 3-2 (3 connected to all in triangle)
    interference_graph_add_edge(fixture->interference_graph, 0, 1);
    interference_graph_add_edge(fixture->interference_graph, 1, 2);
    interference_graph_add_edge(fixture->interference_graph, 0, 2);
    interference_graph_add_edge(fixture->interference_graph, 3, 0);
    interference_graph_add_edge(fixture->interference_graph, 3, 1);
    interference_graph_add_edge(fixture->interference_graph, 3, 2);

    // Get simplicial elimination ordering
    int *ordering = interference_graph_simplicial_elimination_ordering(fixture->interference_graph);
    if (!asthra_test_assert_pointer(context, ordering,
                                    "Should return a valid elimination ordering")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Node 3 should be eliminated first (it's simplicial - its neighbors form a clique)
    if (!asthra_test_assert_int(context, ordering[0], 3,
                                "Node 3 should be eliminated first (simplicial)")) {
        free(ordering);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    free(ordering);
    cleanup_graph_coloring_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test minimum degree heuristic
 */
AsthraTestResult test_minimum_degree_heuristic(AsthraTestContext *context) {
    GraphColoringTestFixture *fixture = setup_graph_coloring_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a star graph: node 0 connected to nodes 1, 2, 3, 4
    // Nodes 1-4 have degree 1, node 0 has degree 4
    interference_graph_add_edge(fixture->interference_graph, 0, 1);
    interference_graph_add_edge(fixture->interference_graph, 0, 2);
    interference_graph_add_edge(fixture->interference_graph, 0, 3);
    interference_graph_add_edge(fixture->interference_graph, 0, 4);

    // Get minimum degree ordering
    int *ordering = interference_graph_minimum_degree_ordering(fixture->interference_graph);
    if (!asthra_test_assert_pointer(context, ordering,
                                    "Should return a valid minimum degree ordering")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // First four nodes in ordering should be 1, 2, 3, 4 (in some order) - they have minimum degree
    bool found_low_degree_nodes = true;
    for (int i = 0; i < 4; i++) {
        if (ordering[i] < 1 || ordering[i] > 4) {
            found_low_degree_nodes = false;
            break;
        }
    }

    if (!asthra_test_assert_bool(context, found_low_degree_nodes,
                                 "First four nodes should be the low-degree nodes (1-4)")) {
        free(ordering);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Last node should be 0 (highest degree)
    if (!asthra_test_assert_int(context, ordering[4], 0,
                                "Node 0 should be eliminated last (highest degree)")) {
        free(ordering);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    free(ordering);
    cleanup_graph_coloring_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test largest degree first heuristic
 */
AsthraTestResult test_largest_degree_first_heuristic(AsthraTestContext *context) {
    GraphColoringTestFixture *fixture = setup_graph_coloring_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Create a star graph: node 0 connected to nodes 1, 2, 3, 4
    interference_graph_add_edge(fixture->interference_graph, 0, 1);
    interference_graph_add_edge(fixture->interference_graph, 0, 2);
    interference_graph_add_edge(fixture->interference_graph, 0, 3);
    interference_graph_add_edge(fixture->interference_graph, 0, 4);

    // Perform coloring with largest degree first heuristic
    int *coloring =
        register_allocator_graph_coloring_ldf(fixture->allocator, fixture->interference_graph);
    if (!asthra_test_assert_pointer(context, coloring,
                                    "LDF coloring should return a valid coloring")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Node 0 (highest degree) should get color 0
    if (!asthra_test_assert_int(context, coloring[0], 0,
                                "Highest degree node should get first color")) {
        free(coloring);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Nodes 1-4 should all get the same color (1) since they don't interfere with each other
    for (int i = 1; i <= 4; i++) {
        if (!asthra_test_assert_int(context, coloring[i], 1,
                                    "Low degree nodes should get second color")) {
            free(coloring);
            cleanup_graph_coloring_fixture(fixture);
            return ASTHRA_TEST_FAIL;
        }
    }

    free(coloring);
    cleanup_graph_coloring_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite suite = {
        .name = "Graph Coloring Heuristics Tests",
        .tests = {{"Simplicial Elimination Ordering", test_simplicial_elimination_ordering},
                  {"Minimum Degree Heuristic", test_minimum_degree_heuristic},
                  {"Largest Degree First Heuristic", test_largest_degree_first_heuristic},
                  {NULL, NULL}}};

    return asthra_test_run_suite(&suite);
}
