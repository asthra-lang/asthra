/**
 * Test file for platform-specific code generation
 * Tests instruction generation for different platforms
 */

#include "cross_platform_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_platform_specific_codegen", __FILE__, __LINE__, "Test platform-specific code generation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
};

// Test functions
static AsthraTestResult test_platform_specific_codegen(AsthraTestContext* context) {
    CrossPlatformContext ctx;
    init_cross_platform_context(&ctx);
    
    // Generate x86_64 instructions
    ctx.current_target = PLATFORM_X86_64_LINUX;
    add_platform_instruction(&ctx, "movq", "$42, %rax", PLATFORM_X86_64_LINUX);
    add_platform_instruction(&ctx, "addq", "%rbx, %rax", PLATFORM_X86_64_LINUX);
    add_platform_instruction(&ctx, "call", "printf", PLATFORM_X86_64_LINUX);
    
    // Generate ARM64 instructions
    add_platform_instruction(&ctx, "mov", "x0, #42", PLATFORM_ARM64_LINUX);
    add_platform_instruction(&ctx, "add", "x0, x0, x1", PLATFORM_ARM64_LINUX);
    add_platform_instruction(&ctx, "bl", "printf", PLATFORM_ARM64_LINUX);
    
    // Generate Windows-specific instructions
    add_platform_instruction(&ctx, "mov", "rcx, 42", PLATFORM_X86_64_WINDOWS);
    add_platform_instruction(&ctx, "call", "printf", PLATFORM_X86_64_WINDOWS);
    
    // Verify instructions were created
    if (ctx.instruction_count != 8) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Check platform-specific instruction properties
    int x86_count = 0, arm_count = 0, windows_count = 0;
    
    for (int i = 0; i < ctx.instruction_count; i++) {
        const PlatformInstruction* inst = &ctx.instructions[i];
        
        if (inst->target_platform == PLATFORM_X86_64_LINUX) {
            x86_count++;
            // x86_64 instructions should use register names like %rax, %rbx
            if (inst->operands && !strstr(inst->operands, "%r") && !strstr(inst->operands, "printf")) {
                return ASTHRA_TEST_FAIL;
            }
        } else if (inst->target_platform == PLATFORM_ARM64_LINUX) {
            arm_count++;
            // ARM64 instructions should use register names like x0, x1
            if (inst->operands && !strstr(inst->operands, "x") && !strstr(inst->operands, "printf")) {
                return ASTHRA_TEST_FAIL;
            }
        } else if (inst->target_platform == PLATFORM_X86_64_WINDOWS) {
            windows_count++;
            // Windows uses Intel syntax (no % prefix)
            if (inst->operands && strstr(inst->operands, "%")) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }
    
    if (x86_count != 3 || arm_count != 3 || windows_count != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify platform detection affects instruction generation
    ctx.current_target = PLATFORM_ARM64_LINUX;
    int before_count = ctx.instruction_count;
    add_platform_instruction(&ctx, "mov", "x2, #100", PLATFORM_X86_64_LINUX);
    
    // This x86 instruction on ARM target should be marked as platform-specific
    if (ctx.instructions[before_count].platform_specific != true) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Platform-Specific Code Generation Tests (Minimal Framework) ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    // Run test
    if (test_platform_specific_codegen(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Platform-Specific Codegen: PASS\n");
        passed++;
    } else {
        printf("❌ Platform-Specific Codegen: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
}