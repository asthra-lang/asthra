/**
 * Test file for Option type integration testing
 * Tests Option<T> enum usage patterns
 */

#include "../framework/backend_stubs.h"
#include "test_enum_integration_common.h"

// Test Option type usage patterns
static void test_option_type_integration(void) {
    printf("Testing Option type integration...\n");

    const char *source = "package test;\n\n"
                         "pub fn test_option_types(none) -> void {\n"
                         "    // Test that Option types can be declared\n"
                         "    let opt1: Option<i32>;\n"
                         "    let opt2: Option<string>;\n"
                         "    let opt3: Option<bool>;\n"
                         "    // Nested Option types\n"
                         "    let nested: Option<Option<i32>>;\n"
                         "    // Test Option constructors with type inference\n"
                         "    let some_val: Option<i32> = Option.Some(42);\n"
                         "    let none_val: Option<i32> = Option.None;\n"
                         "    let string_some: Option<string> = Option.Some(\"test\");\n"
                         "    return ();\n"
                         "}\n"
                         "pub fn test_option_returns(none) -> Option<i32> {\n"
                         "    return Option.Some(999);\n"
                         "}\n";

    printf("Source code:\n%s\n", source);

    // Parse the complete program
    ASTNode *program = parse_test_program(source);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);

    printf("✓ Option program parsed successfully\n");

    // Verify that the functions were parsed
    assert(program->data.program.declarations != NULL);
    assert(ast_node_list_size(program->data.program.declarations) == 2);

    ASTNode *func1 = ast_node_list_get(program->data.program.declarations, 0);
    assert(func1 != NULL);
    assert(func1->type == AST_FUNCTION_DECL);
    assert(strcmp(func1->data.function_decl.name, "test_option_types") == 0);

    ASTNode *func2 = ast_node_list_get(program->data.program.declarations, 1);
    assert(func2 != NULL);
    assert(func2->type == AST_FUNCTION_DECL);
    assert(strcmp(func2->data.function_decl.name, "test_option_returns") == 0);

    // Verify Option.Some and Option.None constructions through semantic analysis
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

    // Perform code generation to ensure Option constructions work
    void *backend = asthra_backend_create(NULL);
    assert(backend != NULL);

    // Connect semantic analysis results to backend
    asthra_backend_set_semantic_analyzer(backend, analyzer);

    bool codegen_success = asthra_backend_generate_program(backend, program);
    assert(codegen_success);

    printf("✓ Option.Some and Option.None constructions with type inference validated\n");

    printf("✓ Option integration test structure validated\n");

    // Cleanup
    asthra_backend_destroy(backend);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);

    printf("✓ Option type integration test passed\n");
}

int main(void) {
    printf("Running Option Type Integration Tests...\n");
    printf("=========================================\n\n");

    test_option_type_integration();

    printf("\n✅ All Option type integration tests completed!\n");

    return 0;
}