/**
 * Asthra Programming Language Compiler
 * Graph Coloring Algorithms Testing
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file implements testing for graph coloring algorithms including
 * basic coloring, different graph types, and spilling scenarios.
 */

#include "graph_coloring_common.h"

// =============================================================================
// GRAPH COLORING ALGORITHM TESTS
// =============================================================================

/**
 * Test basic graph coloring with simple graph
 */
AsthraTestResult test_graph_coloring_basic(AsthraTestContext* context) {
    GraphColoringTestFixture* fixture = setup_graph_coloring_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create a triangle graph (3-clique): 0-1, 1-2, 0-2
    interference_graph_add_edge(fixture->interference_graph, 0, 1);
    interference_graph_add_edge(fixture->interference_graph, 1, 2);
    interference_graph_add_edge(fixture->interference_graph, 0, 2);
    
    // Perform graph coloring
    int* coloring = register_allocator_graph_coloring(fixture->allocator, fixture->interference_graph);
    if (!asthra_test_assert_pointer(context, coloring, "Graph coloring should return a valid coloring")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that adjacent nodes have different colors
    if (!asthra_test_assert_bool(context, coloring[0] != coloring[1],
                                "Adjacent nodes 0 and 1 should have different colors")) {
        free(coloring);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, coloring[1] != coloring[2],
                                "Adjacent nodes 1 and 2 should have different colors")) {
        free(coloring);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_bool(context, coloring[0] != coloring[2],
                                "Adjacent nodes 0 and 2 should have different colors")) {
        free(coloring);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Triangle requires 3 colors
    int max_color = 0;
    for (int i = 0; i < 3; i++) {
        if (coloring[i] > max_color) max_color = coloring[i];
    }
    
    if (!asthra_test_assert_bool(context, max_color >= 2,
                                "Triangle graph should require at least 3 colors (0-indexed)")) {
        free(coloring);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    free(coloring);
    cleanup_graph_coloring_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test graph coloring with bipartite graph
 */
AsthraTestResult test_graph_coloring_bipartite(AsthraTestContext* context) {
    GraphColoringTestFixture* fixture = setup_graph_coloring_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create a bipartite graph: {0,1,2} connected to {3,4,5}
    interference_graph_add_edge(fixture->interference_graph, 0, 3);
    interference_graph_add_edge(fixture->interference_graph, 0, 4);
    interference_graph_add_edge(fixture->interference_graph, 0, 5);
    interference_graph_add_edge(fixture->interference_graph, 1, 3);
    interference_graph_add_edge(fixture->interference_graph, 1, 4);
    interference_graph_add_edge(fixture->interference_graph, 1, 5);
    interference_graph_add_edge(fixture->interference_graph, 2, 3);
    interference_graph_add_edge(fixture->interference_graph, 2, 4);
    interference_graph_add_edge(fixture->interference_graph, 2, 5);
    
    // Perform graph coloring
    int* coloring = register_allocator_graph_coloring(fixture->allocator, fixture->interference_graph);
    if (!asthra_test_assert_pointer(context, coloring, "Graph coloring should return a valid coloring")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that all nodes in first partition have the same color
    if (!asthra_test_assert_bool(context, coloring[0] == coloring[1] && coloring[1] == coloring[2],
                                "Nodes in first partition should have the same color")) {
        free(coloring);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that all nodes in second partition have the same color
    if (!asthra_test_assert_bool(context, coloring[3] == coloring[4] && coloring[4] == coloring[5],
                                "Nodes in second partition should have the same color")) {
        free(coloring);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Check that the two partitions have different colors
    if (!asthra_test_assert_bool(context, coloring[0] != coloring[3],
                                "Different partitions should have different colors")) {
        free(coloring);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Bipartite graph should only need 2 colors
    int max_color = 0;
    for (int i = 0; i < 6; i++) {
        if (coloring[i] > max_color) max_color = coloring[i];
    }
    
    if (!asthra_test_assert_bool(context, max_color <= 1,
                                "Bipartite graph should only need 2 colors (0-indexed)")) {
        free(coloring);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    free(coloring);
    cleanup_graph_coloring_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test graph coloring with register pressure (spilling)
 */
AsthraTestResult test_graph_coloring_spilling(AsthraTestContext* context) {
    GraphColoringTestFixture* fixture = setup_graph_coloring_fixture();
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup test fixture")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create a complete graph with more nodes than available registers
    // This will force spilling
    const int num_nodes = fixture->num_physical_registers + 3;  // 8 + 3 = 11 nodes
    
    for (int i = 0; i < num_nodes; i++) {
        for (int j = i + 1; j < num_nodes; j++) {
            interference_graph_add_edge(fixture->interference_graph, i, j);
        }
    }
    
    // Perform graph coloring with spilling
    RegisterAllocationResult* result = register_allocator_graph_coloring_with_spilling(
        fixture->allocator, fixture->interference_graph);
    
    if (!asthra_test_assert_pointer(context, result, "Graph coloring with spilling should return a result")) {
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Should have some spilled registers
    if (!asthra_test_assert_bool(context, result->num_spilled > 0,
                                "Should have spilled some registers due to complete graph")) {
        register_allocation_result_destroy(result);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Should have assigned some registers
    if (!asthra_test_assert_bool(context, result->num_assigned > 0,
                                "Should have assigned some registers")) {
        register_allocation_result_destroy(result);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    // Total should equal number of nodes
    if (!asthra_test_assert_int(context, result->num_assigned + result->num_spilled, num_nodes,
                               "Total assigned + spilled should equal number of nodes")) {
        register_allocation_result_destroy(result);
        cleanup_graph_coloring_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }
    
    register_allocation_result_destroy(result);
    cleanup_graph_coloring_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite suite = {
        .name = "Graph Coloring Algorithms Tests",
        .tests = {
            { "Basic Graph Coloring", test_graph_coloring_basic },
            { "Bipartite Graph Coloring", test_graph_coloring_bipartite },
            { "Graph Coloring with Spilling", test_graph_coloring_spilling },
            { NULL, NULL }
        }
    };
    
    return asthra_test_run_suite(&suite);
} 
