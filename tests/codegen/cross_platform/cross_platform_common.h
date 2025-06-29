/**
 * Cross Platform Test Common Header - Stub Implementation
 *
 * This is a stub header to allow cross platform tests to compile.
 * The original implementation was removed with the old code generator.
 */

#ifndef CROSS_PLATFORM_COMMON_H
#define CROSS_PLATFORM_COMMON_H

#include "../framework/test_framework.h"
#include <stdbool.h>
#include <stddef.h>

// Platform count for tests
#define PLATFORM_COUNT 3
#define PLATFORM_X86_64_WINDOWS 0
#define PLATFORM_X86_64_LINUX 1
#define PLATFORM_ARM64_LINUX 2

// Stub types
typedef struct {
    const char *name;
    int arch;
    int calling_convention;
} PlatformConfig;

typedef struct {
    const char *opcode;
    const char *operand;
    int platform;
} PlatformInstruction;

typedef struct {
    PlatformConfig platforms[PLATFORM_COUNT];
    int platform_count;
    PlatformInstruction instructions[100];
    int instruction_count;
} CrossPlatformContext;

// Stub functions
static inline void init_cross_platform_context(CrossPlatformContext *ctx) {
    if (!ctx)
        return;

    ctx->platforms[0] = (PlatformConfig){"x86_64", 0, 0};
    ctx->platforms[1] = (PlatformConfig){"arm64", 1, 1};
    ctx->platforms[2] = (PlatformConfig){"wasm32", 2, 2};
    ctx->platform_count = PLATFORM_COUNT;
}

static inline bool validate_platform_configuration(const PlatformConfig *config) {
    return config && config->name != NULL;
}

static inline void add_platform_instruction(CrossPlatformContext *ctx, const char *opcode,
                                            const char *operand, int platform) {
    if (!ctx || ctx->instruction_count >= 100)
        return;

    ctx->instructions[ctx->instruction_count].opcode = opcode;
    ctx->instructions[ctx->instruction_count].operand = operand;
    ctx->instructions[ctx->instruction_count].platform = platform;
    ctx->instruction_count++;
}

#endif // CROSS_PLATFORM_COMMON_H