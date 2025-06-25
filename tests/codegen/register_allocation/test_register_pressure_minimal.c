#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_pressure_analysis", __FILE__, __LINE__, "Test basic register pressure analysis",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_pressure_hotspots", __FILE__, __LINE__, "Test register pressure hotspot detection",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_pressure_reduction", __FILE__, __LINE__, "Test register pressure reduction strategies",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_spill_decisions", __FILE__, __LINE__, "Test spill decisions based on pressure",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}};

// Minimal register pressure tracking
typedef struct {
    int program_point;
    int active_registers[16]; // List of active virtual registers
    int active_count;
    int pressure_level; // Current register pressure
} MinimalPressurePoint;

typedef struct {
    MinimalPressurePoint points[32];
    int point_count;
    int max_pressure;
    int max_pressure_point;
    int available_physical_regs;
} MinimalPressureAnalyzer;

// Helper functions
static void init_pressure_analyzer(MinimalPressureAnalyzer *analyzer, int physical_regs) {
    analyzer->point_count = 0;
    analyzer->max_pressure = 0;
    analyzer->max_pressure_point = -1;
    analyzer->available_physical_regs = physical_regs;

    for (int i = 0; i < 32; i++) {
        analyzer->points[i].program_point = -1;
        analyzer->points[i].active_count = 0;
        analyzer->points[i].pressure_level = 0;

        for (int j = 0; j < 16; j++) {
            analyzer->points[i].active_registers[j] = -1;
        }
    }
}

static void add_register_to_active(MinimalPressurePoint *point, int virtual_reg) {
    // Check if already active
    for (int i = 0; i < point->active_count; i++) {
        if (point->active_registers[i] == virtual_reg) {
            return; // Already active
        }
    }

    // Add to active list
    if (point->active_count < 16) {
        point->active_registers[point->active_count++] = virtual_reg;
        point->pressure_level = point->active_count;
    }
}

static void remove_register_from_active(MinimalPressurePoint *point, int virtual_reg) {
    for (int i = 0; i < point->active_count; i++) {
        if (point->active_registers[i] == virtual_reg) {
            // Shift remaining registers down
            for (int j = i; j < point->active_count - 1; j++) {
                point->active_registers[j] = point->active_registers[j + 1];
            }
            point->active_count--;
            point->pressure_level = point->active_count;
            point->active_registers[point->active_count] = -1;
            break;
        }
    }
}

static void record_pressure_point(MinimalPressureAnalyzer *analyzer, int program_point) {
    if (analyzer->point_count < 32) {
        MinimalPressurePoint *point = &analyzer->points[analyzer->point_count++];
        point->program_point = program_point;

        // Copy active registers from previous point if any
        if (analyzer->point_count > 1) {
            MinimalPressurePoint *prev = &analyzer->points[analyzer->point_count - 2];
            for (int i = 0; i < prev->active_count; i++) {
                point->active_registers[i] = prev->active_registers[i];
            }
            point->active_count = prev->active_count;
            point->pressure_level = prev->pressure_level;
        }

        // Update max pressure tracking
        if (point->pressure_level > analyzer->max_pressure) {
            analyzer->max_pressure = point->pressure_level;
            analyzer->max_pressure_point = program_point;
        }
    }
}

static void simulate_register_def(MinimalPressureAnalyzer *analyzer, int virtual_reg) {
    if (analyzer->point_count > 0) {
        MinimalPressurePoint *current = &analyzer->points[analyzer->point_count - 1];
        add_register_to_active(current, virtual_reg);

        // Update max pressure tracking
        if (current->pressure_level > analyzer->max_pressure) {
            analyzer->max_pressure = current->pressure_level;
            analyzer->max_pressure_point = current->program_point;
        }
    }
}

static void simulate_register_use_end(MinimalPressureAnalyzer *analyzer, int virtual_reg) {
    if (analyzer->point_count > 0) {
        MinimalPressurePoint *current = &analyzer->points[analyzer->point_count - 1];
        remove_register_from_active(current, virtual_reg);
    }
}

