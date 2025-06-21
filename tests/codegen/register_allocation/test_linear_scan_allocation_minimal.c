#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_linear_scan_basic", __FILE__, __LINE__, "Test basic linear scan allocation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_linear_scan_overlapping", __FILE__, __LINE__, "Test linear scan with overlapping lifetimes", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_linear_scan_pressure", __FILE__, __LINE__, "Test linear scan under register pressure", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_linear_scan_holes", __FILE__, __LINE__, "Test linear scan with lifetime holes", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

// Minimal live interval structure
typedef struct {
    int virtual_reg;
    int start;
    int end;
    int physical_reg;  // -1 if unassigned
} MinimalInterval;

// Minimal register pool
typedef struct {
    bool in_use[8];  // 8 physical registers
    int available_count;
} MinimalRegisterPool;

// Helper functions
static void init_register_pool(MinimalRegisterPool* pool) {
    for (int i = 0; i < 8; i++) {
        pool->in_use[i] = false;
    }
    pool->available_count = 8;
}

static int allocate_register(MinimalRegisterPool* pool) {
    for (int i = 0; i < 8; i++) {
        if (!pool->in_use[i]) {
            pool->in_use[i] = true;
            pool->available_count--;
            return i;
        }
    }
    return -1;  // No registers available
}

static void free_register(MinimalRegisterPool* pool, int reg) {
    if (reg >= 0 && reg < 8 && pool->in_use[reg]) {
        pool->in_use[reg] = false;
        pool->available_count++;
    }
}

static bool intervals_overlap(const MinimalInterval* a, const MinimalInterval* b) {
    return !(a->end <= b->start || b->end <= a->start);
}

