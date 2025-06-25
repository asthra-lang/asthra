#include "test_advanced_optimization_pipeline_common.h"

// Test optimization pipeline creation and setup
AsthraTestResult test_optimization_pipeline_creation(AsthraTestContext *context) {
    MinimalOptimizationContext ctx;
    init_optimization_context(&ctx);

    // Verify all optimization passes were created
    if (ctx.pass_count != OPT_COUNT) {
        return ASTHRA_TEST_FAIL;
    }

    // Check that all passes have valid configurations
    for (int i = 0; i < ctx.pass_count; i++) {
        const MinimalOptimizationPass *pass = &ctx.passes[i];

        // Pass should have a name
        if (!pass->name || strlen(pass->name) == 0) {
            return ASTHRA_TEST_FAIL;
        }

        // Pass should be enabled by default
        if (!pass->enabled) {
            return ASTHRA_TEST_FAIL;
        }

        // Pass should have valid priority
        if (pass->priority <= 0 || pass->priority > OPT_COUNT) {
            return ASTHRA_TEST_FAIL;
        }

        // Pass should not have executed yet
        if (pass->execution_count != 0) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Verify specific passes exist
    bool has_dead_code = false, has_constant_fold = false, has_cse = false;
    for (int i = 0; i < ctx.pass_count; i++) {
        if (ctx.passes[i].type == OPT_DEAD_CODE_ELIMINATION)
            has_dead_code = true;
        if (ctx.passes[i].type == OPT_CONSTANT_FOLDING)
            has_constant_fold = true;
        if (ctx.passes[i].type == OPT_COMMON_SUBEXPRESSION)
            has_cse = true;
    }

    if (!has_dead_code || !has_constant_fold || !has_cse) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner for pipeline creation
int main(void) {
    printf("=== Optimization Pipeline Creation Tests ===\n");

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    // Run pipeline creation test
    if (test_optimization_pipeline_creation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Optimization Pipeline Creation: PASS\n");
        passed++;
    } else {
        printf("❌ Optimization Pipeline Creation: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
