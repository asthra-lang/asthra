/**
 * Asthra Programming Language LLVM Backend
 * Main entry point and backend interface implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "backend_interface.h"
#include "llvm_backend_internal.h"
#include "llvm_types.h"
#include "llvm_debug.h"
#include "llvm_runtime.h"
#include "llvm_locals.h"
#include "llvm_expr_gen.h"
#include "llvm_stmt_gen.h"
#include "llvm_func_gen.h"

#include "../parser/ast_types.h"
#include "../analysis/semantic_symbols.h"
#include "../analysis/type_info.h"
#include "../compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>

// Initialize the LLVM backend
static int llvm_backend_initialize(AsthraBackend *backend, const AsthraCompilerOptions *options) {
    if (!backend || !options) return -1;
    
    // Allocate private data
    LLVMBackendData *data = calloc(1, sizeof(LLVMBackendData));
    if (!data) {
        backend->last_error = "Failed to allocate LLVM backend data";
        return -1;
    }
    
    backend->private_data = data;
    
    // Initialize local variables tracking
    data->local_vars = NULL;
    
    // Initialize LLVM
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    LLVMInitializeNativeAsmParser();
    
    // Create context and module
    data->context = LLVMContextCreate();
    if (!data->context) {
        backend->last_error = "Failed to create LLVM context";
        free(data);
        return -1;
    }
    
    const char *module_name = options->input_file ? options->input_file : "asthra_module";
    data->module = LLVMModuleCreateWithNameInContext(module_name, data->context);
    if (!data->module) {
        backend->last_error = "Failed to create LLVM module";
        LLVMContextDispose(data->context);
        free(data);
        return -1;
    }
    
    // Create IR builder
    data->builder = LLVMCreateBuilderInContext(data->context);
    if (!data->builder) {
        backend->last_error = "Failed to create LLVM builder";
        LLVMDisposeModule(data->module);
        LLVMContextDispose(data->context);
        free(data);
        return -1;
    }
    
    // Set target triple
    char *triple = LLVMGetDefaultTargetTriple();
    LLVMSetTarget(data->module, triple);
    LLVMDisposeMessage(triple);
    
    // Initialize type cache
    llvm_types_cache_init(data);
    
    // Declare runtime functions
    declare_runtime_functions(data);
    
    // Store options
    backend->options.optimization_level = options->opt_level;
    backend->options.debug_info = options->debug_info;
    backend->options.verbose = options->verbose;
    backend->options.target_arch = options->target_arch;
    backend->options.asm_syntax = options->asm_syntax;
    
    // Initialize debug info if requested
    if (options->debug_info) {
        const char *filename = options->input_file ? options->input_file : "asthra_module";
        bool is_optimized = options->opt_level > ASTHRA_OPT_NONE;
        initialize_debug_info(data, filename, is_optimized);
    }
    
    // Success
    return 0;
}

// Generate LLVM IR from AST
static int llvm_backend_generate(AsthraBackend *backend, 
                                AsthraCompilerContext *ctx, 
                                const ASTNode *ast, 
                                const char *output_file) {
    if (!backend || !backend->private_data || !ast) {
        if (backend) backend->last_error = "Invalid parameters to generate";
        return -1;
    }
    
    LLVMBackendData *data = (LLVMBackendData *)backend->private_data;
    data->compiler_ctx = ctx;
    
    // Start timing
    clock_t start_time = clock();
    
    // Generate code for all top-level nodes
    fprintf(stderr, "DEBUG: Starting code generation\n");
    const ASTNode *current = ast;
    while (current) {
        fprintf(stderr, "DEBUG: Processing top-level node type=%d\n", current->type);
        
        // Check for program node
        if (current->type == AST_PROGRAM) {
            // Process all children of the program node
            if (current->data.program.declarations) {
                for (size_t i = 0; i < current->data.program.declarations->count; i++) {
                    ASTNode *decl = current->data.program.declarations->nodes[i];
                    fprintf(stderr, "DEBUG: Processing program child %zu, type=%d\n", i, decl ? decl->type : -1);
                    generate_top_level(data, decl);
                }
            }
            break; // Program node should be the only top-level node
        } else {
            // Process individual top-level declaration
            generate_top_level(data, current);
        }
        
        // Move to next sibling
        // AST nodes don't have a next pointer in this implementation
        break;
    }
    
    // Finalize debug info
    if (data->di_builder) {
        finalize_debug_info(data);
    }
    
    // Verify module
    char *error = NULL;
    if (LLVMVerifyModule(data->module, LLVMReturnStatusAction, &error)) {
        backend->last_error = error ? error : "Module verification failed";
        if (error) LLVMDisposeMessage(error);
        
        // Dump module for debugging
        if (backend->options.verbose) {
            char *module_str = LLVMPrintModuleToString(data->module);
            fprintf(stderr, "LLVM Module:\n%s\n", module_str);
            LLVMDisposeMessage(module_str);
        }
        
        return -1;
    }
    
    // Store output filename for later use
    if (data->output_filename) {
        free(data->output_filename);
    }
    data->output_filename = strdup(output_file);
    
    // Write bitcode to file
    if (LLVMWriteBitcodeToFile(data->module, output_file) != 0) {
        backend->last_error = "Failed to write bitcode to file";
        return -1;
    }
    
    // Update statistics
    clock_t end_time = clock();
    backend->stats.generation_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    return 0;
}

// Optimize LLVM IR
static int llvm_backend_optimize(AsthraBackend *backend, void *ir, int opt_level) {
    if (!backend || !backend->private_data) return -1;
    
    LLVMBackendData *data = (LLVMBackendData *)backend->private_data;
    
    // For now, LLVM optimization passes would be applied here
    // This would involve creating a pass manager and adding optimization passes
    
    return 0;
}

// Cleanup LLVM backend
static void llvm_backend_cleanup(AsthraBackend *backend) {
    if (!backend || !backend->private_data) return;
    
    LLVMBackendData *data = (LLVMBackendData *)backend->private_data;
    
    // Clear local variables
    clear_local_vars(data);
    
    // Dispose debug info builder
    if (data->di_builder) {
        LLVMDisposeDIBuilder(data->di_builder);
    }
    
    // Dispose LLVM resources
    if (data->builder) {
        LLVMDisposeBuilder(data->builder);
    }
    
    if (data->module) {
        LLVMDisposeModule(data->module);
    }
    
    if (data->context) {
        LLVMContextDispose(data->context);
    }
    
    if (data->output_filename) {
        free(data->output_filename);
    }
    
    free(data);
    backend->private_data = NULL;
}

// Check if LLVM backend supports a feature
static bool llvm_backend_supports_feature(AsthraBackend *backend, const char *feature) {
    if (!feature) return false;
    
    // LLVM backend supports these features
    if (strcmp(feature, "debug-info") == 0) return true;
    if (strcmp(feature, "optimization") == 0) return true;
    if (strcmp(feature, "cross-compilation") == 0) return true;
    if (strcmp(feature, "llvm-ir") == 0) return true;
    if (strcmp(feature, "bitcode") == 0) return true;
    
    // Target-specific features
    if (strcmp(feature, "x86_64") == 0) return true;
    if (strcmp(feature, "aarch64") == 0) return true;
    if (strcmp(feature, "arm64") == 0) return true;
    
    return false;
}

// Get backend version
static const char* llvm_backend_get_version(AsthraBackend *backend) {
    static char version[256];
    snprintf(version, sizeof(version), "LLVM Backend 1.0 (LLVM %d.%d)", 
             LLVM_VERSION_MAJOR, LLVM_VERSION_MINOR);
    return version;
}

// Get backend name
static const char* llvm_backend_get_name(AsthraBackend *backend) {
    return "LLVM Backend";
}

// LLVM backend operations
const AsthraBackendOps llvm_backend_ops = {
    .initialize = llvm_backend_initialize,
    .generate = llvm_backend_generate,
    .optimize = llvm_backend_optimize,
    .cleanup = llvm_backend_cleanup,
    .supports_feature = llvm_backend_supports_feature,
    .get_version = llvm_backend_get_version,
    .get_name = llvm_backend_get_name
};

// Create LLVM backend instance
AsthraBackend* asthra_create_llvm_backend(void) {
    AsthraBackend *backend = calloc(1, sizeof(AsthraBackend));
    if (!backend) return NULL;
    
    backend->type = ASTHRA_BACKEND_LLVM_IR;
    backend->name = "llvm";
    backend->ops = &llvm_backend_ops;
    
    return backend;
}