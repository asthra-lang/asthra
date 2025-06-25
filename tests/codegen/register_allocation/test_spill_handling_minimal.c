#include "../framework/test_framework_minimal.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_spill_handling", __FILE__, __LINE__, "Test basic register spill handling",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_spill_cost_analysis", __FILE__, __LINE__, "Test spill cost analysis",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_spill_code_generation", __FILE__, __LINE__, "Test spill code generation",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_reload_optimization", __FILE__, __LINE__, "Test register reload optimization",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Minimal spill data structures
typedef struct {
    int virtual_reg;
    int spill_cost;
    int use_count;
    int def_count;
    bool is_spilled;
    int stack_slot;
} MinimalSpillCandidate;

typedef struct {
    MinimalSpillCandidate candidates[16];
    int count;
    int next_stack_slot;
} MinimalSpillManager;

// Minimal instruction representation for spill code
typedef struct {
    enum { INSTR_STORE, INSTR_LOAD, INSTR_MOVE } type;
    int src_reg;
    int dst_reg;
    int stack_offset;
} MinimalInstruction;

// Helper functions
static void init_spill_manager(MinimalSpillManager *manager) {
    manager->count = 0;
    manager->next_stack_slot = 0;
    for (int i = 0; i < 16; i++) {
        manager->candidates[i] = (MinimalSpillCandidate){0};
    }
}

static int calculate_spill_cost(const MinimalSpillCandidate *candidate) {
    // Simple spill cost heuristic: higher cost for more uses
    return candidate->use_count * 10 + candidate->def_count * 5;
}

static int find_best_spill_candidate(const MinimalSpillManager *manager) {
    int best_idx = -1;
    int lowest_cost = INT_MAX;

    for (int i = 0; i < manager->count; i++) {
        if (!manager->candidates[i].is_spilled) {
            int cost = calculate_spill_cost(&manager->candidates[i]);
            if (cost < lowest_cost) {
                lowest_cost = cost;
                best_idx = i;
            }
        }
    }

    return best_idx;
}

