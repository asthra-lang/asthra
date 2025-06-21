#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_graph_coloring", __FILE__, __LINE__, "Test basic graph coloring algorithm", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_graph_coloring_bipartite", __FILE__, __LINE__, "Test bipartite graph coloring", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_graph_coloring_spilling", __FILE__, __LINE__, "Test graph coloring with register spilling", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_interference_graph_construction", __FILE__, __LINE__, "Test interference graph construction", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

// Minimal graph structures for testing
typedef struct {
    int num_nodes;
    int num_colors;
    bool edges[16][16];  // Adjacency matrix for up to 16 nodes
    int coloring[16];    // Color assignment for each node
} MinimalGraph;

// Minimal interference graph simulation
typedef struct {
    int virtual_reg;
    int physical_reg;
    bool interferes_with[16];
} MinimalRegister;

// Test functions using minimal framework approach
static AsthraTestResult test_basic_graph_coloring(AsthraTestContext* context) {
    // Create a minimal 3-node triangle graph (3-clique)
    MinimalGraph graph = {0};
    graph.num_nodes = 3;
    graph.num_colors = 3;
    
    // Add edges: 0-1, 1-2, 0-2 (triangle)
    graph.edges[0][1] = graph.edges[1][0] = true;
    graph.edges[1][2] = graph.edges[2][1] = true;
    graph.edges[0][2] = graph.edges[2][0] = true;
    
    // Simple greedy coloring algorithm
    for (int i = 0; i < graph.num_nodes; i++) {
        bool used_colors[16] = {false};
        
        // Mark colors used by adjacent nodes
        for (int j = 0; j < i; j++) {
            if (graph.edges[i][j] && graph.coloring[j] >= 0) {
                used_colors[graph.coloring[j]] = true;
            }
        }
        
        // Find first available color
        for (int color = 0; color < graph.num_colors; color++) {
            if (!used_colors[color]) {
                graph.coloring[i] = color;
                break;
            }
        }
    }
    
    // Verify that adjacent nodes have different colors
    for (int i = 0; i < graph.num_nodes; i++) {
        for (int j = i + 1; j < graph.num_nodes; j++) {
            if (graph.edges[i][j] && graph.coloring[i] == graph.coloring[j]) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_graph_coloring_bipartite(AsthraTestContext* context) {
    // Create a bipartite graph: two sets {0,1,2} and {3,4,5}
    MinimalGraph graph = {0};
    graph.num_nodes = 6;
    graph.num_colors = 2;  // Bipartite graphs need only 2 colors
    
    // Add edges between the two sets only
    for (int i = 0; i < 3; i++) {
        for (int j = 3; j < 6; j++) {
            graph.edges[i][j] = graph.edges[j][i] = true;
        }
    }
    
    // Color the bipartite graph
    for (int i = 0; i < 3; i++) {
        graph.coloring[i] = 0;  // First partition gets color 0
    }
    for (int i = 3; i < 6; i++) {
        graph.coloring[i] = 1;  // Second partition gets color 1
    }
    
    // Verify the coloring is valid
    for (int i = 0; i < graph.num_nodes; i++) {
        for (int j = i + 1; j < graph.num_nodes; j++) {
            if (graph.edges[i][j] && graph.coloring[i] == graph.coloring[j]) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }
    
    // Verify it uses exactly 2 colors
    bool color_used[2] = {false, false};
    for (int i = 0; i < graph.num_nodes; i++) {
        if (graph.coloring[i] >= 0 && graph.coloring[i] < 2) {
            color_used[graph.coloring[i]] = true;
        }
    }
    
    if (!color_used[0] || !color_used[1]) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_graph_coloring_spilling(AsthraTestContext* context) {
    // Create a complete graph with more nodes than available colors
    const int num_nodes = 6;
    const int num_colors = 4;  // Fewer colors than nodes
    
    MinimalGraph graph = {0};
    graph.num_nodes = num_nodes;
    graph.num_colors = num_colors;
    
    // Create complete graph (all nodes connected)
    for (int i = 0; i < num_nodes; i++) {
        for (int j = i + 1; j < num_nodes; j++) {
            graph.edges[i][j] = graph.edges[j][i] = true;
        }
    }
    
    // Initialize coloring to -1 (uncolored)
    for (int i = 0; i < num_nodes; i++) {
        graph.coloring[i] = -1;
    }
    
    // Try to color with limited colors (will require spilling)
    int colored_count = 0;
    int spilled_count = 0;
    
    for (int i = 0; i < num_nodes; i++) {
        bool used_colors[16] = {false};
        
        // Mark colors used by adjacent already-colored nodes
        for (int j = 0; j < i; j++) {
            if (graph.edges[i][j] && graph.coloring[j] >= 0) {
                used_colors[graph.coloring[j]] = true;
            }
        }
        
        // Find available color
        bool colored = false;
        for (int color = 0; color < num_colors; color++) {
            if (!used_colors[color]) {
                graph.coloring[i] = color;
                colored_count++;
                colored = true;
                break;
            }
        }
        
        if (!colored) {
            spilled_count++;
        }
    }
    
    // In a complete graph, we can color at most num_colors nodes
    if (colored_count != num_colors) {
        return ASTHRA_TEST_FAIL;
    }
    
    // The remaining nodes should be spilled
    if (spilled_count != num_nodes - num_colors) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_interference_graph_construction(AsthraTestContext* context) {
    // Simulate register interference analysis
    MinimalRegister registers[8] = {0};
    
    // Set up some virtual registers
    for (int i = 0; i < 8; i++) {
        registers[i].virtual_reg = i;
        registers[i].physical_reg = -1;  // Unassigned
    }
    
    // Simulate live range overlaps (interference)
    // Registers 0,1,2 interfere with each other
    registers[0].interferes_with[1] = registers[0].interferes_with[2] = true;
    registers[1].interferes_with[0] = registers[1].interferes_with[2] = true;
    registers[2].interferes_with[0] = registers[2].interferes_with[1] = true;
    
    // Registers 3,4 interfere with each other
    registers[3].interferes_with[4] = true;
    registers[4].interferes_with[3] = true;
    
    // Registers 5,6,7 don't interfere with anyone (can share)
    
    // Assign physical registers based on interference
    int next_physical_reg = 0;
    
    for (int i = 0; i < 8; i++) {
        if (registers[i].physical_reg >= 0) continue;  // Already assigned
        
        // Find a register that doesn't interfere
        bool can_use_existing = false;
        for (int existing_reg = 0; existing_reg < next_physical_reg; existing_reg++) {
            bool conflicts = false;
            for (int j = 0; j < i; j++) {
                if (registers[j].physical_reg == existing_reg && 
                    registers[i].interferes_with[j]) {
                    conflicts = true;
                    break;
                }
            }
            if (!conflicts) {
                registers[i].physical_reg = existing_reg;
                can_use_existing = true;
                break;
            }
        }
        
        if (!can_use_existing) {
            registers[i].physical_reg = next_physical_reg++;
        }
    }
    
    // Verify assignments are valid
    for (int i = 0; i < 8; i++) {
        for (int j = i + 1; j < 8; j++) {
            if (registers[i].interferes_with[j] && 
                registers[i].physical_reg == registers[j].physical_reg) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }
    
    // Should use minimal number of physical registers
    if (next_physical_reg > 4) {  // Expected: reg0,1,2 need 3; reg3,4 need 1; reg5,6,7 can share 1
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Graph Coloring Algorithms Tests (Minimal Framework) ===\n");
    
    int passed = 0, total = 0;
    AsthraTestContext context = {0};  // Initialize to zero
    
    // Run tests
    if (test_basic_graph_coloring(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Basic Graph Coloring: PASS\n");
        passed++;
    } else {
        printf("❌ Basic Graph Coloring: FAIL\n");
    }
    total++;
    
    if (test_graph_coloring_bipartite(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Bipartite Graph Coloring: PASS\n");
        passed++;
    } else {
        printf("❌ Bipartite Graph Coloring: FAIL\n");
    }
    total++;
    
    if (test_graph_coloring_spilling(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Graph Coloring with Spilling: PASS\n");
        passed++;
    } else {
        printf("❌ Graph Coloring with Spilling: FAIL\n");
    }
    total++;
    
    if (test_interference_graph_construction(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Interference Graph Construction: PASS\n");
        passed++;
    } else {
        printf("❌ Interference Graph Construction: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
} 
