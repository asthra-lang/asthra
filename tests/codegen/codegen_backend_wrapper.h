/**
 * Codegen Backend Test Wrapper - Simplified API for Codegen Tests
 *
 * Provides a simplified wrapper around the AsthraBackend interface
 * to make codegen test migration from old CodeGenerator API easier.
 */

#ifndef CODEGEN_BACKEND_WRAPPER_H
#define CODEGEN_BACKEND_WRAPPER_H

// Temporary placeholder for removed backend type
typedef void AsthraBackend;
#include "semantic_analyzer.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Legacy constants for compatibility
// Backend stubs are provided by backend_stubs.h

/**
 * Set semantic analyzer for the backend
 * This is a wrapper to store the analyzer for later use
 */
void asthra_backend_set_semantic_analyzer(AsthraBackend *backend, SemanticAnalyzer *analyzer);

/**
 * Generate program using simplified API
 * This wraps the full backend generate API for test compatibility
 */
bool asthra_backend_generate_program(AsthraBackend *backend, ASTNode *ast);

/**
 * Emit assembly to buffer using simplified API
 * This wraps the backend's assembly emission functionality
 */
bool asthra_backend_emit_assembly(AsthraBackend *backend, char *buffer, size_t buffer_size);

/**
 * Generate code to file using simplified API
 * This wraps the backend's file generation functionality
 */
bool asthra_backend_generate_to_file(AsthraBackend *backend, ASTNode *ast, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // CODEGEN_BACKEND_WRAPPER_H