// Test functions using minimal framework approach
static AsthraTestResult test_linear_scan_basic(AsthraTestContext* context) {
    // Create simple non-overlapping intervals
    MinimalInterval intervals[4] = {
        {0, 0, 10, -1},   // Register 0: lifetime 0-10
        {1, 12, 20, -1},  // Register 1: lifetime 12-20 (after reg 0)
        {2, 22, 30, -1},  // Register 2: lifetime 22-30 (after reg 1)
        {3, 32, 40, -1}   // Register 3: lifetime 32-40 (after reg 2)
    };
    
    MinimalRegisterPool pool;
    init_register_pool(&pool);
    
    // Sort intervals by start time (linear scan requirement)
    // Already sorted in this case
    
    // Simulate linear scan allocation
    for (int i = 0; i < 4; i++) {
        // Free registers for expired intervals
        for (int j = 0; j < i; j++) {
            if (intervals[j].physical_reg >= 0 && intervals[j].end <= intervals[i].start) {
                free_register(&pool, intervals[j].physical_reg);
            }
        }
        
        // Allocate register for current interval
        intervals[i].physical_reg = allocate_register(&pool);
    }
    
    // All intervals should be allocated since they don't overlap
    for (int i = 0; i < 4; i++) {
        if (intervals[i].physical_reg < 0) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Non-overlapping intervals should reuse the same register
    if (intervals[0].physical_reg != intervals[1].physical_reg ||
        intervals[1].physical_reg != intervals[2].physical_reg ||
        intervals[2].physical_reg != intervals[3].physical_reg) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_linear_scan_overlapping(AsthraTestContext* context) {
    // Create overlapping intervals
    MinimalInterval intervals[4] = {
        {0, 0, 20, -1},   // Register 0: lifetime 0-20
        {1, 5, 25, -1},   // Register 1: lifetime 5-25 (overlaps with reg 0)
        {2, 10, 30, -1},  // Register 2: lifetime 10-30 (overlaps with reg 0 & 1)
        {3, 15, 35, -1}   // Register 3: lifetime 15-35 (overlaps with all)
    };
    
    MinimalRegisterPool pool;
    init_register_pool(&pool);
    
    // Track active intervals for proper linear scan
    int active_intervals[4];
    int active_count = 0;
    
    for (int i = 0; i < 4; i++) {
        // Remove expired intervals from active list
        int new_active_count = 0;
        for (int j = 0; j < active_count; j++) {
            int active_idx = active_intervals[j];
            if (intervals[active_idx].end > intervals[i].start) {
                active_intervals[new_active_count++] = active_idx;
            } else {
                // Free the register
                free_register(&pool, intervals[active_idx].physical_reg);
            }
        }
        active_count = new_active_count;
        
        // Allocate register for current interval
        intervals[i].physical_reg = allocate_register(&pool);
        
        if (intervals[i].physical_reg >= 0) {
            active_intervals[active_count++] = i;
        }
    }
    
    // All intervals should be allocated (we have enough registers)
    for (int i = 0; i < 4; i++) {
        if (intervals[i].physical_reg < 0) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Overlapping intervals should have different registers
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (intervals_overlap(&intervals[i], &intervals[j]) && 
                intervals[i].physical_reg == intervals[j].physical_reg) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_linear_scan_pressure(AsthraTestContext* context) {
    // Create more overlapping intervals than available registers
    MinimalInterval intervals[10];
    
    // All intervals overlap significantly
    for (int i = 0; i < 10; i++) {
        intervals[i] = (MinimalInterval){i, i * 2, i * 2 + 20, -1};
    }
    
    MinimalRegisterPool pool;
    init_register_pool(&pool);
    
    int allocated_count = 0;
    int spilled_count = 0;
    
    // Track active intervals
    int active_intervals[10];
    int active_count = 0;
    
    for (int i = 0; i < 10; i++) {
        // Remove expired intervals from active list
        int new_active_count = 0;
        for (int j = 0; j < active_count; j++) {
            int active_idx = active_intervals[j];
            if (intervals[active_idx].end > intervals[i].start) {
                active_intervals[new_active_count++] = active_idx;
            } else {
                free_register(&pool, intervals[active_idx].physical_reg);
            }
        }
        active_count = new_active_count;
        
        // Try to allocate register for current interval
        intervals[i].physical_reg = allocate_register(&pool);
        
        if (intervals[i].physical_reg >= 0) {
            active_intervals[active_count++] = i;
            allocated_count++;
        } else {
            spilled_count++;
        }
    }
    
    // Should not allocate more than available registers
    if (allocated_count > 8) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Should have spilled some intervals due to pressure
    if (spilled_count == 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Total should equal number of intervals
    if (allocated_count + spilled_count != 10) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_linear_scan_holes(AsthraTestContext* context) {
    // Create intervals with gaps that can be exploited
    MinimalInterval intervals[4] = {
        {0, 0, 10, -1},   // Register 0: lifetime 0-10
        {1, 15, 25, -1},  // Register 1: lifetime 15-25 (gap from 10-15)
        {2, 5, 20, -1},   // Register 2: lifetime 5-20 (overlaps both)
        {3, 12, 18, -1}   // Register 3: lifetime 12-18 (fits in gap)
    };
    
    MinimalRegisterPool pool;
    init_register_pool(&pool);
    
    // Sort by start time (required for linear scan)
    // intervals[0]: 0-10, intervals[2]: 5-20, intervals[3]: 12-18, intervals[1]: 15-25
    MinimalInterval sorted[4] = {intervals[0], intervals[2], intervals[3], intervals[1]};
    
    // Track active intervals
    int active_intervals[4];
    int active_count = 0;
    
    for (int i = 0; i < 4; i++) {
        // Remove expired intervals from active list
        int new_active_count = 0;
        for (int j = 0; j < active_count; j++) {
            int active_idx = active_intervals[j];
            if (sorted[active_idx].end > sorted[i].start) {
                active_intervals[new_active_count++] = active_idx;
            } else {
                free_register(&pool, sorted[active_idx].physical_reg);
            }
        }
        active_count = new_active_count;
        
        // Allocate register for current interval
        sorted[i].physical_reg = allocate_register(&pool);
        
        if (sorted[i].physical_reg >= 0) {
            active_intervals[active_count++] = i;
        }
    }
    
    // All intervals should be allocated
    for (int i = 0; i < 4; i++) {
        if (sorted[i].physical_reg < 0) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // intervals[0] (0-10) and intervals[1] (15-25) cannot share a register due to 
    // overlapping interval sorted[2] = intervals[3] (12-18) which overlaps with intervals[1]
    // So sorted[0] and sorted[3] should have different registers
    if (sorted[0].physical_reg == sorted[3].physical_reg) {
        return ASTHRA_TEST_FAIL;  // They should have different registers
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Linear Scan Allocation Tests (Minimal Framework) ===\n");
    
    AsthraTestContext context = {0};  // Initialize to zero
    
    int passed = 0, total = 0;
    
    // Run tests
    if (test_linear_scan_basic(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Basic Linear Scan: PASS\n");
        passed++;
    } else {
        printf("❌ Basic Linear Scan: FAIL\n");
    }
    total++;
    
    if (test_linear_scan_overlapping(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Overlapping Lifetimes: PASS\n");
        passed++;
    } else {
        printf("❌ Overlapping Lifetimes: FAIL\n");
    }
    total++;
    
    if (test_linear_scan_pressure(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Register Pressure: PASS\n");
        passed++;
    } else {
        printf("❌ Register Pressure: FAIL\n");
    }
    total++;
    
    if (test_linear_scan_holes(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Lifetime Holes: PASS\n");
        passed++;
    } else {
        printf("❌ Lifetime Holes: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
} 
