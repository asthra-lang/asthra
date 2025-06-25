/**
 * FFI Test Stubs - Code Generation Implementation
 *
 * Provides enhanced codegen context functions using real infrastructure
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ffi_stubs_codegen.h"
#include "ffi_stubs_marshaling.h"
#include <stdio.h>
#include <stdlib.h>

// Stub implementations for code generator functions
bool code_generator_emit_function_call(CodeGenerator *generator, const char *func_name,
                                       int arg_count, void *args) {
    // Stub implementation - just return success
    return true;
}

bool code_generator_emit_return(CodeGenerator *generator, void *value) {
    // Stub implementation - just return success
    return true;
}

// =============================================================================
// ENHANCED CODEGEN IMPLEMENTATION
// =============================================================================

EnhancedCodegenContext *enhanced_codegen_create_context(void) {
    EnhancedCodegenContext *ctx = malloc(sizeof(EnhancedCodegenContext));
    if (!ctx)
        return NULL;

    // Create real code generator
    ctx->real_generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!ctx->real_generator) {
        free(ctx);
        return NULL;
    }

    // Create FFI assembly generator
    ctx->ffi_generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!ctx->ffi_generator) {
        code_generator_destroy(ctx->real_generator);
        free(ctx);
        return NULL;
    }

    ctx->initialized = true;
    atomic_store(&ctx->instruction_count, 0);
    atomic_store(&ctx->function_calls_generated, 0);
    atomic_store(&ctx->returns_generated, 0);
    atomic_store(&ctx->failed_generations, 0);
    ctx->output_buffer[0] = '\0';
    ctx->output_buffer_pos = 0;

    printf("[CODEGEN] Enhanced codegen context created with real infrastructure\n");
    return ctx;
}

void enhanced_codegen_destroy_context(EnhancedCodegenContext *ctx) {
    if (ctx) {
        if (ctx->real_generator) {
            code_generator_destroy(ctx->real_generator);
        }
        if (ctx->ffi_generator) {
            ffi_assembly_generator_destroy(ctx->ffi_generator);
        }
        printf("[CODEGEN] Enhanced codegen context destroyed\n");
        free(ctx);
    }
}

bool enhanced_codegen_generate_function_call(EnhancedCodegenContext *ctx, const char *func_name) {
    if (!ctx || !ctx->initialized || !func_name) {
        if (ctx)
            atomic_fetch_add(&ctx->failed_generations, 1);
        return false;
    }

    // Use real code generator to generate function call
    bool success = code_generator_emit_function_call(ctx->real_generator, func_name, 0, NULL);

    if (success) {
        atomic_fetch_add(&ctx->instruction_count, 1);
        atomic_fetch_add(&ctx->function_calls_generated, 1);

        // Add to output buffer
        int written =
            snprintf(ctx->output_buffer + ctx->output_buffer_pos,
                     sizeof(ctx->output_buffer) - ctx->output_buffer_pos, "call %s\n", func_name);
        if (written > 0) {
            ctx->output_buffer_pos += written;
        }

        printf("[CODEGEN] Generated function call: %s\n", func_name);
    } else {
        atomic_fetch_add(&ctx->failed_generations, 1);
    }

    return success;
}

bool enhanced_codegen_generate_return(EnhancedCodegenContext *ctx, void *value) {
    if (!ctx || !ctx->initialized) {
        if (ctx)
            atomic_fetch_add(&ctx->failed_generations, 1);
        return false;
    }

    // Use real code generator to generate return statement
    bool success = code_generator_emit_return(ctx->real_generator, value);

    if (success) {
        atomic_fetch_add(&ctx->instruction_count, 1);
        atomic_fetch_add(&ctx->returns_generated, 1);

        // Add to output buffer
        int written = snprintf(ctx->output_buffer + ctx->output_buffer_pos,
                               sizeof(ctx->output_buffer) - ctx->output_buffer_pos, "ret\n");
        if (written > 0) {
            ctx->output_buffer_pos += written;
        }

        printf("[CODEGEN] Generated return statement\n");
    } else {
        atomic_fetch_add(&ctx->failed_generations, 1);
    }

    return success;
}

bool enhanced_codegen_generate_ffi_call(EnhancedCodegenContext *ctx, const char *func_name,
                                        void **params, size_t param_count) {
    if (!ctx || !ctx->initialized || !func_name) {
        if (ctx)
            atomic_fetch_add(&ctx->failed_generations, 1);
        return false;
    }

    // Use FFI generator for external function calls
    bool success = stub_ffi_generate_extern_call(func_name, params, param_count, NULL);

    if (success) {
        atomic_fetch_add(&ctx->instruction_count, 1);
        atomic_fetch_add(&ctx->function_calls_generated, 1);

        printf("[CODEGEN] Generated FFI call: %s with %zu parameters\n", func_name, param_count);
    } else {
        atomic_fetch_add(&ctx->failed_generations, 1);
    }

    return success;
}

const char *enhanced_codegen_get_assembly_output(EnhancedCodegenContext *ctx) {
    return ctx ? ctx->output_buffer : NULL;
}

CodegenStats enhanced_codegen_get_stats(EnhancedCodegenContext *ctx) {
    CodegenStats stats = {0};

    if (ctx && ctx->initialized) {
        stats.total_instructions = atomic_load(&ctx->instruction_count);
        stats.function_calls = atomic_load(&ctx->function_calls_generated);
        stats.returns = atomic_load(&ctx->returns_generated);
        stats.failed_operations = atomic_load(&ctx->failed_generations);

        uint32_t total_operations = stats.function_calls + stats.returns;
        if (total_operations > 0) {
            stats.success_rate =
                (double)(total_operations - stats.failed_operations) / total_operations * 100.0;
        }
    }

    return stats;
}

// =============================================================================
// LEGACY COMPATIBILITY IMPLEMENTATION
// =============================================================================

CodegenContext *codegen_create_context(void) {
    return (CodegenContext *)enhanced_codegen_create_context();
}

void codegen_destroy_context(CodegenContext *ctx) {
    enhanced_codegen_destroy_context((EnhancedCodegenContext *)ctx);
}

bool codegen_generate_function_call(CodegenContext *ctx, const char *func_name) {
    return enhanced_codegen_generate_function_call((EnhancedCodegenContext *)ctx, func_name);
}

bool codegen_generate_return(CodegenContext *ctx, void *value) {
    return enhanced_codegen_generate_return((EnhancedCodegenContext *)ctx, value);
}

int codegen_get_instruction_count(CodegenContext *ctx) {
    EnhancedCodegenContext *enhanced_ctx = (EnhancedCodegenContext *)ctx;
    return enhanced_ctx ? atomic_load(&enhanced_ctx->instruction_count) : 0;
}