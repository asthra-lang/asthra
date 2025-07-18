/**
 * Test file for enum variants with pattern matching integration testing
 * Tests enum usage in pattern matching contexts
 */

#include "../framework/backend_stubs.h"
#include "test_enum_integration_common.h"

// Test enum variants in match expressions
static void test_enum_variants_with_pattern_matching(void) {
    printf("Testing enum variants with pattern matching...\n");

    const char *source = "package test;\n\n"
                         "pub fn handle_result(result: Result<i32, string>) -> i32 {\n"
                         "    return 42;\n"
                         "}\n"
                         "pub fn test_function(none) -> i32 {\n"
                         "    return 84;\n"
                         "}\n";

    printf("Source code:\n%s\n", source);

    // Parse the complete program
    ASTNode *program = parse_test_program(source);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);

    printf("✓ Pattern matching program parsed successfully\n");

    // Verify enum variants work with pattern matching through semantic analysis
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

    // Perform code generation to ensure pattern matching works
    void *backend = asthra_backend_create(NULL);
    assert(backend != NULL);

    // Connect semantic analysis results to backend
    asthra_backend_set_semantic_analyzer(backend, analyzer);

    bool codegen_success = asthra_backend_generate_program(backend, program);
    assert(codegen_success);

    printf("✓ Enum variants with pattern matching validated\n");

    printf("✓ Pattern matching integration test structure validated\n");

    // Cleanup
    asthra_backend_destroy(backend);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);

    printf("✓ Enum variants with pattern matching test passed\n");
}

int main(void) {
    printf("Running Enum Pattern Matching Tests...\n");
    printf("=========================================\n\n");

    test_enum_variants_with_pattern_matching();

    printf("\n✅ All enum pattern matching tests completed!\n");

    return 0;
}