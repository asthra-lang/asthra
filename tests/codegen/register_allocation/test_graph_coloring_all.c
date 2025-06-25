/**
 * Asthra Programming Language Compiler
 * Graph Coloring Register Allocation - Master Test Runner
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file provides a master test runner that executes all graph coloring
 * test suites including interference graph, algorithms, and heuristics tests.
 */

#include "graph_coloring_common.h"

// =============================================================================
// EXTERNAL TEST FUNCTION DECLARATIONS
// =============================================================================

// From test_interference_graph.c
extern AsthraTestResult test_interference_graph_construction(AsthraTestContext *context);
extern AsthraTestResult test_interference_graph_degree(AsthraTestContext *context);

// From test_graph_coloring_algorithms.c
extern AsthraTestResult test_graph_coloring_basic(AsthraTestContext *context);
extern AsthraTestResult test_graph_coloring_bipartite(AsthraTestContext *context);
extern AsthraTestResult test_graph_coloring_spilling(AsthraTestContext *context);

// From test_graph_coloring_heuristics.c
extern AsthraTestResult test_simplicial_elimination_ordering(AsthraTestContext *context);
extern AsthraTestResult test_minimum_degree_heuristic(AsthraTestContext *context);
extern AsthraTestResult test_largest_degree_first_heuristic(AsthraTestContext *context);

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite suite = {
        .name = "Graph Coloring Register Allocation - All Tests",
        .tests = {// Interference Graph Tests
                  {"Interference Graph Construction", test_interference_graph_construction},
                  {"Interference Graph Degree", test_interference_graph_degree},

                  // Graph Coloring Algorithm Tests
                  {"Basic Graph Coloring", test_graph_coloring_basic},
                  {"Bipartite Graph Coloring", test_graph_coloring_bipartite},
                  {"Graph Coloring with Spilling", test_graph_coloring_spilling},

                  // Heuristic Tests
                  {"Simplicial Elimination Ordering", test_simplicial_elimination_ordering},
                  {"Minimum Degree Heuristic", test_minimum_degree_heuristic},
                  {"Largest Degree First Heuristic", test_largest_degree_first_heuristic},

                  {NULL, NULL}}};

    return asthra_test_run_suite(&suite);
}
