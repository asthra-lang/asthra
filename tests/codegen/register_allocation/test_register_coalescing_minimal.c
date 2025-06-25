#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_register_coalescing", __FILE__, __LINE__, "Test basic register coalescing",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_coalescing_conflicts", __FILE__, __LINE__, "Test coalescing conflict detection",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_move_elimination", __FILE__, __LINE__, "Test move instruction elimination",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_conservative_coalescing", __FILE__, __LINE__, "Test conservative coalescing strategy",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Minimal move instruction representation
typedef struct {
    int src_reg;
    int dst_reg;
    bool can_coalesce;
    bool is_eliminated;
} MinimalMoveInstr;

// Minimal coalescing data structures
typedef struct {
    int virtual_reg;
    int coalesced_with; // -1 if not coalesced
    bool interferes_with[16];
} MinimalRegisterInfo;

typedef struct {
    MinimalRegisterInfo registers[16];
    int register_count;
    MinimalMoveInstr moves[32];
    int move_count;
} MinimalCoalescingContext;

// Helper functions
static void init_coalescing_context(MinimalCoalescingContext *ctx) {
    ctx->register_count = 0;
    ctx->move_count = 0;

    for (int i = 0; i < 16; i++) {
        ctx->registers[i].virtual_reg = i;
        ctx->registers[i].coalesced_with = -1;
        for (int j = 0; j < 16; j++) {
            ctx->registers[i].interferes_with[j] = false;
        }
    }

    for (int i = 0; i < 32; i++) {
        ctx->moves[i] = (MinimalMoveInstr){-1, -1, false, false};
    }
}

static void add_interference(MinimalCoalescingContext *ctx, int reg1, int reg2) {
    if (reg1 >= 0 && reg1 < 16 && reg2 >= 0 && reg2 < 16 && reg1 != reg2) {
        ctx->registers[reg1].interferes_with[reg2] = true;
        ctx->registers[reg2].interferes_with[reg1] = true;
    }
}

static void add_move_instruction(MinimalCoalescingContext *ctx, int src, int dst) {
    if (ctx->move_count < 32) {
        ctx->moves[ctx->move_count++] = (MinimalMoveInstr){
            .src_reg = src, .dst_reg = dst, .can_coalesce = false, .is_eliminated = false};
    }
}

static bool can_coalesce_registers(MinimalCoalescingContext *ctx, int src, int dst) {
    // Can't coalesce if they interfere
    if (ctx->registers[src].interferes_with[dst]) {
        return false;
    }

    // Can't coalesce if either is already coalesced with another register
    if (ctx->registers[src].coalesced_with != -1 || ctx->registers[dst].coalesced_with != -1) {
        return false;
    }

    // Check that coalescing won't create new interferences
    for (int i = 0; i < 16; i++) {
        if (ctx->registers[src].interferes_with[i] && ctx->registers[dst].interferes_with[i]) {
            return false; // Both interfere with the same register
        }
    }

    return true;
}

static void coalesce_registers(MinimalCoalescingContext *ctx, int src, int dst) {
    ctx->registers[src].coalesced_with = dst;
    ctx->registers[dst].coalesced_with = src;

    // Merge interference information
    for (int i = 0; i < 16; i++) {
        if (ctx->registers[src].interferes_with[i]) {
            ctx->registers[dst].interferes_with[i] = true;
            ctx->registers[i].interferes_with[dst] = true;
        }
    }
}