// Test functions using minimal framework approach
static AsthraTestResult test_basic_pressure_analysis(AsthraTestContext *context) {
    MinimalPressureAnalyzer analyzer;
    init_pressure_analyzer(&analyzer, 8); // 8 physical registers available

    // Simulate a simple sequence with increasing pressure
    record_pressure_point(&analyzer, 0);   // Point 0: start
    simulate_register_def(&analyzer, 100); // r100 becomes active

    record_pressure_point(&analyzer, 1);   // Point 1
    simulate_register_def(&analyzer, 101); // r101 becomes active

    record_pressure_point(&analyzer, 2);   // Point 2
    simulate_register_def(&analyzer, 102); // r102 becomes active

    record_pressure_point(&analyzer, 3);       // Point 3
    simulate_register_use_end(&analyzer, 100); // r100 dies

    // Check pressure progression
    if (analyzer.points[0].pressure_level != 1) {
        return ASTHRA_TEST_FAIL; // One register active after r100 def
    }

    if (analyzer.points[1].pressure_level != 2) {
        return ASTHRA_TEST_FAIL; // Two registers active after r101 def
    }

    if (analyzer.points[2].pressure_level != 3) {
        return ASTHRA_TEST_FAIL; // Three registers active after r102 def
    }

    if (analyzer.points[3].pressure_level != 2) {
        return ASTHRA_TEST_FAIL; // Back to two after r100 dies
    }

    // Check max pressure tracking
    if (analyzer.max_pressure != 3 || analyzer.max_pressure_point != 2) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_pressure_hotspots(AsthraTestContext *context) {
    MinimalPressureAnalyzer analyzer;
    init_pressure_analyzer(&analyzer, 4); // Only 4 physical registers

    // Create a scenario with pressure spikes
    record_pressure_point(&analyzer, 0);
    simulate_register_def(&analyzer, 100);
    simulate_register_def(&analyzer, 101);

    record_pressure_point(&analyzer, 1); // Low pressure region

    record_pressure_point(&analyzer, 2); // Start of hotspot
    simulate_register_def(&analyzer, 102);
    simulate_register_def(&analyzer, 103);
    simulate_register_def(&analyzer, 104); // Exceeds physical registers

    record_pressure_point(&analyzer, 3);   // Peak hotspot
    simulate_register_def(&analyzer, 105); // Even more pressure

    record_pressure_point(&analyzer, 4); // End of hotspot
    simulate_register_use_end(&analyzer, 100);
    simulate_register_use_end(&analyzer, 101);
    simulate_register_use_end(&analyzer, 102);

    // Find hotspots (pressure > available physical registers)
    int hotspot_count = 0;
    for (int i = 0; i < analyzer.point_count; i++) {
        if (analyzer.points[i].pressure_level > analyzer.available_physical_regs) {
            hotspot_count++;
        }
    }

    // Should have detected hotspots
    if (hotspot_count == 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Max pressure should exceed available registers
    if (analyzer.max_pressure <= analyzer.available_physical_regs) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_pressure_reduction(AsthraTestContext *context) {
    MinimalPressureAnalyzer analyzer;
    init_pressure_analyzer(&analyzer, 6); // 6 physical registers

    // Simulate high pressure scenario
    record_pressure_point(&analyzer, 0);
    for (int i = 0; i < 8; i++) {
        simulate_register_def(&analyzer, 100 + i); // Add 8 virtual registers
    }

    int initial_pressure = analyzer.points[analyzer.point_count - 1].pressure_level;

    // Simulate pressure reduction by ending some live ranges
    record_pressure_point(&analyzer, 1);
    simulate_register_use_end(&analyzer, 100);
    simulate_register_use_end(&analyzer, 101);
    simulate_register_use_end(&analyzer, 102);

    int reduced_pressure = analyzer.points[analyzer.point_count - 1].pressure_level;

    // Pressure should have reduced
    if (reduced_pressure >= initial_pressure) {
        return ASTHRA_TEST_FAIL;
    }

    // Should reduce by exactly 3 (the registers we ended)
    if (initial_pressure - reduced_pressure != 3) {
        return ASTHRA_TEST_FAIL;
    }

    // Reduced pressure should be within available registers
    if (reduced_pressure > analyzer.available_physical_regs) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_spill_decisions(AsthraTestContext *context) {
    MinimalPressureAnalyzer analyzer;
    init_pressure_analyzer(&analyzer, 4); // 4 physical registers

    // Create scenario requiring spill decisions
    record_pressure_point(&analyzer, 0);
    simulate_register_def(&analyzer, 100);
    simulate_register_def(&analyzer, 101);
    simulate_register_def(&analyzer, 102);
    simulate_register_def(&analyzer, 103);
    simulate_register_def(&analyzer, 104); // 5 registers, need to spill 1

    int current_pressure = analyzer.points[analyzer.point_count - 1].pressure_level;
    int spill_needed = current_pressure - analyzer.available_physical_regs;

    // Should need exactly 1 spill
    if (spill_needed != 1) {
        return ASTHRA_TEST_FAIL;
    }

    // Simulate spill decision (remove register with lowest priority)
    // For simplicity, spill the first register
    record_pressure_point(&analyzer, 1);
    simulate_register_use_end(&analyzer, 100); // Spill r100

    int post_spill_pressure = analyzer.points[analyzer.point_count - 1].pressure_level;

    // Pressure should now be within limits
    if (post_spill_pressure > analyzer.available_physical_regs) {
        return ASTHRA_TEST_FAIL;
    }

    // Should be exactly equal to available registers
    if (post_spill_pressure != analyzer.available_physical_regs) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Register Pressure Analysis Tests (Minimal Framework) ===\n");

    AsthraTestContext context = {0}; // Initialize to zero
    int passed = 0, total = 0;

    // Run tests
    if (test_basic_pressure_analysis(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Basic Pressure Analysis: PASS\n");
        passed++;
    } else {
        printf("❌ Basic Pressure Analysis: FAIL\n");
    }
    total++;

    if (test_pressure_hotspots(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Pressure Hotspots: PASS\n");
        passed++;
    } else {
        printf("❌ Pressure Hotspots: FAIL\n");
    }
    total++;

    if (test_pressure_reduction(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Pressure Reduction: PASS\n");
        passed++;
    } else {
        printf("❌ Pressure Reduction: FAIL\n");
    }
    total++;

    if (test_spill_decisions(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Spill Decisions: PASS\n");
        passed++;
    } else {
        printf("❌ Spill Decisions: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
