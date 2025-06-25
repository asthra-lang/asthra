/**
 * Test file for cross-compilation validation
 * Tests cross-compilation scenarios and platform consistency
 */

#include "cross_platform_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_cross_compilation_validation", __FILE__, __LINE__, "Test cross-compilation validation",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Test functions
static AsthraTestResult test_cross_compilation_validation(AsthraTestContext *context) {
    CrossPlatformContext ctx;
    init_cross_platform_context(&ctx);

    // Enable cross-compilation mode
    ctx.cross_compilation_mode = true;

    // Test compilation for multiple targets
    ctx.current_target = PLATFORM_X86_64_LINUX;
    add_platform_instruction(&ctx, "movq", "$1, %rdi", PLATFORM_X86_64_LINUX);
    add_platform_instruction(&ctx, "call", "exit", PLATFORM_X86_64_LINUX);

    ctx.current_target = PLATFORM_X86_64_WINDOWS;
    add_platform_instruction(&ctx, "mov", "rcx, 1", PLATFORM_X86_64_WINDOWS);
    add_platform_instruction(&ctx, "call", "ExitProcess", PLATFORM_X86_64_WINDOWS);

    ctx.current_target = PLATFORM_ARM64_LINUX;
    add_platform_instruction(&ctx, "mov", "x0, #1", PLATFORM_ARM64_LINUX);
    add_platform_instruction(&ctx, "bl", "exit", PLATFORM_ARM64_LINUX);

    // Verify cross-compilation generated different code for each platform
    if (ctx.instruction_count != 6) {
        return ASTHRA_TEST_FAIL;
    }

    // Validate platform-specific differences
    bool has_linux_x86 = false, has_windows_x86 = false, has_linux_arm = false;

    for (int i = 0; i < ctx.instruction_count; i++) {
        const PlatformInstruction *inst = &ctx.instructions[i];

        if (inst->target_platform == PLATFORM_X86_64_LINUX) {
            has_linux_x86 = true;
            // Linux should use System V calling convention and exit function
            if (strstr(inst->operands, "%rdi") || strcmp(inst->mnemonic, "call") == 0) {
                if (inst->operands && strcmp(inst->operands, "exit") == 0) {
                    // Correct Linux exit call
                } else if (inst->operands && strstr(inst->operands, "%rdi")) {
                    // Correct System V register usage
                } else if (strcmp(inst->operands, "$1, %rdi") == 0) {
                    // Correct immediate to register
                } else {
                    return ASTHRA_TEST_FAIL;
                }
            }
        } else if (inst->target_platform == PLATFORM_X86_64_WINDOWS) {
            has_windows_x86 = true;
            // Windows should use Win64 calling convention and ExitProcess
            if (strstr(inst->operands, "rcx") || strcmp(inst->mnemonic, "call") == 0) {
                if (inst->operands && strcmp(inst->operands, "ExitProcess") == 0) {
                    // Correct Windows exit call
                } else if (inst->operands && strstr(inst->operands, "rcx")) {
                    // Correct Win64 register usage
                } else {
                    return ASTHRA_TEST_FAIL;
                }
            }
        } else if (inst->target_platform == PLATFORM_ARM64_LINUX) {
            has_linux_arm = true;
            // ARM64 should use AAPCS and different instruction syntax
            if (strstr(inst->operands, "x0") || strcmp(inst->mnemonic, "bl") == 0) {
                if (inst->operands && strcmp(inst->operands, "exit") == 0) {
                    // Correct ARM64 exit call
                } else if (inst->operands && strstr(inst->operands, "x0")) {
                    // Correct ARM64 register usage
                } else {
                    return ASTHRA_TEST_FAIL;
                }
            }
        }
    }

    if (!has_linux_x86 || !has_windows_x86 || !has_linux_arm) {
        return ASTHRA_TEST_FAIL;
    }

    // Test calling convention consistency across platforms
    const char *test_params[] = {"int", "char*"};

    add_function_call(&ctx, "cross_func", "void", 2, test_params, CALLING_CONV_SYSTEM_V);
    add_function_call(&ctx, "cross_func", "void", 2, test_params, CALLING_CONV_WIN64);
    add_function_call(&ctx, "cross_func", "void", 2, test_params, CALLING_CONV_AAPCS64);

    // Verify that the same function has different calling conventions
    if (ctx.function_call_count != 3) {
        return ASTHRA_TEST_FAIL;
    }

    // Each calling convention should use different registers for the same parameters
    const char *first_reg_sysv = ctx.function_calls[0].register_usage[0];
    const char *first_reg_win64 = ctx.function_calls[1].register_usage[0];
    const char *first_reg_aapcs = ctx.function_calls[2].register_usage[0];

    if (strcmp(first_reg_sysv, first_reg_win64) == 0 ||
        strcmp(first_reg_sysv, first_reg_aapcs) == 0 ||
        strcmp(first_reg_win64, first_reg_aapcs) == 0) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Cross-Compilation Validation Tests (Minimal Framework) ===\n");

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    // Run test
    if (test_cross_compilation_validation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Cross-Compilation Validation: PASS\n");
        passed++;
    } else {
        printf("❌ Cross-Compilation Validation: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}