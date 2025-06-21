#include "test_advanced_optimization_pipeline_common.h"

// Test optimization validation and correctness
AsthraTestResult test_optimization_validation(AsthraTestContext* context) {
    MinimalOptimizationContext ctx;
    init_optimization_context(&ctx);
    
    // Create a test case where we can validate correctness
    add_instruction(&ctx, 0x1000, "mov $100, %eax");
    add_instruction(&ctx, 0x1004, "mov $200, %ebx");
    add_instruction(&ctx, 0x1008, "add %eax, %ebx");    // Result should be 300
    add_instruction(&ctx, 0x100C, "mov %ebx, result");
    
    // Add some optimizable patterns
    add_instruction(&ctx, 0x1010, "mov $100, %ecx");    // Same as first instruction
    add_instruction(&ctx, 0x1014, "add $5, %ecx");      // Should fold - constant with $
    add_instruction(&ctx, 0x1018, "nop");               // Dead code
    add_instruction(&ctx, 0x101C, "nop");               // More dead code
    
    // Mark nop instructions as having zero reference count for dead code elimination
    ctx.instructions[6].reference_count = 0;  // First nop
    ctx.instructions[7].reference_count = 0;  // Second nop
    
    // Store original instruction count
    int original_count = ctx.instruction_count;
    
    // Run optimizations
    run_optimization_pipeline(&ctx, 3);
    
    // Validation 1: No corruption of essential instructions
    bool found_result_store = false;
    for (int i = 0; i < ctx.instruction_count; i++) {
        const MinimalInstruction* inst = &ctx.instructions[i];
        if (strstr(inst->instruction, "result") && !inst->is_dead) {
            found_result_store = true;
        }
    }
    
    if (!found_result_store) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Validation 2: Dead code should be identified
    int dead_instructions = 0;
    for (int i = 0; i < ctx.instruction_count; i++) {
        if (ctx.instructions[i].is_dead) {
            dead_instructions++;
        }
    }
    
    if (dead_instructions == 0) {
        printf("ERROR: No dead instructions found. Expected nop instructions to be marked as dead.\n");
        printf("Total instructions: %d\n", ctx.instruction_count);
        for (int i = 0; i < ctx.instruction_count; i++) {
            printf("  [%d] %s (ref_count=%d, is_dead=%d)\n", 
                   i, ctx.instructions[i].instruction, 
                   ctx.instructions[i].reference_count, 
                   ctx.instructions[i].is_dead);
        }
        return ASTHRA_TEST_FAIL; // Should have found the nop instructions
    }
    
    // Validation 3: Constants should be properly folded
    int constant_instructions = 0;
    for (int i = 0; i < ctx.instruction_count; i++) {
        if (ctx.instructions[i].is_constant) {
            constant_instructions++;
        }
    }
    
    if (constant_instructions == 0) {
        printf("ERROR: No constant folding found. Expected 'add $5, %%ecx' to be marked as constant.\n");
        for (int i = 0; i < ctx.instruction_count; i++) {
            printf("  [%d] %s (is_constant=%d)\n", 
                   i, ctx.instructions[i].instruction, 
                   ctx.instructions[i].is_constant);
        }
        return ASTHRA_TEST_FAIL; // Should have found constant folding opportunities
    }
    
    // Validation 4: All passes should have run without errors
    int passes_executed = 0;
    for (int i = 0; i < ctx.pass_count; i++) {
        const MinimalOptimizationPass* pass = &ctx.passes[i];
        
        if (pass->execution_count > 0) {
            passes_executed++;
        }
        
        // Effectiveness should be reasonable (only check if pass executed)
        if (pass->execution_count > 0 && (pass->effectiveness < 0.0 || pass->effectiveness > 1.0)) {
            printf("ERROR: Pass %s has invalid effectiveness: %f\n", pass->name, pass->effectiveness);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    if (passes_executed == 0) {
        printf("ERROR: No optimization passes executed\n");
        return ASTHRA_TEST_FAIL;
    }
    
    // Validation 5: Pipeline should have made forward progress
    if (ctx.total_optimizations == 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Validation 6: Overall effectiveness should be reasonable
    if (ctx.overall_effectiveness < 0.0 || ctx.overall_effectiveness > 1.0) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner for optimization validation
int main(void) {
    printf("=== Optimization Validation Tests ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    // Run optimization validation test
    if (test_optimization_validation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Optimization Validation: PASS\n");
        passed++;
    } else {
        printf("❌ Optimization Validation: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
} 
