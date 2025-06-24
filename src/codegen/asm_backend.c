/**
 * Asthra Programming Language Assembly Backend
 * Generates assembly code from Asthra AST
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "backend_interface.h"
#include "code_generator_core.h"
#include "code_generator_common.h"
#include "../parser/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Private data for Assembly backend
typedef struct AsmBackendData {
    CodeGenerator *codegen_ctx;
    char *output_filename;
    AsthraTargetArch target_arch;
} AsmBackendData;

// Initialize the Assembly backend
static int asm_backend_initialize(AsthraBackend *backend, const AsthraCompilerOptions *options) {
    if (!backend || !options) return -1;
    
    // Allocate private data
    AsmBackendData *data = calloc(1, sizeof(AsmBackendData));
    if (!data) {
        backend->last_error = "Failed to allocate backend data";
        return -1;
    }
    
    // Store options
    backend->options.optimization_level = options->opt_level;
    backend->options.debug_info = options->debug_info;
    backend->options.verbose = options->verbose;
    backend->options.target_arch = options->target_arch;
    data->target_arch = options->target_arch;
    
    // Initialize code generator context
    // Map Asthra target arch to code generator arch
    TargetArchitecture arch = TARGET_ARCH_X86_64; // Default
    switch (data->target_arch) {
        case ASTHRA_TARGET_X86_64:
            arch = TARGET_ARCH_X86_64;
            break;
        case ASTHRA_TARGET_ARM64:
            arch = TARGET_ARCH_AARCH64;
            break;
        default:
            arch = TARGET_ARCH_X86_64;
            break;
    }
    
    data->codegen_ctx = code_generator_create(arch, CALLING_CONV_SYSTEM_V_AMD64);
    if (!data->codegen_ctx) {
        free(data);
        backend->last_error = "Failed to create code generator context";
        return -1;
    }
    
    backend->private_data = data;
    return 0;
}

// Generate assembly code from AST
static int asm_backend_generate(AsthraBackend *backend, 
                               AsthraCompilerContext *ctx,
                               const ASTNode *ast,
                               const char *output_file) {
    if (!backend || !ctx || !ast) return -1;
    
    AsmBackendData *data = (AsmBackendData*)backend->private_data;
    if (!data || !data->codegen_ctx) {
        backend->last_error = "Backend not initialized";
        return -1;
    }
    
    // Track generation time
    clock_t start_time = clock();
    
    // Store output filename
    if (output_file) {
        data->output_filename = strdup(output_file);
    }
    
    // Generate code using the existing code generator
    bool result = code_generate_program(data->codegen_ctx, (ASTNode*)ast);
    if (!result) {
        backend->last_error = "Code generation failed";
        return -1;
    }
    
    // Emit assembly to buffer then write to file
    const char *target_file = output_file ? output_file : "output.s";
    
    // Allocate buffer for assembly output
    size_t buffer_size = 1024 * 1024; // 1MB buffer
    char *asm_buffer = malloc(buffer_size);
    if (!asm_buffer) {
        backend->last_error = "Failed to allocate assembly buffer";
        return -1;
    }
    
    // Generate assembly to buffer
    if (!code_generator_emit_assembly(data->codegen_ctx, asm_buffer, buffer_size)) {
        free(asm_buffer);
        backend->last_error = "Failed to emit assembly";
        return -1;
    }
    
    // Write buffer to file
    FILE *fp = fopen(target_file, "w");
    if (!fp) {
        free(asm_buffer);
        backend->last_error = "Failed to open output file";
        return -1;
    }
    
    fprintf(fp, "%s", asm_buffer);
    fclose(fp);
    free(asm_buffer);
    
    // Update statistics
    // TODO: Get actual statistics from code generator
    backend->stats.lines_generated = 1000; // Placeholder
    backend->stats.functions_processed = 10; // Placeholder
    backend->stats.generation_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;
    
    return 0;
}

// Optimize assembly code
static int asm_backend_optimize(AsthraBackend *backend, void *ir, int opt_level) {
    if (!backend || !backend->private_data) return -1;
    
    AsmBackendData *data = (AsmBackendData*)backend->private_data;
    if (!data || !data->codegen_ctx) return -1;
    
    // Apply optimizations using the code generator's optimizer
    // TODO: Implement optimization pass integration
    
    return 0;
}

// Cleanup Assembly backend
static void asm_backend_cleanup(AsthraBackend *backend) {
    if (!backend || !backend->private_data) return;
    
    AsmBackendData *data = (AsmBackendData*)backend->private_data;
    
    if (data->codegen_ctx) {
        code_generator_destroy(data->codegen_ctx);
    }
    
    if (data->output_filename) {
        free(data->output_filename);
    }
    
    free(data);
    backend->private_data = NULL;
}

// Check if Assembly backend supports a feature
static bool asm_backend_supports_feature(AsthraBackend *backend, const char *feature) {
    if (!feature) return false;
    
    // List of supported features
    const char *supported[] = {
        "functions",
        "expressions",
        "statements", 
        "types",
        "integer_literals",
        "float_literals",
        "arithmetic",
        "comparisons",
        "logical_operations",
        "if_statements",
        "while_loops",
        "for_loops",
        "return_statements",
        "blocks",
        "function_calls",
        "register_allocation",
        "instruction_selection",
        "x86_64",
        "arm64",
        "elf_output",
        "macho_output",
        NULL
    };
    
    for (int i = 0; supported[i]; i++) {
        if (strcmp(feature, supported[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

// Get backend version
static const char* asm_backend_get_version(AsthraBackend *backend) {
    return "1.0.0";
}

// Get backend name
static const char* asm_backend_get_name(AsthraBackend *backend) {
    return "Asthra Assembly Generator Backend";
}

// Assembly Backend operations structure
const AsthraBackendOps asm_backend_ops = {
    .initialize = asm_backend_initialize,
    .generate = asm_backend_generate,
    .optimize = asm_backend_optimize,
    .cleanup = asm_backend_cleanup,
    .supports_feature = asm_backend_supports_feature,
    .get_version = asm_backend_get_version,
    .get_name = asm_backend_get_name
};