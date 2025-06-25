#include "test_advanced_optimization_pipeline_common.h"

// Test multi-pass optimization with sample instructions and loops
AsthraTestResult test_multi_pass_optimization(AsthraTestContext *context) {
    MinimalOptimizationContext ctx;
    init_optimization_context(&ctx);

    // Add sample instructions for optimization
    add_instruction(&ctx, 0x1000, "mov $5, %eax");
    add_instruction(&ctx, 0x1004, "add $3, %eax"); // Constant folding candidate
    add_instruction(&ctx, 0x1008, "mov %eax, %ebx");
    add_instruction(&ctx, 0x100C, "add $3, %ebx"); // Common subexpression
    add_instruction(&ctx, 0x1010, "mul $2, %ecx"); // Strength reduction candidate
    add_instruction(&ctx, 0x1014, "nop");          // Dead code candidate
    add_instruction(&ctx, 0x1018, "call func");    // Inlining candidate
    add_instruction(&ctx, 0x101C, "div $4, %edx"); // Strength reduction candidate

    // Add a simple loop
    add_loop(&ctx, 0x1020, 0x1040, 3);

    // Run optimization pipeline
    bool converged = run_optimization_pipeline(&ctx, 5);

    // Verify that passes were executed
    for (int i = 0; i < ctx.pass_count; i++) {
        if (ctx.passes[i].execution_count == 0) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Verify that optimizations were found
    if (ctx.total_optimizations == 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Check specific optimization results
    bool found_constant = false, found_redundant = false, found_dead = false;

    for (int i = 0; i < ctx.instruction_count; i++) {
        const MinimalInstruction *inst = &ctx.instructions[i];
        if (inst->is_constant)
            found_constant = true;
        if (inst->is_redundant)
            found_redundant = true;
        if (inst->is_dead)
            found_dead = true;
    }

    // At least some optimizations should have been applied
    if (!found_constant && !found_redundant) {
        return ASTHRA_TEST_FAIL;
    }

    // Pipeline should eventually converge or reach iteration limit
    if (!converged && ctx.total_optimizations == 0) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner for multi-pass optimization
int main(void) {
    printf("=== Multi-Pass Optimization Tests ===\n");

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    // Run multi-pass optimization test
    if (test_multi_pass_optimization(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Multi-Pass Optimization: PASS\n");
        passed++;
    } else {
        printf("❌ Multi-Pass Optimization: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
