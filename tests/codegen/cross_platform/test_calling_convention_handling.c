/**
 * Test file for calling convention handling
 * Tests calling convention adaptations for different platforms
 */

#include "cross_platform_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_calling_convention_handling", __FILE__, __LINE__, "Test calling convention adaptations",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Test functions
static AsthraTestResult test_calling_convention_handling(AsthraTestContext *context) {
    CrossPlatformContext ctx;
    init_cross_platform_context(&ctx);

    // Test System V ABI (Linux/macOS)
    const char *sysv_params[] = {"int", "char*", "double"};
    add_function_call(&ctx, "test_func_sysv", "int", 3, sysv_params, CALLING_CONV_SYSTEM_V);

    // Test Windows x64 ABI
    const char *win64_params[] = {"int", "char*", "double", "long"};
    add_function_call(&ctx, "test_func_win64", "int", 4, win64_params, CALLING_CONV_WIN64);

    // Test ARM64 AAPCS
    const char *aapcs_params[] = {"int", "char*", "double", "long", "float"};
    add_function_call(&ctx, "test_func_aapcs", "int", 5, aapcs_params, CALLING_CONV_AAPCS64);

    // Verify function calls were created
    if (ctx.function_call_count != 3) {
        return ASTHRA_TEST_FAIL;
    }

    // Check System V calling convention
    const FunctionCall *sysv_call = &ctx.function_calls[0];
    if (sysv_call->calling_conv != CALLING_CONV_SYSTEM_V) {
        return ASTHRA_TEST_FAIL;
    }

    // System V should use rdi, rsi, rdx for first 3 parameters
    if (strcmp(sysv_call->register_usage[0], "rdi") != 0 ||
        strcmp(sysv_call->register_usage[1], "rsi") != 0 ||
        strcmp(sysv_call->register_usage[2], "rdx") != 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Check Windows x64 calling convention
    const FunctionCall *win64_call = &ctx.function_calls[1];
    if (win64_call->calling_conv != CALLING_CONV_WIN64) {
        return ASTHRA_TEST_FAIL;
    }

    // Windows x64 should use rcx, rdx, r8, r9 for first 4 parameters
    if (strcmp(win64_call->register_usage[0], "rcx") != 0 ||
        strcmp(win64_call->register_usage[1], "rdx") != 0 ||
        strcmp(win64_call->register_usage[2], "r8") != 0 ||
        strcmp(win64_call->register_usage[3], "r9") != 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Check ARM64 AAPCS calling convention
    const FunctionCall *aapcs_call = &ctx.function_calls[2];
    if (aapcs_call->calling_conv != CALLING_CONV_AAPCS64) {
        return ASTHRA_TEST_FAIL;
    }

    // ARM64 AAPCS should use x0, x1, x2, x3, x4 for first 5 parameters
    if (strcmp(aapcs_call->register_usage[0], "x0") != 0 ||
        strcmp(aapcs_call->register_usage[1], "x1") != 0 ||
        strcmp(aapcs_call->register_usage[2], "x2") != 0 ||
        strcmp(aapcs_call->register_usage[3], "x3") != 0 ||
        strcmp(aapcs_call->register_usage[4], "x4") != 0) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Calling Convention Handling Tests (Minimal Framework) ===\n");

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    // Run test
    if (test_calling_convention_handling(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Calling Convention Handling: PASS\n");
        passed++;
    } else {
        printf("❌ Calling Convention Handling: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}