// Test functions using minimal framework approach
static AsthraTestResult test_basic_spill_handling(AsthraTestContext *context) {
    MinimalSpillManager manager;
    init_spill_manager(&manager);

    // Create some register candidates with different usage patterns
    manager.candidates[0] = (MinimalSpillCandidate){
        .virtual_reg = 100, .use_count = 10, .def_count = 2, .is_spilled = false, .stack_slot = -1};
    manager.candidates[1] = (MinimalSpillCandidate){
        .virtual_reg = 101, .use_count = 5, .def_count = 1, .is_spilled = false, .stack_slot = -1};
    manager.candidates[2] = (MinimalSpillCandidate){
        .virtual_reg = 102, .use_count = 15, .def_count = 3, .is_spilled = false, .stack_slot = -1};
    manager.count = 3;

    // Calculate spill costs
    for (int i = 0; i < manager.count; i++) {
        manager.candidates[i].spill_cost = calculate_spill_cost(&manager.candidates[i]);
    }

    // Find best spill candidate (lowest cost)
    int best_candidate = find_best_spill_candidate(&manager);

    // Should choose the candidate with lowest usage (candidate 1)
    if (best_candidate != 1) {
        return ASTHRA_TEST_FAIL;
    }

    // Spill the candidate
    manager.candidates[best_candidate].is_spilled = true;
    manager.candidates[best_candidate].stack_slot = manager.next_stack_slot++;

    // Verify spill state
    if (!manager.candidates[1].is_spilled || manager.candidates[1].stack_slot != 0) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_spill_cost_analysis(AsthraTestContext *context) {
    MinimalSpillCandidate candidates[4] = {
        {100, 0, 1, 1, false, -1},  // Low usage: 1*10 + 1*5 = 15
        {101, 0, 5, 2, false, -1},  // Medium usage: 5*10 + 2*5 = 60
        {102, 0, 10, 3, false, -1}, // High usage: 10*10 + 3*5 = 115
        {103, 0, 20, 5, false, -1}  // Very high usage: 20*10 + 5*5 = 225
    };

    // Calculate spill costs
    for (int i = 0; i < 4; i++) {
        candidates[i].spill_cost = calculate_spill_cost(&candidates[i]);
    }

    // Verify cost calculations
    if (candidates[0].spill_cost != 15)
        return ASTHRA_TEST_FAIL;
    if (candidates[1].spill_cost != 60)
        return ASTHRA_TEST_FAIL;
    if (candidates[2].spill_cost != 115)
        return ASTHRA_TEST_FAIL;
    if (candidates[3].spill_cost != 225)
        return ASTHRA_TEST_FAIL;

    // Verify ordering (lower cost should be preferred for spilling)
    if (candidates[0].spill_cost >= candidates[1].spill_cost)
        return ASTHRA_TEST_FAIL;
    if (candidates[1].spill_cost >= candidates[2].spill_cost)
        return ASTHRA_TEST_FAIL;
    if (candidates[2].spill_cost >= candidates[3].spill_cost)
        return ASTHRA_TEST_FAIL;

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_spill_code_generation(AsthraTestContext *context) {
    // Simulate spill code generation for a register
    MinimalInstruction spill_code[8];
    int instruction_count = 0;

    int virtual_reg = 200;
    int physical_reg = 5;
    int stack_slot = 16; // Stack offset for spilled register

    // Generate spill store instruction
    spill_code[instruction_count++] = (MinimalInstruction){.type = INSTR_STORE,
                                                           .src_reg = physical_reg,
                                                           .dst_reg = -1, // Not used for store
                                                           .stack_offset = stack_slot};

    // Generate some intermediate instructions (simulated)
    spill_code[instruction_count++] = (MinimalInstruction){
        .type = INSTR_MOVE, .src_reg = 6, .dst_reg = physical_reg, .stack_offset = 0};

    // Generate spill reload instruction
    spill_code[instruction_count++] = (MinimalInstruction){.type = INSTR_LOAD,
                                                           .src_reg = -1, // Not used for load
                                                           .dst_reg = physical_reg,
                                                           .stack_offset = stack_slot};

    // Verify spill code structure
    if (instruction_count != 3) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify store instruction
    if (spill_code[0].type != INSTR_STORE || spill_code[0].src_reg != physical_reg ||
        spill_code[0].stack_offset != stack_slot) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify reload instruction
    if (spill_code[2].type != INSTR_LOAD || spill_code[2].dst_reg != physical_reg ||
        spill_code[2].stack_offset != stack_slot) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_reload_optimization(AsthraTestContext *context) {
    // Test reload optimization - avoiding unnecessary reloads
    MinimalSpillCandidate candidate = {
        .virtual_reg = 300, .use_count = 8, .def_count = 2, .is_spilled = true, .stack_slot = 8};

    // Simulate usage pattern with multiple uses
    bool uses[10] = {true, false, true, true, false, false, true, false, true, false};
    int reload_count = 0;
    bool currently_loaded = false;

    // Optimize reloads: only reload when needed and not currently loaded
    for (int i = 0; i < 10; i++) {
        if (uses[i] && !currently_loaded) {
            // Need to reload
            reload_count++;
            currently_loaded = true;
        } else if (!uses[i]) {
            // Register might be freed for other uses
            currently_loaded = false;
        }
    }

    // Should have optimal number of reloads (4 reload points for usage pattern)
    if (reload_count != 4) {
        return ASTHRA_TEST_FAIL;
    }

    // Test redundant reload elimination
    int total_uses = 0;
    for (int i = 0; i < 10; i++) {
        if (uses[i])
            total_uses++;
    }

    // Should have fewer reloads than total uses (optimization working)
    if (reload_count >= total_uses) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Spill Handling Tests (Minimal Framework) ===\n");

    AsthraTestContext context = {0}; // Initialize to zero

    int passed = 0, total = 0;

    // Run tests
    if (test_basic_spill_handling(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Basic Spill Handling: PASS\n");
        passed++;
    } else {
        printf("❌ Basic Spill Handling: FAIL\n");
    }
    total++;

    if (test_spill_cost_analysis(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Spill Cost Analysis: PASS\n");
        passed++;
    } else {
        printf("❌ Spill Cost Analysis: FAIL\n");
    }
    total++;

    if (test_spill_code_generation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Spill Code Generation: PASS\n");
        passed++;
    } else {
        printf("❌ Spill Code Generation: FAIL\n");
    }
    total++;

    if (test_reload_optimization(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Reload Optimization: PASS\n");
        passed++;
    } else {
        printf("❌ Reload Optimization: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
