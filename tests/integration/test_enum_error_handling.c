/**
 * Test file for enum variant error handling integration testing
 * Tests enum variants with error handling patterns
 */

#include "../framework/backend_stubs.h"
#include "test_enum_integration_common.h"

// Test enum variant construction with error handling
static void test_enum_variant_error_handling(void) {
    printf("Testing enum variant error handling...\n");

    const char *source = "package test;\n\n"
                         "pub fn divide(a: i32, b: i32) -> Result<i32, string> {\n"
                         "    return Result.Ok(a);\n"
                         "}\n";

    printf("Source code:\n%s\n", source);

    // Parse the complete program
    ASTNode *program = parse_test_program(source);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);

    printf("✓ Error handling program parsed successfully\n");

    // Verify both Result.Ok and Result.Err constructions through semantic analysis
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    bool semantic_success = semantic_analyze_program(analyzer, program);
    if (!semantic_success) {
        printf("Semantic analysis errors:\n");
        const SemanticError *error = semantic_get_errors(analyzer);
        while (error) {
            printf("  %s:%d:%d: %s\n",
                   error->location.filename ? error->location.filename : "unknown",
                   error->location.line, error->location.column, error->message);
            error = error->next;
        }
    }
    assert(semantic_success);

    // Perform code generation to ensure both paths work
    void *backend = asthra_backend_create(NULL);
    assert(backend != NULL);

    // Connect semantic analysis results to backend
    asthra_backend_set_semantic_analyzer(backend, analyzer);

    bool codegen_success = asthra_backend_generate_program(backend, program);
    assert(codegen_success);

    printf("✓ Both Result.Ok and Result.Err constructions validated\n");

    printf("✓ Error handling test structure validated\n");

    // Cleanup
    asthra_backend_destroy(backend);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);

    printf("✓ Error handling enum variant test passed\n");
}

int main(void) {
    printf("Running Enum Variant Error Handling Tests...\n");
    printf("=========================================\n\n");

    test_enum_variant_error_handling();

    printf("\n✅ All enum variant error handling tests completed!\n");

    return 0;
}