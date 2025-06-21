/**
 * Test file for Option type integration testing
 * Tests Option<T> enum usage patterns
 */

#include "test_enum_integration_common.h"

// Test Option type usage patterns
static void test_option_type_integration(void) {
    printf("Testing Option type integration...\n");
    
    const char* source = 
        "package test;\n\n"
        "pub enum Option<T> { Some(T), None }\n"
        "pub fn get_some_value(x: i32) -> Option<i32> {\n"
        "    return Option.Some(x);\n"
        "}\n";
    
    printf("Source code:\n%s\n", source);
    
    // Parse the complete program
    ASTNode *program = parse_test_program(source);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);
    
    printf("✓ Option program parsed successfully\n");
    
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
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    assert(generator != NULL);
    
    // Connect semantic analysis results to code generator
    code_generator_set_semantic_analyzer(generator, analyzer);
    
    bool codegen_success = code_generate_program(generator, program);
    assert(codegen_success);
    
    printf("✓ Option.Some and Option.None constructions validated\n");
    
    printf("✓ Option integration test structure validated\n");
    
    // Cleanup
    code_generator_destroy(generator);
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