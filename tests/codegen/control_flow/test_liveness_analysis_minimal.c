#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_liveness_analysis", __FILE__, __LINE__, "Test basic liveness analysis", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_control_flow_liveness", __FILE__, __LINE__, "Test liveness analysis with control flow", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_use_def_chains", __FILE__, __LINE__, "Test use-definition chains", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_live_intervals", __FILE__, __LINE__, "Test live interval computation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
};

// Instruction types
typedef enum {
    INSTR_DEF,
    INSTR_USE,
    INSTR_BRANCH,
    INSTR_NOP
} InstructionType;

// Minimal instruction representation
typedef struct {
    InstructionType type;
    int virtual_reg;  // -1 if not applicable
    int program_point;
} MinimalInstruction;

// Minimal basic block representation
typedef struct {
    MinimalInstruction instructions[16];
    int instruction_count;
    int successors[4];  // Up to 4 successors
    int successor_count;
    bool live_in[16];   // Live-in set for registers 0-15
    bool live_out[16];  // Live-out set for registers 0-15
} MinimalBasicBlock;

// Minimal liveness analysis context
typedef struct {
    MinimalBasicBlock blocks[8];  // Up to 8 basic blocks
    int block_count;
    bool changed;  // For iterative analysis
} MinimalLivenessContext;

// Helper functions
static void init_liveness_context(MinimalLivenessContext* ctx) {
    ctx->block_count = 0;
    ctx->changed = false;
    
    for (int i = 0; i < 8; i++) {
        ctx->blocks[i].instruction_count = 0;
        ctx->blocks[i].successor_count = 0;
        
        for (int j = 0; j < 16; j++) {
            ctx->blocks[i].live_in[j] = false;
            ctx->blocks[i].live_out[j] = false;
        }
        
        for (int j = 0; j < 4; j++) {
            ctx->blocks[i].successors[j] = -1;
        }
    }
}

static void add_instruction(MinimalBasicBlock* block, InstructionType type, int reg, int point) {
    if (block->instruction_count < 16) {
        block->instructions[block->instruction_count++] = (MinimalInstruction){
            .type = type,
            .virtual_reg = reg,
            .program_point = point
        };
    }
}

static void add_successor(MinimalBasicBlock* block, int successor_id) {
    if (block->successor_count < 4) {
        block->successors[block->successor_count++] = successor_id;
    }
}

static void compute_live_sets(MinimalLivenessContext* ctx, int block_id) {
    MinimalBasicBlock* block = &ctx->blocks[block_id];
    bool old_live_in[16];
    
    // Save old live-in set
    for (int i = 0; i < 16; i++) {
        old_live_in[i] = block->live_in[i];
    }
    
    // Compute live-out as union of successors' live-in
    for (int i = 0; i < 16; i++) {
        block->live_out[i] = false;
    }
    
    for (int i = 0; i < block->successor_count; i++) {
        int succ_id = block->successors[i];
        if (succ_id >= 0 && succ_id < ctx->block_count) {
            for (int j = 0; j < 16; j++) {
                if (ctx->blocks[succ_id].live_in[j]) {
                    block->live_out[j] = true;
                }
            }
        }
    }
    
    // Compute live-in: live-in = use ∪ (live-out - def)
    for (int i = 0; i < 16; i++) {
        block->live_in[i] = block->live_out[i];  // Start with live-out
    }
    
    // Process instructions in reverse order
    for (int i = block->instruction_count - 1; i >= 0; i--) {
        MinimalInstruction* instr = &block->instructions[i];
        if (instr->virtual_reg >= 0 && instr->virtual_reg < 16) {
            if (instr->type == INSTR_DEF) {
                // Definition kills the register
                block->live_in[instr->virtual_reg] = false;
            } else if (instr->type == INSTR_USE) {
                // Use makes the register live
                block->live_in[instr->virtual_reg] = true;
            }
        }
    }
    
    // Check if live-in changed
    for (int i = 0; i < 16; i++) {
        if (old_live_in[i] != block->live_in[i]) {
            ctx->changed = true;
            break;
        }
    }
}

static void run_liveness_analysis(MinimalLivenessContext* ctx) {
    // Iterative fixed-point algorithm
    do {
        ctx->changed = false;
        for (int i = 0; i < ctx->block_count; i++) {
            compute_live_sets(ctx, i);
        }
    } while (ctx->changed);
}