// Test functions using minimal framework approach
static AsthraTestResult test_basic_register_coalescing(AsthraTestContext *context) {
    MinimalCoalescingContext ctx;
    init_coalescing_context(&ctx);
    ctx.register_count = 4;

    // Create some move instructions
    add_move_instruction(&ctx, 0, 1); // mov r0 -> r1
    add_move_instruction(&ctx, 2, 3); // mov r2 -> r3

    // Add some interferences
    add_interference(&ctx, 0, 2); // r0 and r2 interfere
    add_interference(&ctx, 1, 3); // r1 and r3 interfere

    // First pass: check which moves can be coalesced
    for (int i = 0; i < ctx.move_count; i++) {
        int src = ctx.moves[i].src_reg;
        int dst = ctx.moves[i].dst_reg;

        if (can_coalesce_registers(&ctx, src, dst)) {
            ctx.moves[i].can_coalesce = true;
        }
    }

    // Second pass: actually perform the coalescing
    for (int i = 0; i < ctx.move_count; i++) {
        if (ctx.moves[i].can_coalesce) {
            int src = ctx.moves[i].src_reg;
            int dst = ctx.moves[i].dst_reg;
            coalesce_registers(&ctx, src, dst);
            ctx.moves[i].is_eliminated = true;
        }
    }

    // Both moves should be coalesceable
    if (!ctx.moves[0].can_coalesce || !ctx.moves[1].can_coalesce) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify coalescing happened
    if (ctx.registers[0].coalesced_with != 1 || ctx.registers[1].coalesced_with != 0) {
        return ASTHRA_TEST_FAIL;
    }

    if (ctx.registers[2].coalesced_with != 3 || ctx.registers[3].coalesced_with != 2) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_coalescing_conflicts(AsthraTestContext *context) {
    MinimalCoalescingContext ctx;
    init_coalescing_context(&ctx);
    ctx.register_count = 3;

    // Create move instruction where source and destination interfere
    add_move_instruction(&ctx, 0, 1); // mov r0 -> r1

    // Add interference between source and destination
    add_interference(&ctx, 0, 1); // r0 and r1 interfere

    // Try to coalesce
    int src = ctx.moves[0].src_reg;
    int dst = ctx.moves[0].dst_reg;

    bool can_coalesce = can_coalesce_registers(&ctx, src, dst);

    // Should not be able to coalesce interfering registers
    if (can_coalesce) {
        return ASTHRA_TEST_FAIL;
    }

    // Move should remain uncoalesced
    if (ctx.moves[0].can_coalesce || ctx.moves[0].is_eliminated) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_move_elimination(AsthraTestContext *context) {
    MinimalCoalescingContext ctx;
    init_coalescing_context(&ctx);
    ctx.register_count = 6;

    // Create several move instructions
    add_move_instruction(&ctx, 0, 1); // mov r0 -> r1 (can eliminate)
    add_move_instruction(&ctx, 2, 3); // mov r2 -> r3 (can eliminate)
    add_move_instruction(&ctx, 4, 5); // mov r4 -> r5 (will conflict)

    // Add interference to prevent one coalescing
    add_interference(&ctx, 4, 5); // r4 and r5 interfere

    int eliminated_count = 0;
    int total_moves = ctx.move_count;

    // Process moves for coalescing
    for (int i = 0; i < ctx.move_count; i++) {
        int src = ctx.moves[i].src_reg;
        int dst = ctx.moves[i].dst_reg;

        if (can_coalesce_registers(&ctx, src, dst)) {
            ctx.moves[i].can_coalesce = true;
            coalesce_registers(&ctx, src, dst);
            ctx.moves[i].is_eliminated = true;
            eliminated_count++;
        }
    }

    // Should eliminate 2 out of 3 moves
    if (eliminated_count != 2) {
        return ASTHRA_TEST_FAIL;
    }

    // First two moves should be eliminated
    if (!ctx.moves[0].is_eliminated || !ctx.moves[1].is_eliminated) {
        return ASTHRA_TEST_FAIL;
    }

    // Third move should not be eliminated
    if (ctx.moves[2].is_eliminated) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_conservative_coalescing(AsthraTestContext *context) {
    MinimalCoalescingContext ctx;
    init_coalescing_context(&ctx);
    ctx.register_count = 6;

    // Create a scenario where coalescing one pair affects another
    add_move_instruction(&ctx, 0, 1); // mov r0 -> r1
    add_move_instruction(&ctx, 1, 2); // mov r1 -> r2 (chained move)

    // Add interference pattern
    add_interference(&ctx, 0, 3); // r0 interferes with r3
    add_interference(&ctx, 2, 3); // r2 interferes with r3

    // Process first move
    if (can_coalesce_registers(&ctx, 0, 1)) {
        coalesce_registers(&ctx, 0, 1);
        ctx.moves[0].is_eliminated = true;
    }

    // Check if second move can still be coalesced after first coalescing
    // Since r0 and r1 are now coalesced, and r0 interferes with r3,
    // r1 effectively interferes with r3 too
    bool can_coalesce_second = can_coalesce_registers(&ctx, 1, 2);

    // First move should be coalesceable
    if (!ctx.moves[0].is_eliminated) {
        return ASTHRA_TEST_FAIL;
    }

    // Conservative coalescing should NOT allow the second move
    // After coalescing r0 and r1, both r1 and r2 interfere with r3, so they can't be coalesced
    if (can_coalesce_second) {
        return ASTHRA_TEST_FAIL; // Should NOT be able to coalesce
    }

    // Check that the first coalescing worked correctly
    if (ctx.registers[0].coalesced_with != 1 || ctx.registers[1].coalesced_with != 0) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Register Coalescing Tests (Minimal Framework) ===\n");

    AsthraTestContext context = {0}; // Initialize to zero
    int passed = 0, total = 0;

    // Run tests
    if (test_basic_register_coalescing(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Basic Register Coalescing: PASS\n");
        passed++;
    } else {
        printf("❌ Basic Register Coalescing: FAIL\n");
    }
    total++;

    if (test_coalescing_conflicts(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Coalescing Conflicts: PASS\n");
        passed++;
    } else {
        printf("❌ Coalescing Conflicts: FAIL\n");
    }
    total++;

    if (test_move_elimination(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Move Elimination: PASS\n");
        passed++;
    } else {
        printf("❌ Move Elimination: FAIL\n");
    }
    total++;

    if (test_conservative_coalescing(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Conservative Coalescing: PASS\n");
        passed++;
    } else {
        printf("❌ Conservative Coalescing: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
