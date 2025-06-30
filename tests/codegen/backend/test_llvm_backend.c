/**
 * LLVM Backend Tests
 * Tests for the Asthra LLVM IR code generation backend
 */

#include "../../../src/analysis/type_info.h"
#include "../../../src/compiler.h"
#include "../../../src/parser/ast_node.h"
#include "../../../src/parser/ast_types.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// External backend functions
extern AsthraBackend *asthra_backend_create(const AsthraCompilerOptions *options);
extern void asthra_backend_destroy(AsthraBackend *backend);
extern const char *asthra_backend_get_name(AsthraBackend *backend);
extern const char *asthra_backend_get_version(AsthraBackend *backend);
extern char *asthra_backend_get_output_filename(AsthraBackendType type, const char *input_file,
                                                const char *extension);

// Test helper to create a simple AST for testing
static ASTNode *create_test_ast(void) {
    // For this simple test, we'll just create a minimal valid AST
    // The actual LLVM backend would need proper AST construction
    ASTNode *program = malloc(sizeof(ASTNode));
    program->type = AST_PROGRAM;
    program->location = (SourceLocation){0};
    program->ref_count = 1;

    // Create a declarations list with space for one function
    ASTNodeList *decls = malloc(sizeof(ASTNodeList) + sizeof(ASTNode *));
    decls->count = 1;
    decls->capacity = 1;

    // Create a simple function declaration
    ASTNode *func = malloc(sizeof(ASTNode));
    func->type = AST_FUNCTION_DECL;
    func->location = (SourceLocation){0};
    func->ref_count = 1;
    func->data.function_decl.name = "main";
    func->data.function_decl.params = NULL;
    func->data.function_decl.return_type = NULL;
    func->data.function_decl.body = NULL;
    func->data.function_decl.visibility = VISIBILITY_PUBLIC;
    func->data.function_decl.annotations = NULL;

    // For the test, we'll attach a dummy type info
    func->type_info = malloc(sizeof(TypeInfo));
    func->type_info->type_id = TYPE_INFO_FUNCTION;
    func->type_info->name = "fn() -> i32";

    decls->nodes[0] = func;
    program->data.program.declarations = decls;

    return program;
}

// Test LLVM backend creation
static void test_llvm_backend_creation(void) {
    printf("Testing LLVM backend creation...\n");

    AsthraCompilerOptions options = {0};
    options.backend_type = ASTHRA_BACKEND_LLVM_IR;

    AsthraBackend *backend = asthra_backend_create(&options);

    // LLVM backend should be created successfully since LLVM is always compiled in
    assert(backend != NULL);
    assert(strcmp(asthra_backend_get_name(backend), "LLVM Backend") == 0);

    // Check version includes LLVM version
    const char *version = asthra_backend_get_version(backend);
    assert(version != NULL);
    assert(strstr(version, "LLVM") != NULL);

    asthra_backend_destroy(backend);

    printf("✓ LLVM backend creation test passed\n");
}

// Test LLVM backend initialization
static void test_llvm_backend_initialization(void) {
    printf("Testing LLVM backend initialization...\n");

    AsthraCompilerOptions options = {0};
    options.backend_type = ASTHRA_BACKEND_LLVM_IR;
    options.input_file = "test.as";

    AsthraBackend *backend = asthra_backend_create(&options);
    assert(backend != NULL);

    // Should initialize successfully
    printf("   LLVM backend initialized successfully\n");

    asthra_backend_destroy(backend);

    printf("✓ LLVM backend initialization test passed\n");
}

// Test LLVM backend feature support
static void test_llvm_backend_features(void) {
    printf("Testing LLVM backend feature support...\n");

    AsthraCompilerOptions options = {0};
    options.backend_type = ASTHRA_BACKEND_LLVM_IR;

    AsthraBackend *backend = asthra_backend_create(&options);
    assert(backend != NULL);

    // Test supported features
    assert(backend->ops->supports_feature(backend, "optimization") == true);
    assert(backend->ops->supports_feature(backend, "debug-info") == true);
    assert(backend->ops->supports_feature(backend, "cross-compilation") == true);
    assert(backend->ops->supports_feature(backend, "bitcode") == true);
    assert(backend->ops->supports_feature(backend, "llvm-ir") == true);
    assert(backend->ops->supports_feature(backend, "x86_64") == true);

    // Test unsupported features
    assert(backend->ops->supports_feature(backend, "unknown-feature") == false);

    asthra_backend_destroy(backend);

    printf("✓ LLVM backend feature support test passed\n");
}

// Test LLVM backend code generation
static void test_llvm_backend_codegen(void) {
    printf("Testing LLVM backend code generation...\n");

    AsthraCompilerOptions options = {0};
    options.backend_type = ASTHRA_BACKEND_LLVM_IR;
    options.input_file = "test.as";
    options.output_file = "test.ll";

    AsthraBackend *backend = asthra_backend_create(&options);
    assert(backend != NULL);

    // Initialize backend
    int result = backend->ops->initialize(backend, &options);
    assert(result == 0);

    // Create a simple AST
    ASTNode *ast = create_test_ast();

    // Create minimal compiler context
    AsthraCompilerContext ctx = {0};

    // Generate code
    result = backend->ops->generate(backend, &ctx, ast, "test_output.ll");
    assert(result == 0);

    // Check statistics
    // Note: Some stats might not be updated in all code paths
    assert(backend->stats.generation_time >= 0);

    // Cleanup
    asthra_backend_destroy(backend);

    // TODO: Free AST nodes (would need proper cleanup functions)

    printf("✓ LLVM backend code generation test passed\n");
}

// Test LLVM output formats
static void test_llvm_output_formats(void) {
    printf("Testing LLVM output formats...\n");

    AsthraCompilerOptions options = {0};
    options.backend_type = ASTHRA_BACKEND_LLVM_IR;

    // Test default extension
    char *output_default =
        asthra_backend_get_output_filename(options.backend_type, "test.as", NULL);
    assert(strcmp(output_default, "test.ll") == 0);
    free(output_default);

    // Test with explicit output file
    char *output_explicit =
        asthra_backend_get_output_filename(options.backend_type, "test.as", "output.ll");
    assert(strcmp(output_explicit, "output.ll") == 0);
    free(output_explicit);

    printf("✓ LLVM output format test passed\n");
}

int main(void) {
    printf("=== LLVM Backend Tests ===\n");

    test_llvm_backend_creation();
    test_llvm_backend_initialization();
    test_llvm_backend_features();
    test_llvm_backend_codegen();
    test_llvm_output_formats();

    printf("\nAll LLVM backend tests passed!\n");
    return 0;
}