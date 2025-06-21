/**
 * Common header for cross-platform codegen tests
 * Shared types, enums, and helper functions
 */

#ifndef CROSS_PLATFORM_COMMON_H
#define CROSS_PLATFORM_COMMON_H

#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Platform types
typedef enum {
    PLATFORM_X86_64_LINUX,
    PLATFORM_X86_64_WINDOWS,
    PLATFORM_X86_64_MACOS,
    PLATFORM_ARM64_LINUX,
    PLATFORM_ARM64_MACOS,
    PLATFORM_ARM64_ANDROID,
    PLATFORM_COUNT
} PlatformType;

// Calling conventions
typedef enum {
    CALLING_CONV_SYSTEM_V,    // Linux/macOS x86_64
    CALLING_CONV_WIN64,       // Windows x86_64
    CALLING_CONV_AAPCS64,     // ARM64 standard
    CALLING_CONV_COUNT
} CallingConvention;

// Platform-specific configuration
typedef struct {
    PlatformType platform;
    const char* name;
    const char* arch;
    const char* os;
    CallingConvention calling_conv;
    int pointer_size;      // Size in bytes
    int alignment;         // Default alignment
    bool little_endian;
    const char* asm_syntax; // "att" or "intel"
} PlatformConfig;

// Platform-specific instruction
typedef struct {
    uint64_t address;
    const char* mnemonic;
    const char* operands;
    PlatformType target_platform;
    bool platform_specific;
} PlatformInstruction;

// Function call representation
typedef struct {
    const char* function_name;
    const char* return_type;
    int param_count;
    const char* param_types[8];
    CallingConvention calling_conv;
    const char* register_usage[8]; // Which registers are used for params
} FunctionCall;

// Cross-platform context
typedef struct {
    PlatformConfig platforms[PLATFORM_COUNT];
    int platform_count;
    PlatformInstruction instructions[64];
    int instruction_count;
    FunctionCall function_calls[16];
    int function_call_count;
    PlatformType current_target;
    bool cross_compilation_mode;
} CrossPlatformContext;

// Helper functions as inline to avoid multiple definition errors
static inline void init_cross_platform_context(CrossPlatformContext* ctx) {
    ctx->platform_count = 0;
    ctx->instruction_count = 0;
    ctx->function_call_count = 0;
    ctx->current_target = PLATFORM_X86_64_LINUX;
    ctx->cross_compilation_mode = false;
    
    // Initialize platform configurations
    PlatformConfig configs[] = {
        {PLATFORM_X86_64_LINUX, "x86_64-linux", "x86_64", "linux", CALLING_CONV_SYSTEM_V, 8, 8, true, "att"},
        {PLATFORM_X86_64_WINDOWS, "x86_64-windows", "x86_64", "windows", CALLING_CONV_WIN64, 8, 8, true, "intel"},
        {PLATFORM_X86_64_MACOS, "x86_64-macos", "x86_64", "macos", CALLING_CONV_SYSTEM_V, 8, 8, true, "att"},
        {PLATFORM_ARM64_LINUX, "arm64-linux", "arm64", "linux", CALLING_CONV_AAPCS64, 8, 8, true, "arm"},
        {PLATFORM_ARM64_MACOS, "arm64-macos", "arm64", "macos", CALLING_CONV_AAPCS64, 8, 8, true, "arm"},
        {PLATFORM_ARM64_ANDROID, "arm64-android", "arm64", "android", CALLING_CONV_AAPCS64, 8, 8, true, "arm"}
    };
    
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        ctx->platforms[i] = configs[i];
        ctx->platform_count++;
    }
    
    for (int i = 0; i < 64; i++) {
        ctx->instructions[i] = (PlatformInstruction){0};
    }
    
    for (int i = 0; i < 16; i++) {
        ctx->function_calls[i] = (FunctionCall){0};
    }
}

static inline const PlatformConfig* get_platform_config(const CrossPlatformContext* ctx, PlatformType platform) {
    for (int i = 0; i < ctx->platform_count; i++) {
        if (ctx->platforms[i].platform == platform) {
            return &ctx->platforms[i];
        }
    }
    return NULL;
}

static inline void add_platform_instruction(CrossPlatformContext* ctx, const char* mnemonic, const char* operands, PlatformType platform) {
    if (ctx->instruction_count < 64) {
        PlatformInstruction* inst = &ctx->instructions[ctx->instruction_count++];
        inst->address = 0x1000 + (ctx->instruction_count * 4);
        inst->mnemonic = mnemonic;
        inst->operands = operands;
        inst->target_platform = platform;
        inst->platform_specific = (platform != ctx->current_target);
    }
}

static inline void add_function_call(CrossPlatformContext* ctx, const char* name, const char* ret_type, 
                                   int param_count, const char** param_types, CallingConvention conv) {
    if (ctx->function_call_count < 16 && param_count <= 8) {
        FunctionCall* call = &ctx->function_calls[ctx->function_call_count++];
        call->function_name = name;
        call->return_type = ret_type;
        call->param_count = param_count;
        call->calling_conv = conv;
        
        for (int i = 0; i < param_count; i++) {
            call->param_types[i] = param_types[i];
        }
        
        // Set platform-specific register usage
        if (conv == CALLING_CONV_SYSTEM_V) {
            // System V ABI: rdi, rsi, rdx, rcx, r8, r9
            const char* regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
            for (int i = 0; i < param_count && i < 6; i++) {
                call->register_usage[i] = regs[i];
            }
        } else if (conv == CALLING_CONV_WIN64) {
            // Windows x64: rcx, rdx, r8, r9
            const char* regs[] = {"rcx", "rdx", "r8", "r9"};
            for (int i = 0; i < param_count && i < 4; i++) {
                call->register_usage[i] = regs[i];
            }
        } else if (conv == CALLING_CONV_AAPCS64) {
            // ARM64 AAPCS: x0-x7
            const char* regs[] = {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"};
            for (int i = 0; i < param_count && i < 8; i++) {
                call->register_usage[i] = regs[i];
            }
        }
    }
}

static inline bool validate_platform_configuration(const PlatformConfig* config) {
    if (!config || !config->name || !config->arch || !config->os) {
        return false;
    }
    
    if (config->pointer_size != 8) { // Currently only support 64-bit
        return false;
    }
    
    if (config->alignment <= 0 || config->alignment > 16) {
        return false;
    }
    
    if (config->calling_conv >= CALLING_CONV_COUNT) {
        return false;
    }
    
    return true;
}

#endif // CROSS_PLATFORM_COMMON_H