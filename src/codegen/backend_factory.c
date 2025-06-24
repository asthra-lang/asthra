/**
 * Asthra Programming Language Backend Factory
 * Creates and manages code generation backends
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "backend_interface.h"
#include "../compiler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Backend registry
typedef struct BackendRegistryEntry {
    AsthraBackendType type;
    const char *name;
    const AsthraBackendOps *ops;
} BackendRegistryEntry;

// Static registry for built-in backends
static BackendRegistryEntry backend_registry[16];
static size_t registry_count = 0;

// Forward declarations for built-in backends
extern const AsthraBackendOps llvm_backend_ops;

// Initialize built-in backends
static void initialize_builtin_backends(void) {
    static bool initialized = false;
    if (initialized) return;
    
    // Register LLVM backend
    backend_registry[0] = (BackendRegistryEntry){
        .type = ASTHRA_BACKEND_LLVM_IR,
        .name = "LLVM IR Generator",
        .ops = &llvm_backend_ops
    };
    
    registry_count = 1;
    initialized = true;
}

// Create a backend based on compiler options
AsthraBackend* asthra_backend_create(const AsthraCompilerOptions *options) {
    if (!options) return NULL;
    
    // LLVM is now the only backend
    AsthraBackendType type = ASTHRA_BACKEND_LLVM_IR;
    
    if (options->backend_type != 0 && options->backend_type != ASTHRA_BACKEND_LLVM_IR) {
        // For now, allow but warn about unsupported backend types
        fprintf(stderr, "Warning: Backend type %d is no longer supported, using LLVM backend\n", options->backend_type);
        type = ASTHRA_BACKEND_LLVM_IR;
    }
    
    AsthraBackend *backend = asthra_backend_create_by_type(type);
    if (backend && asthra_backend_initialize(backend, options) != 0) {
        asthra_backend_destroy(backend);
        return NULL;
    }
    return backend;
}

// Create a specific backend
AsthraBackend* asthra_backend_create_by_type(AsthraBackendType type) {
    initialize_builtin_backends();
    
    // Find the backend in registry
    const AsthraBackendOps *ops = NULL;
    const char *name = NULL;
    
    for (size_t i = 0; i < registry_count; i++) {
        if (backend_registry[i].type == type) {
            ops = backend_registry[i].ops;
            name = backend_registry[i].name;
            break;
        }
    }
    
    if (!ops) {
        fprintf(stderr, "Backend type %d not found in registry\n", type);
        return NULL;
    }
    
    // Allocate backend structure
    AsthraBackend *backend = calloc(1, sizeof(AsthraBackend));
    if (!backend) {
        fprintf(stderr, "Failed to allocate backend\n");
        return NULL;
    }
    
    // Initialize backend
    backend->type = type;
    backend->name = name;
    backend->ops = ops;
    backend->private_data = NULL;
    backend->last_error = NULL;
    
    // Initialize statistics
    backend->stats.lines_generated = 0;
    backend->stats.functions_processed = 0;
    backend->stats.generation_time = 0.0;
    
    return backend;
}

// Destroy a backend and free resources
void asthra_backend_destroy(AsthraBackend *backend) {
    if (!backend) return;
    
    // Call backend cleanup if available
    if (backend->ops && backend->ops->cleanup) {
        backend->ops->cleanup(backend);
    }
    
    // Free the backend structure
    free(backend);
}

// Initialize backend with options
int asthra_backend_initialize(AsthraBackend *backend, const AsthraCompilerOptions *options) {
    if (!backend || !backend->ops || !backend->ops->initialize) {
        return -1;
    }
    
    return backend->ops->initialize(backend, options);
}

// Generate code from AST
int asthra_backend_generate(AsthraBackend *backend,
                           AsthraCompilerContext *ctx,
                           const ASTNode *ast,
                           const char *output_file) {
    if (!backend || !backend->ops || !backend->ops->generate) {
        return -1;
    }
    
    return backend->ops->generate(backend, ctx, ast, output_file);
}

// Perform backend-specific optimizations
int asthra_backend_optimize(AsthraBackend *backend, void *ir, int opt_level) {
    if (!backend || !backend->ops || !backend->ops->optimize) {
        return 0; // No optimization available is not an error
    }
    
    return backend->ops->optimize(backend, ir, opt_level);
}

// Check if backend supports a feature
bool asthra_backend_supports_feature(AsthraBackend *backend, const char *feature) {
    if (!backend || !backend->ops || !backend->ops->supports_feature) {
        return false;
    }
    
    return backend->ops->supports_feature(backend, feature);
}

// Get backend information
const char* asthra_backend_get_name(AsthraBackend *backend) {
    if (!backend || !backend->ops || !backend->ops->get_name) {
        return backend ? backend->name : "Unknown";
    }
    
    return backend->ops->get_name(backend);
}

const char* asthra_backend_get_version(AsthraBackend *backend) {
    if (!backend || !backend->ops || !backend->ops->get_version) {
        return "1.0.0";
    }
    
    return backend->ops->get_version(backend);
}

const char* asthra_backend_get_last_error(AsthraBackend *backend) {
    if (!backend) return "Invalid backend";
    return backend->last_error ? backend->last_error : "No error";
}

// Get backend statistics
void asthra_backend_get_stats(AsthraBackend *backend, 
                             size_t *lines_generated,
                             size_t *functions_processed,
                             double *generation_time) {
    if (!backend) return;
    
    if (lines_generated) *lines_generated = backend->stats.lines_generated;
    if (functions_processed) *functions_processed = backend->stats.functions_processed;
    if (generation_time) *generation_time = backend->stats.generation_time;
}

// Register a custom backend
int asthra_backend_register(AsthraBackendType type, 
                           const char *name,
                           const AsthraBackendOps *ops) {
    initialize_builtin_backends();
    
    if (registry_count >= 16) {
        return -1; // Registry full
    }
    
    // Check if already registered
    for (size_t i = 0; i < registry_count; i++) {
        if (backend_registry[i].type == type) {
            return -2; // Already registered
        }
    }
    
    // Add to registry
    backend_registry[registry_count++] = (BackendRegistryEntry){
        .type = type,
        .name = name,
        .ops = ops
    };
    
    return 0;
}

// Unregister a backend
int asthra_backend_unregister(AsthraBackendType type) {
    initialize_builtin_backends();
    
    // Don't allow unregistering built-in backends
    if (type == ASTHRA_BACKEND_LLVM_IR) {
        return -1;
    }
    
    // Find and remove from registry
    for (size_t i = 1; i < registry_count; i++) {
        if (backend_registry[i].type == type) {
            // Shift remaining entries
            for (size_t j = i; j < registry_count - 1; j++) {
                backend_registry[j] = backend_registry[j + 1];
            }
            registry_count--;
            return 0;
        }
    }
    
    return -2; // Not found
}

// Determine output file name based on backend type
char* asthra_backend_get_output_filename(AsthraBackendType type, 
                                        const char *input_file,
                                        const char *output_file) {
    if (output_file && *output_file) {
        return strdup(output_file);
    }
    
    if (!input_file || !*input_file) {
        return strdup("a.out");
    }
    
    // Get base name without extension
    const char *base = strrchr(input_file, '/');
    base = base ? base + 1 : input_file;
    
    size_t base_len = strlen(base);
    const char *dot = strrchr(base, '.');
    if (dot) {
        base_len = dot - base;
    }
    
    // Determine extension based on backend
    const char *ext = asthra_backend_get_file_extension(type);
    
    // Allocate buffer for output name
    size_t out_len = base_len + strlen(ext) + 2;
    char *out_name = malloc(out_len);
    if (!out_name) return NULL;
    
    snprintf(out_name, out_len, "%.*s.%s", (int)base_len, base, ext);
    
    return out_name;
}

// Validate backend options
bool asthra_backend_validate_options(AsthraBackendType type,
                                   const AsthraBackendOptions *options) {
    if (!options) return false;
    
    // Basic validation
    if (options->optimization_level < 0 || options->optimization_level > 3) {
        return false;
    }
    
    // Backend-specific validation could go here
    
    return true;
}

// Get default file extension for backend
const char* asthra_backend_get_file_extension(AsthraBackendType type) {
    switch (type) {
        case ASTHRA_BACKEND_LLVM_IR:
            return "ll";
        default:
            return "out";
    }
}