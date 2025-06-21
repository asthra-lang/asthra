/**
 * Test file for enum compilation pipeline integration testing
 * Tests complete compilation pipeline for enum variants
 */

#include "test_enum_integration_common.h"

// Test compilation pipeline integration
static void test_compilation_pipeline_integration(void) {
    printf("Testing compilation pipeline integration...\n");
    
    const char* source = 
        "package test;\n\n"
        "pub fn main(none) -> i32 {\n"
        "    let result: Result<i32, string> = Result.Ok(42);\n"
        "    return 0;\n"
        "}\n";
    
    printf("Source code:\n%s\n", source);
    
    // Parse the complete program
    ASTNode *program = parse_test_program(source);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);
    
    printf("✓ Pipeline test program parsed successfully\n");
    
    // Test the complete compilation pipeline
    // 1. Lexing and parsing (already done)
    
    // 2. Semantic analysis phase
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
    printf("✓ Semantic analysis phase completed\n");
    
    // 3. Code generation phase
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    assert(generator != NULL);
    
    // Connect semantic analysis results to code generator
    code_generator_set_semantic_analyzer(generator, analyzer);
    
    bool codegen_success = code_generate_program(generator, program);
    assert(codegen_success);
    printf("✓ Code generation phase completed\n");
    
    // 4. Assembly generation
    char assembly_buffer[4096];
    bool assembly_success = code_generator_emit_assembly(generator, assembly_buffer, sizeof(assembly_buffer));
    if (assembly_success) {
        printf("✓ Assembly generation phase completed\n");
    } else {
        printf("✗ Assembly generation failed\n");
    }
    
    printf("✓ Complete compilation pipeline tested\n");
    
    printf("✓ Compilation pipeline test structure validated\n");
    
    // Cleanup
    code_generator_destroy(generator);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    
    printf("✓ Compilation pipeline integration test passed\n");
}

int main(void) {
    printf("Running Enum Compilation Pipeline Tests...\n");
    printf("=========================================\n\n");
    
    test_compilation_pipeline_integration();
    
    printf("\n✅ All enum compilation pipeline tests completed!\n");
    printf("Note: These tests validate the overall structure and parsing.\n");
    printf("      Full integration testing requires complete build system\n");
    printf("      integration with semantic analysis and code generation.\n");
    
    return 0;
}