/**
 * Simple test to verify Never type is working
 */

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "semantic_analyzer.h"

int main(void) {
    printf("=== Simple Never Type Test ===\n");
    
    const char* test_source = 
        "package test;\n"
        "pub fn simple_never(none) -> Never {\n"
        "    return ();\n"
        "}\n";
        
    Parser* parser = create_test_parser(test_source);
    if (!parser) {
        printf("Failed to create parser\n");
        return 1;
    }
    
    ASTNode* ast = parser_parse_program(parser);
    if (!ast) {
        printf("Failed to parse program\n");
        destroy_test_parser(parser);
        return 1;
    }
    
    printf("Parse successful!\n");
    
    SemanticAnalyzer* analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        ast_free_node(ast);
        destroy_test_parser(parser);
        return 1;
    }
    
    bool semantic_result = semantic_analyze_program(analyzer, ast);
    printf("Semantic analysis result: %s\n", semantic_result ? "SUCCESS" : "FAILURE");
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);
    destroy_test_parser(parser);
    
    return semantic_result ? 0 : 1;
}