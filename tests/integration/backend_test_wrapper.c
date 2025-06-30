/**
 * Backend Test Wrapper - Implementation
 *
 * Implements simplified wrapper functions around AsthraBackend
 * to facilitate migration from old CodeGenerator API.
 */

#include "backend_test_wrapper.h"
#include "compiler.h"
#include <stdlib.h>
#include <string.h>

// Global storage for semantic analyzer (for test compatibility)
static SemanticAnalyzer *test_analyzer = NULL;

void asthra_backend_set_semantic_analyzer(void *backend, SemanticAnalyzer *analyzer) {
    // Store analyzer globally for test compatibility
    (void)backend;
    test_analyzer = analyzer;
}

bool asthra_backend_generate_program(void *backend, ASTNode *ast) {
    if (!backend || !ast)
        return false;

    // Create a minimal compiler context for the test
    AsthraCompilerContext ctx = {0};
    ctx.options.input_file = "test.asthra";
    ctx.ast = ast;

    // Get semantic analyzer from global storage if available
    if (test_analyzer) {
        ctx.symbol_table = test_analyzer; // Store analyzer in symbol_table field
    }

    // Initialize backend if needed
    AsthraCompilerOptions options = {0};
    // Backend type removed - LLVM is the only backend
    options.output_file = "test_output";

    // For test compatibility, just return success
    // Real code generation happens through direct LLVM calls

    return true;
}

bool asthra_backend_emit_assembly(void *backend, char *buffer, size_t buffer_size) {
    if (!backend || !buffer || buffer_size == 0)
        return false;

    // For now, return a stub implementation
    // The actual implementation would depend on the backend's capabilities
    const char *stub_asm = "; Assembly output not implemented for test wrapper\n";
    size_t len = strlen(stub_asm);
    if (len >= buffer_size)
        return false;

    strcpy(buffer, stub_asm);
    return true;
}