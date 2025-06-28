/**
 * Codegen Backend Test Wrapper - Implementation
 * 
 * Implements simplified wrapper functions around AsthraBackend
 * to facilitate migration from old CodeGenerator API.
 */

#include "codegen_backend_wrapper.h"
#include "compiler.h"
#include <stdlib.h>
#include <string.h>

// Store semantic analyzer in backend's private data for tests
typedef struct {
    SemanticAnalyzer *analyzer;
    void *original_private_data;
} BackendTestData;

void asthra_backend_set_semantic_analyzer(AsthraBackend *backend, SemanticAnalyzer *analyzer) {
    if (!backend) return;
    
    // Allocate test data if not already present
    BackendTestData *test_data = (BackendTestData *)backend->private_data;
    if (!test_data) {
        test_data = calloc(1, sizeof(BackendTestData));
        if (!test_data) return;
        test_data->original_private_data = backend->private_data;
        backend->private_data = test_data;
    }
    
    test_data->analyzer = analyzer;
}

bool asthra_backend_generate_program(AsthraBackend *backend, ASTNode *ast) {
    if (!backend || !ast) return false;
    
    // Create a minimal compiler context for the test
    AsthraCompilerContext ctx = {0};
    ctx.input_file = "test.asthra";
    ctx.ast = ast;
    
    // Get semantic analyzer from test data if available
    BackendTestData *test_data = (BackendTestData *)backend->private_data;
    if (test_data && test_data->analyzer) {
        ctx.semantic_analyzer = test_data->analyzer;
    }
    
    // Initialize backend if needed
    AsthraCompilerOptions options = {0};
    options.backend_type = backend->type;
    options.output_file = "test_output";
    
    if (backend->ops && backend->ops->initialize) {
        int init_result = backend->ops->initialize(backend, &options);
        if (init_result != 0) return false;
    }
    
    // Generate code
    if (backend->ops && backend->ops->generate) {
        int result = backend->ops->generate(backend, &ctx, ast, "test_output");
        return result == 0;
    }
    
    return false;
}

bool asthra_backend_emit_assembly(AsthraBackend *backend, char *buffer, size_t buffer_size) {
    if (!backend || !buffer || buffer_size == 0) return false;
    
    // For now, return a stub implementation
    // The actual implementation would depend on the backend's capabilities
    const char *stub_asm = "; Assembly output not implemented for test wrapper\n";
    size_t len = strlen(stub_asm);
    if (len >= buffer_size) return false;
    
    strcpy(buffer, stub_asm);
    return true;
}

bool asthra_backend_generate_to_file(AsthraBackend *backend, ASTNode *ast, const char *filename) {
    if (!backend || !ast || !filename) return false;
    
    // Create a compiler context for file generation
    AsthraCompilerContext ctx = {0};
    ctx.input_file = "test.asthra";
    ctx.ast = ast;
    
    // Get semantic analyzer from test data if available
    BackendTestData *test_data = (BackendTestData *)backend->private_data;
    if (test_data && test_data->analyzer) {
        ctx.semantic_analyzer = test_data->analyzer;
    }
    
    // Initialize backend if needed
    AsthraCompilerOptions options = {0};
    options.backend_type = backend->type;
    options.output_file = filename;
    
    if (backend->ops && backend->ops->initialize) {
        int init_result = backend->ops->initialize(backend, &options);
        if (init_result != 0) return false;
    }
    
    // Generate code to file
    if (backend->ops && backend->ops->generate) {
        int result = backend->ops->generate(backend, &ctx, ast, filename);
        return result == 0;
    }
    
    return false;
}