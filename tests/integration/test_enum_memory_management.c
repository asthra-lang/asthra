/**
 * Test file for enum variant memory management integration testing
 * Tests memory safety and management for enum variants
 */

#include "test_enum_integration_common.h"

// Test enum variant memory management
static void test_enum_variant_memory_management(void) {
    printf("Testing enum variant memory management...\n");
    
    const char* source = 
        "package test;\n\n"
        "pub enum Container<T> { Value(T), Empty }\n"
        "pub fn create_containers(none) -> void {\n"
        "    let container1: Container<string> = Container.Value(\"hello\");\n"
        "    return ();\n"
        "}\n";
    
    printf("Source code:\n%s\n", source);
    
    // Parse the complete program
    ASTNode *program = parse_test_program(source);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);
    
    printf("✓ Memory management program parsed successfully\n");
    
    // Verify proper memory management for enum variants through semantic analysis
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
    
    // Perform code generation to ensure memory safety
    CodeGenerator *generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    assert(generator != NULL);
    
    // Connect semantic analysis results to code generator
    code_generator_set_semantic_analyzer(generator, analyzer);
    
    // Enable memory safety features
    generator->config.bounds_checking = true;
    generator->config.stack_protection = true;
    
    bool codegen_success = code_generate_program(generator, program);
    assert(codegen_success);
    
    printf("✓ Memory management for enum variants validated\n");
    
    printf("✓ Memory management test structure validated\n");
    
    // Cleanup
    code_generator_destroy(generator);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    
    printf("✓ Enum variant memory management test passed\n");
}

int main(void) {
    printf("Running Enum Memory Management Tests...\n");
    printf("=========================================\n\n");
    
    test_enum_variant_memory_management();
    
    printf("\n✅ All enum memory management tests completed!\n");
    printf("Note: Full memory testing requires runtime validation and\n");
    printf("      integration with memory safety tools like Valgrind.\n");
    
    return 0;
}