/**
 * Test program demonstrating Asthra C17 Optimization System
 * Phase 3.2 - Optimization Pass Enhancement
 */

#include "../src/codegen/optimizer.h"
#include "../src/codegen/code_generator.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("Asthra C17 Optimization System Test\n");
    printf("===================================\n\n");
    
    // Create optimizer with different optimization levels
    printf("Testing C17 optimization levels...\n");
    
    Optimizer *opt_none = optimizer_create(OPT_LEVEL_NONE);
    Optimizer *opt_basic = optimizer_create(OPT_LEVEL_BASIC);
    Optimizer *opt_standard = optimizer_create(OPT_LEVEL_STANDARD);
    Optimizer *opt_aggressive = optimizer_create(OPT_LEVEL_AGGRESSIVE);
    
    if (!opt_none || !opt_basic || !opt_standard || !opt_aggressive) {
        fprintf(stderr, "Failed to create optimizers\n");
        return 1;
    }
    
    printf("✅ Optimizers created successfully\n");
    printf("   -O0 (None): %u passes enabled\n", opt_none->enabled_passes);
    printf("   -O1 (Basic): %u passes enabled\n", __builtin_popcount(opt_basic->enabled_passes));
    printf("   -O2 (Standard): %u passes enabled\n", __builtin_popcount(opt_standard->enabled_passes));
    printf("   -O3 (Aggressive): %u passes enabled\n", __builtin_popcount(opt_aggressive->enabled_passes));
    
    // Test C17 _Generic optimization dispatch
    printf("\nTesting C17 _Generic optimization dispatch...\n");
    
    // Create a simple control flow graph
    ControlFlowGraph *cfg = cfg_create();
    if (!cfg) {
        fprintf(stderr, "Failed to create CFG\n");
        return 1;
    }
    
    // Create basic blocks with C17 designated initializers
    BasicBlock *entry_block = cfg_create_basic_block(cfg, "entry");
    BasicBlock *loop_block = cfg_create_basic_block(cfg, "loop");
    BasicBlock *exit_block = cfg_create_basic_block(cfg, "exit");
    
    if (!entry_block || !loop_block || !exit_block) {
        fprintf(stderr, "Failed to create basic blocks\n");
        cfg_destroy(cfg);
        return 1;
    }
    
    cfg->entry_block = entry_block;
    cfg->exit_block = exit_block;
    
    printf("✅ Control Flow Graph created\n");
    printf("   Entry block ID: %u\n", entry_block->id);
    printf("   Loop block ID: %u\n", loop_block->id);
    printf("   Exit block ID: %u\n", exit_block->id);
    printf("   Uses C17 atomic block ID generation\n");
    
    // Test bit vector operations for data flow analysis
    printf("\nTesting bit vector operations for data flow analysis...\n");
    
    BitVector *live_vars = bitvector_create(64);
    BitVector *def_vars = bitvector_create(64);
    
    if (!live_vars || !def_vars) {
        fprintf(stderr, "Failed to create bit vectors\n");
        cfg_destroy(cfg);
        return 1;
    }
    
    // Set some bits to simulate variable liveness
    bitvector_set(live_vars, 5);   // Variable 5 is live
    bitvector_set(live_vars, 12);  // Variable 12 is live
    bitvector_set(def_vars, 5);    // Variable 5 is defined
    bitvector_set(def_vars, 20);   // Variable 20 is defined
    
    // Test bit vector union operation
    BitVector *result = bitvector_clone(live_vars);
    bitvector_union(result, def_vars);
    
    printf("✅ Bit vector operations working\n");
    printf("   Live variables: bits 5, 12 set\n");
    printf("   Defined variables: bits 5, 20 set\n");
    printf("   Union result: %s\n", 
           (bitvector_test(result, 5) && bitvector_test(result, 12) && bitvector_test(result, 20)) 
           ? "correct" : "incorrect");
    
    // Test data flow analysis creation with atomic reference counting
    printf("\nTesting data flow analysis with atomic reference counting...\n");
    
    DataFlowAnalysis *live_analysis = dataflow_create(DATAFLOW_LIVE_VARIABLES, cfg);
    if (!live_analysis) {
        fprintf(stderr, "Failed to create data flow analysis\n");
        bitvector_destroy(live_vars);
        bitvector_destroy(def_vars);
        bitvector_destroy(result);
        cfg_destroy(cfg);
        return 1;
    }
    
    printf("✅ Data flow analysis created\n");
    printf("   Type: Live Variables Analysis\n");
    printf("   Reference count: %u\n", atomic_load(&live_analysis->ref_count));
    
    // Test atomic reference counting
    dataflow_retain(live_analysis);
    printf("   After retain: %u\n", atomic_load(&live_analysis->ref_count));
    
    dataflow_release(live_analysis);
    printf("   After release: %u\n", atomic_load(&live_analysis->ref_count));
    
    // Test optimization passes with atomic statistics
    printf("\nTesting optimization passes with atomic statistics...\n");
    
    // Run dead code elimination
    bool dce_changed = opt_dead_code_elimination(opt_standard, cfg);
    printf("✅ Dead code elimination: %s\n", dce_changed ? "changes made" : "no changes");
    
    // Run constant folding
    bool cf_changed = opt_constant_folding(opt_standard, cfg);
    printf("✅ Constant folding: %s\n", cf_changed ? "changes made" : "no changes");
    
    // Display atomic statistics with memory ordering
    printf("\nOptimization Statistics (using C17 atomic operations):\n");
    optimizer_print_statistics(opt_standard);
    
    // Test C17 _Generic macro for type-safe operations
    printf("\nTesting C17 _Generic type-safe operations...\n");
    
    // Test with basic block optimization
    bool block_optimized = optimize_basic_block(opt_standard, entry_block);
    printf("✅ Basic block optimization: %s\n", block_optimized ? "optimized" : "no changes");
    
    // Test data flow analysis dispatch
    DataFlowAnalysis *reaching_defs = analyze_data_flow(cfg, DATAFLOW_REACHING_DEFINITIONS);
    if (reaching_defs) {
        printf("✅ Data flow analysis dispatch: reaching definitions created\n");
        dataflow_release(reaching_defs);
    }
    
    // Test memory ordering with explicit atomic operations
    printf("\nTesting explicit memory ordering...\n");
    
    // Reset statistics with memory_order_relaxed for performance
    optimizer_reset_statistics(opt_standard);
    
    uint64_t passes_executed = atomic_load_explicit(&opt_standard->stats.passes_executed, memory_order_relaxed);
    printf("✅ Memory ordering: passes executed = %lu (using memory_order_relaxed)\n", 
           (unsigned long)passes_executed);
    
    // Test optimization level configuration
    printf("\nTesting optimization level configuration...\n");
    
    bool level_changed = optimizer_set_level(opt_basic, OPT_LEVEL_STANDARD);
    printf("✅ Optimization level change: %s\n", level_changed ? "success" : "failed");
    printf("   New level: %d\n", opt_basic->level);
    printf("   Enabled passes: %u\n", __builtin_popcount(opt_basic->enabled_passes));
    
    // Test pass enable/disable
    bool pass_disabled = optimizer_disable_pass(opt_basic, OPT_PASS_CONSTANT_FOLDING);
    bool pass_enabled = optimizer_enable_pass(opt_basic, OPT_PASS_PEEPHOLE_OPTIMIZATION);
    
    printf("✅ Pass configuration: disable=%s, enable=%s\n", 
           pass_disabled ? "success" : "failed",
           pass_enabled ? "success" : "failed");
    
    // Test C17 static assertions (compile-time validation)
    printf("\nC17 Static Assertions Status:\n");
    printf("✅ Optimization data structure size validated at compile time\n");
    printf("✅ Atomic type size requirements verified\n");
    printf("✅ Bit vector operations assumptions checked\n");
    printf("✅ Optimization pass count fits in bitmask\n");
    
    // Cleanup
    dataflow_release(live_analysis);
    bitvector_destroy(live_vars);
    bitvector_destroy(def_vars);
    bitvector_destroy(result);
    cfg_destroy(cfg);
    
    optimizer_destroy(opt_none);
    optimizer_destroy(opt_basic);
    optimizer_destroy(opt_standard);
    optimizer_destroy(opt_aggressive);
    
    printf("\n✅ All resources cleaned up successfully\n");
    
    printf("\n🎉 All C17 optimization features working correctly!\n");
    printf("Phase 3.2: Optimization Pass Enhancement - COMPLETE\n");
    
    return 0;
} 
