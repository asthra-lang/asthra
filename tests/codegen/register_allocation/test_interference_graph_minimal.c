#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_interference_graph_construction", __FILE__, __LINE__,
     "Test interference graph construction", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_interference_graph_degree", __FILE__, __LINE__,
     "Test interference graph degree calculation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false,
     NULL},
    {"test_interference_graph_neighbors", __FILE__, __LINE__,
     "Test interference graph neighbor management", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL,
     false, NULL},
    {"test_live_range_interference", __FILE__, __LINE__, "Test live range interference detection",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}};

// Minimal interference graph structure
typedef struct {
    bool adjacency_matrix[16][16]; // Support up to 16 virtual registers
    int node_count;
    int degrees[16];
} MinimalInterferenceGraph;

// Minimal live range structure
typedef struct {
    int virtual_reg;
    int start_point;
    int end_point;
} MinimalLiveRange;

// Helper functions
static void init_interference_graph(MinimalInterferenceGraph *graph, int node_count) {
    graph->node_count = node_count;

    // Initialize adjacency matrix
    for (int i = 0; i < 16; i++) {
        graph->degrees[i] = 0;
        for (int j = 0; j < 16; j++) {
            graph->adjacency_matrix[i][j] = false;
        }
    }
}

static void add_interference_edge(MinimalInterferenceGraph *graph, int node1, int node2) {
    if (node1 >= 0 && node1 < graph->node_count && node2 >= 0 && node2 < graph->node_count &&
        node1 != node2) {
        if (!graph->adjacency_matrix[node1][node2]) {
            graph->adjacency_matrix[node1][node2] = true;
            graph->adjacency_matrix[node2][node1] = true;
            graph->degrees[node1]++;
            graph->degrees[node2]++;
        }
    }
}

static bool nodes_interfere(const MinimalInterferenceGraph *graph, int node1, int node2) {
    if (node1 >= 0 && node1 < graph->node_count && node2 >= 0 && node2 < graph->node_count) {
        return graph->adjacency_matrix[node1][node2];
    }
    return false;
}

static int get_node_degree(const MinimalInterferenceGraph *graph, int node) {
    if (node >= 0 && node < graph->node_count) {
        return graph->degrees[node];
    }
    return -1;
}

static bool live_ranges_overlap(const MinimalLiveRange *range1, const MinimalLiveRange *range2) {
    return !(range1->end_point <= range2->start_point || range2->end_point <= range1->start_point);
}

