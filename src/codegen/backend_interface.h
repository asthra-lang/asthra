/**
 * Asthra Programming Language Backend Interface
 * Common interface for all code generation backends
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_BACKEND_INTERFACE_H
#define ASTHRA_BACKEND_INTERFACE_H

#include "../compiler.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct AsthraBackend AsthraBackend;
typedef struct AsthraBackendOptions AsthraBackendOptions;
typedef struct ASTNode ASTNode;

// Backend-specific options
struct AsthraBackendOptions {
    // Common options
    const char *output_file;
    int optimization_level;
    bool debug_info;
    bool verbose;

    // Target-specific options
    AsthraTargetArch target_arch;
    const char *target_triple;

    // Assembly-specific options
    AsthraAssemblySyntax asm_syntax;

    // Backend-specific data
    void *backend_specific;
};

// Backend interface operations
typedef struct AsthraBackendOps {
    // Initialize the backend
    int (*initialize)(AsthraBackend *backend, const AsthraCompilerOptions *options);

    // Generate code from AST
    int (*generate)(AsthraBackend *backend, AsthraCompilerContext *ctx, const ASTNode *ast,
                    const char *output_file);

    // Perform backend-specific optimizations
    int (*optimize)(AsthraBackend *backend, void *ir, int opt_level);

    // Cleanup and release resources
    void (*cleanup)(AsthraBackend *backend);

    // Get backend capabilities
    bool (*supports_feature)(AsthraBackend *backend, const char *feature);

    // Get backend information
    const char *(*get_version)(AsthraBackend *backend);
    const char *(*get_name)(AsthraBackend *backend);
} AsthraBackendOps;

// Backend structure
struct AsthraBackend {
    AsthraBackendType type;
    const char *name;
    const AsthraBackendOps *ops;
    AsthraBackendOptions options;

    // Backend-specific private data
    void *private_data;

    // Error reporting
    const char *last_error;

    // Statistics
    struct {
        size_t lines_generated;
        size_t functions_processed;
        double generation_time;
    } stats;
};

// =============================================================================
// BACKEND FACTORY INTERFACE
// =============================================================================

// Create a backend based on compiler options
AsthraBackend *asthra_backend_create(const AsthraCompilerOptions *options);

// Create a specific backend
AsthraBackend *asthra_backend_create_by_type(AsthraBackendType type);

// Destroy a backend and free resources
void asthra_backend_destroy(AsthraBackend *backend);

// =============================================================================
// BACKEND OPERATIONS
// =============================================================================

// Initialize backend with options
int asthra_backend_initialize(AsthraBackend *backend, const AsthraCompilerOptions *options);

// Generate code from AST
int asthra_backend_generate(AsthraBackend *backend, AsthraCompilerContext *ctx, const ASTNode *ast,
                            const char *output_file);

// Perform backend-specific optimizations
int asthra_backend_optimize(AsthraBackend *backend, void *ir, int opt_level);

// Check if backend supports a feature
bool asthra_backend_supports_feature(AsthraBackend *backend, const char *feature);

// Get backend information
const char *asthra_backend_get_name(AsthraBackend *backend);
const char *asthra_backend_get_version(AsthraBackend *backend);
const char *asthra_backend_get_last_error(AsthraBackend *backend);

// Get backend statistics
void asthra_backend_get_stats(AsthraBackend *backend, size_t *lines_generated,
                              size_t *functions_processed, double *generation_time);

// =============================================================================
// BACKEND REGISTRATION
// =============================================================================

// Register a custom backend
int asthra_backend_register(AsthraBackendType type, const char *name, const AsthraBackendOps *ops);

// Unregister a backend
int asthra_backend_unregister(AsthraBackendType type);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Determine output file name based on backend type
char *asthra_backend_get_output_filename(AsthraBackendType type, const char *input_file,
                                         const char *output_file);

// Validate backend options
bool asthra_backend_validate_options(AsthraBackendType type, const AsthraBackendOptions *options);

// Get default file extension for backend
const char *asthra_backend_get_file_extension(AsthraBackendType type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_BACKEND_INTERFACE_H