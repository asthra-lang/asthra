#include "test_advanced_optimization_pipeline_common.h"

// Test optimization effectiveness metrics and performance validation
AsthraTestResult test_optimization_effectiveness(AsthraTestContext* context) {
    MinimalOptimizationContext ctx;
    init_optimization_context(&ctx);
    
    // Create a scenario with many optimization opportunities
    add_instruction(&ctx, 0x1000, "mov $10, %eax");
    add_instruction(&ctx, 0x1004, "add $5, %eax");      // Constant: 15
    add_instruction(&ctx, 0x1008, "mov $10, %ebx");
    add_instruction(&ctx, 0x100C, "add $5, %ebx");      // Duplicate of above
    add_instruction(&ctx, 0x1010, "mul $2, %ecx");      // Strength reduction
    add_instruction(&ctx, 0x1014, "div $8, %edx");      // Strength reduction
    add_instruction(&ctx, 0x1018, "mov %eax, %esi");
    add_instruction(&ctx, 0x101C, "mov %esi, %edi");    // Chain that could be optimized
    
    // Add loops for unrolling
    add_loop(&ctx, 0x1020, 0x1030, 2);  // Small loop - good for unrolling
    add_loop(&ctx, 0x1040, 0x1080, 100); // Large loop - not good for unrolling
    
    // Run optimization pipeline
    run_optimization_pipeline(&ctx, 3);
    
    // Check effectiveness metrics
    if (ctx.overall_effectiveness < 0.0 || ctx.overall_effectiveness > 1.0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Should have made some optimizations
    if (ctx.total_optimizations == 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Check individual pass effectiveness
    bool has_effective_pass = false;
    for (int i = 0; i < ctx.pass_count; i++) {
        const MinimalOptimizationPass* pass = &ctx.passes[i];
        
        // Effectiveness should be in valid range
        if (pass->effectiveness < 0.0 || pass->effectiveness > 1.0) {
            return ASTHRA_TEST_FAIL;
        }
        
        // At least one pass should be effective
        if (pass->effectiveness > 0.1) {
            has_effective_pass = true;
        }
    }
    
    if (!has_effective_pass) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify that strength reduction was effective (we have 2 candidates)
    MinimalOptimizationPass* strength_pass = &ctx.passes[OPT_STRENGTH_REDUCTION];
    if (strength_pass->effectiveness == 0.0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify that loop unrolling made decisions correctly
    bool found_unrolled = false, found_not_unrolled = false;
    for (int i = 0; i < ctx.loop_count; i++) {
        const MinimalLoop* loop = &ctx.loops[i];
        if (loop->iteration_count <= 4) {
            found_unrolled = true;
        } else {
            found_not_unrolled = true;
        }
    }
    
    if (!found_unrolled || !found_not_unrolled) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner for optimization effectiveness
int main(void) {
    printf("=== Optimization Effectiveness Tests ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    // Run optimization effectiveness test
    if (test_optimization_effectiveness(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Optimization Effectiveness: PASS\n");
        passed++;
    } else {
        printf("❌ Optimization Effectiveness: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
} 
