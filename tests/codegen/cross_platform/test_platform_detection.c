/**
 * Test file for platform detection and configuration
 * Tests platform initialization and validation
 */

#include "cross_platform_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_target_platform_detection", __FILE__, __LINE__, "Test platform detection and configuration", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
};

// Test functions
static AsthraTestResult test_target_platform_detection(AsthraTestContext* context) {
    CrossPlatformContext ctx;
    init_cross_platform_context(&ctx);
    
    // Verify all platforms were initialized
    if (ctx.platform_count != PLATFORM_COUNT) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Validate each platform configuration
    for (int i = 0; i < ctx.platform_count; i++) {
        const PlatformConfig* config = &ctx.platforms[i];
        
        if (!validate_platform_configuration(config)) {
            return ASTHRA_TEST_FAIL;
        }
        
        // Check platform-specific attributes
        if (strstr(config->arch, "x86_64")) {
            if (config->calling_conv != CALLING_CONV_SYSTEM_V && 
                config->calling_conv != CALLING_CONV_WIN64) {
                return ASTHRA_TEST_FAIL;
            }
        } else if (strstr(config->arch, "arm64")) {
            if (config->calling_conv != CALLING_CONV_AAPCS64) {
                return ASTHRA_TEST_FAIL;
            }
        }
        
        // Verify OS-specific settings
        if (strcmp(config->os, "windows") == 0) {
            if (strcmp(config->asm_syntax, "intel") != 0) {
                return ASTHRA_TEST_FAIL;
            }
        } else {
            if (strcmp(config->asm_syntax, "att") != 0 && strcmp(config->asm_syntax, "arm") != 0) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }
    
    // Test platform lookup
    const PlatformConfig* linux_config = get_platform_config(&ctx, PLATFORM_X86_64_LINUX);
    if (!linux_config || strcmp(linux_config->os, "linux") != 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    const PlatformConfig* windows_config = get_platform_config(&ctx, PLATFORM_X86_64_WINDOWS);
    if (!windows_config || strcmp(windows_config->os, "windows") != 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    const PlatformConfig* arm_config = get_platform_config(&ctx, PLATFORM_ARM64_LINUX);
    if (!arm_config || strcmp(arm_config->arch, "arm64") != 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Platform Detection Tests (Minimal Framework) ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    // Run test
    if (test_target_platform_detection(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Target Platform Detection: PASS\n");
        passed++;
    } else {
        printf("❌ Target Platform Detection: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
}