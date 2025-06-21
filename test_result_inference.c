/**
 * Simple test for Result type inference fix
 */

#include "tests/framework/test_framework.h"
#include "tests/framework/semantic_test_utils.h"
#include <stdio.h>

int main(void) {
    printf("Testing Result type inference...\n");
    
    const char* test_source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let result: Result<i32, string> = Result.Ok(42);\n"
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("❌ Failed to create semantic analyzer\n");
        return 1;
    }

    ASTNode* ast = parse_test_source(test_source, "result_inference_test");
    if (!ast) {
        printf("❌ Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return 1;
    }

    printf("✅ Parsed successfully, starting semantic analysis...\n");
    
    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("❌ Semantic analysis failed:\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    } else {
        printf("✅ Semantic analysis succeeded!\n");
        printf("✅ Result type inference is working correctly\n");
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success ? 0 : 1;
}