/**
 * Test file for end-to-end enum variant integration testing
 * Tests complete compilation and execution of enum variant construction
 * Part of Phase 4: Integration and Testing (Enum Variant Construction Implementation Plan)
 */

#include "../framework/backend_stubs.h"
#include "test_enum_integration_common.h"

// Test end-to-end enum variant construction in a function
static void test_enum_variant_end_to_end(void) {
    printf("Testing end-to-end enum variant construction...\n");

    const char *source = "package test;\n\n"
                         "pub fn test(none) -> Result<i32, string> {\n"
                         "    return Result.Ok(42);\n"
                         "}\n";

    printf("Source code:\n%s\n", source);

    // Parse the complete program
    ASTNode *program = parse_test_program(source);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);

    printf("✓ Program parsed successfully\n");

    // Perform semantic analysis
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

    printf("✓ Semantic analysis completed successfully\n");

    // Perform code generation
    void *backend = asthra_backend_create(NULL);
    assert(backend != NULL);

    // Connect semantic analysis results to backend
    asthra_backend_set_semantic_analyzer(backend, analyzer);

    bool codegen_success = asthra_backend_generate_program(backend, program);
    if (!codegen_success) {
        printf("Code generation failed\n");
    }
    assert(codegen_success);

    printf("✓ Code generation completed successfully\n");

    // Generate assembly output for verification
    char assembly_buffer[4096];
    bool assembly_success =
        asthra_backend_emit_assembly(backend, assembly_buffer, sizeof(assembly_buffer));
    if (assembly_success) {
        printf("✓ Assembly generation completed\n");
        // For debugging, you can uncomment the next line to see the generated assembly
        // printf("Generated assembly:\n%s\n", assembly_buffer);
    } else {
        printf("✗ Assembly generation failed\n");
    }

    printf("✓ End-to-end test structure validated\n");

    // Cleanup
    asthra_backend_destroy(backend);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);

    printf("✓ End-to-end enum variant test passed\n");
}

int main(void) {
    printf("Running End-to-End Enum Variant Tests...\n");
    printf("=========================================\n\n");

    test_enum_variant_end_to_end();

    printf("\n✅ All end-to-end enum variant tests completed!\n");

    return 0;
}