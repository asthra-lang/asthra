/**
 * Backend Test Wrapper - Simplified API for Integration Tests
 *
 * Provides a simplified wrapper around the AsthraBackend interface
 * to make test migration from old CodeGenerator API easier.
 */

#ifndef BACKEND_TEST_WRAPPER_H
#define BACKEND_TEST_WRAPPER_H

// Backend interface removed - using LLVM directly
#include "semantic_analyzer.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set semantic analyzer for the backend
 * This is a wrapper to store the analyzer for later use
 */
void asthra_backend_set_semantic_analyzer(void *backend, SemanticAnalyzer *analyzer);

/**
 * Generate program using simplified API
 * This wraps the full backend generate API for test compatibility
 */
bool asthra_backend_generate_program(void *backend, ASTNode *ast);

/**
 * Emit assembly to buffer using simplified API
 * This wraps the backend's assembly emission functionality
 */
bool asthra_backend_emit_assembly(void *backend, char *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif // BACKEND_TEST_WRAPPER_H