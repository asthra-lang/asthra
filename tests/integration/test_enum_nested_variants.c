/**
 * Test file for nested enum variants integration testing
 * Tests complex enum nesting patterns
 */

#include "../framework/backend_stubs.h"
#include "test_enum_integration_common.h"

// Test nested enum variants
static void test_nested_enum_variants(void) {
    printf("Testing nested enum variants...\n");

    const char *source = "package test;\n\n"
                         "pub fn complex_operation(none) -> Result<Option<i32>, string> {\n"
                         "    let maybe_value: Option<i32> = Option.Some(42);\n"
                         "    return Result.Ok(maybe_value);\n"
                         "}\n";

    printf("Source code:\n%s\n", source);

    // Parse the complete program
    ASTNode *program = parse_test_program(source);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);

    printf("✓ Nested enum program parsed successfully\n");

    // Verify nested enum variant constructions through semantic analysis
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

    // Perform code generation to ensure nested constructions work
    void *backend = asthra_backend_create(NULL);
    assert(backend != NULL);

    // Connect semantic analysis results to backend
    asthra_backend_set_semantic_analyzer(backend, analyzer);

    bool codegen_success = asthra_backend_generate_program(backend, program);
    assert(codegen_success);

    printf("✓ Nested enum variant constructions validated\n");

    printf("✓ Nested enum variants test structure validated\n");

    // Cleanup
    asthra_backend_destroy(backend);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);

    printf("✓ Nested enum variants test passed\n");
}

int main(void) {
    printf("Running Nested Enum Variants Tests...\n");
    printf("=========================================\n\n");

    test_nested_enum_variants();

    printf("\n✅ All nested enum variants tests completed!\n");

    return 0;
}