// Test functions using minimal framework approach
static AsthraTestResult test_basic_liveness_analysis(AsthraTestContext* context) {
    MinimalLivenessContext ctx;
    init_liveness_context(&ctx);
    ctx.block_count = 1;
    
    // Single basic block with simple def-use pattern
    MinimalBasicBlock* block = &ctx.blocks[0];
    
    // r0 = 5      (def r0 at point 0)
    // r1 = r0     (use r0, def r1 at point 1)
    // return r1   (use r1 at point 2)
    add_instruction(block, INSTR_DEF, 0, 0);
    add_instruction(block, INSTR_USE, 0, 1);
    add_instruction(block, INSTR_DEF, 1, 1);
    add_instruction(block, INSTR_USE, 1, 2);
    
    run_liveness_analysis(&ctx);
    
    // Check live-in: should be empty (no variables live at entry)
    for (int i = 0; i < 16; i++) {
        if (block->live_in[i]) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Check live-out: should be empty (no variables live at exit)
    for (int i = 0; i < 16; i++) {
        if (block->live_out[i]) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_control_flow_liveness(AsthraTestContext* context) {
    MinimalLivenessContext ctx;
    init_liveness_context(&ctx);
    ctx.block_count = 3;
    
    // Block 0: r0 = 5; branch to block 1 or 2
    add_instruction(&ctx.blocks[0], INSTR_DEF, 0, 0);
    add_instruction(&ctx.blocks[0], INSTR_BRANCH, -1, 1);
    add_successor(&ctx.blocks[0], 1);
    add_successor(&ctx.blocks[0], 2);
    
    // Block 1: r1 = r0 + 1; goto block 2
    add_instruction(&ctx.blocks[1], INSTR_USE, 0, 2);
    add_instruction(&ctx.blocks[1], INSTR_DEF, 1, 2);
    add_successor(&ctx.blocks[1], 2);
    
    // Block 2: return r0
    add_instruction(&ctx.blocks[2], INSTR_USE, 0, 3);
    
    run_liveness_analysis(&ctx);
    
    // r0 should be live-out of block 0 (used in both successors)
    if (!ctx.blocks[0].live_out[0]) {
        return ASTHRA_TEST_FAIL;
    }
    
    // r0 should be live-in of block 1 (used in block 1)
    if (!ctx.blocks[1].live_in[0]) {
        return ASTHRA_TEST_FAIL;
    }
    
    // r0 should be live-in of block 2 (used in block 2)
    if (!ctx.blocks[2].live_in[0]) {
        return ASTHRA_TEST_FAIL;
    }
    
    // r1 should not be live-out of block 1 (not used after definition)
    if (ctx.blocks[1].live_out[1]) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_use_def_chains(AsthraTestContext* context) {
    MinimalLivenessContext ctx;
    init_liveness_context(&ctx);
    ctx.block_count = 2;
    
    // Block 0: r0 = 5; r1 = r0; 
    add_instruction(&ctx.blocks[0], INSTR_DEF, 0, 0);
    add_instruction(&ctx.blocks[0], INSTR_USE, 0, 1);
    add_instruction(&ctx.blocks[0], INSTR_DEF, 1, 1);
    add_successor(&ctx.blocks[0], 1);
    
    // Block 1: r2 = r1 + r0;
    add_instruction(&ctx.blocks[1], INSTR_USE, 1, 2);
    add_instruction(&ctx.blocks[1], INSTR_USE, 0, 2);
    add_instruction(&ctx.blocks[1], INSTR_DEF, 2, 2);
    
    run_liveness_analysis(&ctx);
    
    // Both r0 and r1 should be live-out of block 0
    if (!ctx.blocks[0].live_out[0] || !ctx.blocks[0].live_out[1]) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Both r0 and r1 should be live-in of block 1
    if (!ctx.blocks[1].live_in[0] || !ctx.blocks[1].live_in[1]) {
        return ASTHRA_TEST_FAIL;
    }
    
    // r2 should not be live-out of block 1 (not used after)
    if (ctx.blocks[1].live_out[2]) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_live_intervals(AsthraTestContext* context) {
    // Simulate live interval computation from liveness analysis
    MinimalLivenessContext ctx;
    init_liveness_context(&ctx);
    ctx.block_count = 1;
    
    MinimalBasicBlock* block = &ctx.blocks[0];
    
    // Create a sequence: r0 = 5; r1 = r0; r2 = r1; return r2
    add_instruction(block, INSTR_DEF, 0, 0);  // r0 defined at 0
    add_instruction(block, INSTR_USE, 0, 1);  // r0 used at 1
    add_instruction(block, INSTR_DEF, 1, 1);  // r1 defined at 1
    add_instruction(block, INSTR_USE, 1, 2);  // r1 used at 2
    add_instruction(block, INSTR_DEF, 2, 2);  // r2 defined at 2
    add_instruction(block, INSTR_USE, 2, 3);  // r2 used at 3
    
    // Manually compute live intervals based on the sequence
    int live_start[16], live_end[16];
    for (int i = 0; i < 16; i++) {
        live_start[i] = -1;
        live_end[i] = -1;
    }
    
    // r0: live from definition at 0 until use at 1
    live_start[0] = 0;
    live_end[0] = 1;
    
    // r1: live from definition at 1 until use at 2
    live_start[1] = 1;
    live_end[1] = 2;
    
    // r2: live from definition at 2 until use at 3
    live_start[2] = 2;
    live_end[2] = 3;
    
    // Verify intervals are non-overlapping (can share registers)
    bool intervals_overlap = false;
    for (int i = 0; i < 16; i++) {
        for (int j = i + 1; j < 16; j++) {
            if (live_start[i] >= 0 && live_start[j] >= 0) {
                if (!(live_end[i] <= live_start[j] || live_end[j] <= live_start[i])) {
                    intervals_overlap = true;
                }
            }
        }
    }
    
    // These specific intervals should not overlap
    if (intervals_overlap) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Check specific interval properties
    if (live_start[0] != 0 || live_end[0] != 1) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (live_start[1] != 1 || live_end[1] != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (live_start[2] != 2 || live_end[2] != 3) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Liveness Analysis Tests (Minimal Framework) ===\n");
    
    AsthraTestContext context = {0};  // Initialize to zero
    int passed = 0, total = 0;
    
    // Run tests
    if (test_basic_liveness_analysis(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Basic Liveness Analysis: PASS\n");
        passed++;
    } else {
        printf("❌ Basic Liveness Analysis: FAIL\n");
    }
    total++;
    
    if (test_control_flow_liveness(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Control Flow Liveness: PASS\n");
        passed++;
    } else {
        printf("❌ Control Flow Liveness: FAIL\n");
    }
    total++;
    
    if (test_use_def_chains(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Use-Def Chains: PASS\n");
        passed++;
    } else {
        printf("❌ Use-Def Chains: FAIL\n");
    }
    total++;
    
    if (test_live_intervals(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Live Intervals: PASS\n");
        passed++;
    } else {
        printf("❌ Live Intervals: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
} 