// Test functions using minimal framework approach
static AsthraTestResult test_interference_graph_construction(AsthraTestContext *context) {
    MinimalInterferenceGraph graph;
    init_interference_graph(&graph, 6);

    // Add some interference edges
    add_interference_edge(&graph, 0, 1);
    add_interference_edge(&graph, 1, 2);
    add_interference_edge(&graph, 2, 0); // Triangle: 0-1-2-0
    add_interference_edge(&graph, 3, 4); // Separate edge: 3-4
    add_interference_edge(&graph, 0, 5); // Connect node 5 to triangle

    // Verify edges were added correctly
    if (!nodes_interfere(&graph, 0, 1) || !nodes_interfere(&graph, 1, 0)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!nodes_interfere(&graph, 1, 2) || !nodes_interfere(&graph, 2, 1)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!nodes_interfere(&graph, 2, 0) || !nodes_interfere(&graph, 0, 2)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!nodes_interfere(&graph, 3, 4) || !nodes_interfere(&graph, 4, 3)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!nodes_interfere(&graph, 0, 5) || !nodes_interfere(&graph, 5, 0)) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify non-edges
    if (nodes_interfere(&graph, 1, 3) || nodes_interfere(&graph, 2, 4)) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify no self-edges
    for (int i = 0; i < graph.node_count; i++) {
        if (nodes_interfere(&graph, i, i)) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_interference_graph_degree(AsthraTestContext *context) {
    MinimalInterferenceGraph graph;
    init_interference_graph(&graph, 5);

    // Create a star graph: node 0 connected to all others
    add_interference_edge(&graph, 0, 1);
    add_interference_edge(&graph, 0, 2);
    add_interference_edge(&graph, 0, 3);
    add_interference_edge(&graph, 0, 4);

    // Verify degrees
    if (get_node_degree(&graph, 0) != 4) { // Center node has degree 4
        return ASTHRA_TEST_FAIL;
    }

    for (int i = 1; i < 5; i++) {
        if (get_node_degree(&graph, i) != 1) { // Leaf nodes have degree 1
            return ASTHRA_TEST_FAIL;
        }
    }

    // Add more edges and verify degree updates
    add_interference_edge(&graph, 1, 2); // Connect two leaf nodes

    if (get_node_degree(&graph, 1) != 2) { // Node 1 now has degree 2
        return ASTHRA_TEST_FAIL;
    }

    if (get_node_degree(&graph, 2) != 2) { // Node 2 now has degree 2
        return ASTHRA_TEST_FAIL;
    }

    // Adding duplicate edge should not change degrees
    add_interference_edge(&graph, 0, 1); // Already exists
    if (get_node_degree(&graph, 0) != 4 || get_node_degree(&graph, 1) != 2) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_interference_graph_neighbors(AsthraTestContext *context) {
    MinimalInterferenceGraph graph;
    init_interference_graph(&graph, 6);

    // Create a specific pattern
    add_interference_edge(&graph, 0, 1);
    add_interference_edge(&graph, 0, 2);
    add_interference_edge(&graph, 0, 3);
    add_interference_edge(&graph, 1, 4);
    add_interference_edge(&graph, 2, 5);

    // Count neighbors of node 0
    int neighbors_of_0 = 0;
    for (int i = 0; i < graph.node_count; i++) {
        if (nodes_interfere(&graph, 0, i)) {
            neighbors_of_0++;
        }
    }

    if (neighbors_of_0 != 3) { // Should have neighbors 1, 2, 3
        return ASTHRA_TEST_FAIL;
    }

    // Count neighbors of node 1
    int neighbors_of_1 = 0;
    for (int i = 0; i < graph.node_count; i++) {
        if (nodes_interfere(&graph, 1, i)) {
            neighbors_of_1++;
        }
    }

    if (neighbors_of_1 != 2) { // Should have neighbors 0, 4
        return ASTHRA_TEST_FAIL;
    }

    // Verify specific neighbor relationships
    if (!nodes_interfere(&graph, 0, 1) || !nodes_interfere(&graph, 0, 2) ||
        !nodes_interfere(&graph, 0, 3)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!nodes_interfere(&graph, 1, 4) || !nodes_interfere(&graph, 2, 5)) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify non-neighbors
    if (nodes_interfere(&graph, 1, 2) || nodes_interfere(&graph, 3, 4) ||
        nodes_interfere(&graph, 4, 5)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_live_range_interference(AsthraTestContext *context) {
    // Test live range overlap detection and interference graph construction
    MinimalLiveRange ranges[6] = {
        {100, 0, 10},  // Register 100: points 0-10
        {101, 5, 15},  // Register 101: points 5-15 (overlaps with 100)
        {102, 12, 20}, // Register 102: points 12-20 (overlaps with 101)
        {103, 18, 25}, // Register 103: points 18-25 (overlaps with 102)
        {104, 30, 35}, // Register 104: points 30-35 (no overlap)
        {105, 8, 14}   // Register 105: points 8-14 (overlaps with 100, 101)
    };

    // Build interference graph based on live range overlaps
    MinimalInterferenceGraph graph;
    init_interference_graph(&graph, 6);

    for (int i = 0; i < 6; i++) {
        for (int j = i + 1; j < 6; j++) {
            if (live_ranges_overlap(&ranges[i], &ranges[j])) {
                add_interference_edge(&graph, i, j);
            }
        }
    }

    // Verify expected interferences
    // Range 0 (0-10) overlaps with Range 1 (5-15) and Range 5 (8-14)
    if (!nodes_interfere(&graph, 0, 1) || !nodes_interfere(&graph, 0, 5)) {
        return ASTHRA_TEST_FAIL;
    }

    // Range 1 (5-15) overlaps with Range 0 (0-10), Range 2 (12-20), and Range 5 (8-14)
    if (!nodes_interfere(&graph, 1, 0) || !nodes_interfere(&graph, 1, 2) ||
        !nodes_interfere(&graph, 1, 5)) {
        return ASTHRA_TEST_FAIL;
    }

    // Range 2 (12-20) overlaps with Range 1 (5-15) and Range 3 (18-25)
    if (!nodes_interfere(&graph, 2, 1) || !nodes_interfere(&graph, 2, 3)) {
        return ASTHRA_TEST_FAIL;
    }

    // Range 4 (30-35) should not interfere with any other range
    for (int i = 0; i < 6; i++) {
        if (i != 4 && nodes_interfere(&graph, 4, i)) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Verify specific non-interferences
    if (nodes_interfere(&graph, 0, 2) || nodes_interfere(&graph, 0, 3) ||
        nodes_interfere(&graph, 0, 4)) {
        return ASTHRA_TEST_FAIL;
    }

    // Count total edges
    int total_edges = 0;
    for (int i = 0; i < 6; i++) {
        total_edges += graph.degrees[i];
    }
    total_edges /= 2; // Each edge counted twice

    // Expected edges: 0-1, 0-5, 1-2, 1-5, 2-3, 2-5 = 6 edges
    // Range overlaps: (0-10)&(5-15), (0-10)&(8-14), (5-15)&(12-20), (5-15)&(8-14), (12-20)&(18-25),
    // (12-20)&(8-14)
    if (total_edges != 6) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Interference Graph Tests (Minimal Framework) ===\n");

    AsthraTestContext context = {0}; // Initialize to zero

    int passed = 0, total = 0;

    // Run tests
    if (test_interference_graph_construction(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Interference Graph Construction: PASS\n");
        passed++;
    } else {
        printf("❌ Interference Graph Construction: FAIL\n");
    }
    total++;

    if (test_interference_graph_degree(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Interference Graph Degree: PASS\n");
        passed++;
    } else {
        printf("❌ Interference Graph Degree: FAIL\n");
    }
    total++;

    if (test_interference_graph_neighbors(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Interference Graph Neighbors: PASS\n");
        passed++;
    } else {
        printf("❌ Interference Graph Neighbors: FAIL\n");
    }
    total++;

    if (test_live_range_interference(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Live Range Interference: PASS\n");
        passed++;
    } else {
        printf("❌ Live Range Interference: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
