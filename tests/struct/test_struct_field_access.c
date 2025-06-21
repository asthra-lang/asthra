/**
 * Test struct field access functionality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic_analyzer.h"

// Function prototypes
static void test_basic_struct_field_access(void);
static void test_struct_field_access_with_invalid_field(void);
static void test_struct_field_access_on_non_struct(void);

void test_basic_struct_field_access(void) {
    printf("Testing basic struct field access...\n");
    
    const char *source = 
        "package test;\n"
        "priv struct Point {\n"
        "    x: f64,\n"
        "    y: f64\n"
        "}\n"
        "priv fn main(none) -> void {\n"
        "    let p: Point = Point { x: 1.0, y: 2.0 };\n"
        "    let x_val: f64 = p.x;\n"
        "    let y_val: f64 = p.y;\n"
        "    return ();\n"
        "}\n";
    
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    assert(lexer != NULL);
    
    Parser *parser = parser_create(lexer);
    assert(parser != NULL);
    
    ASTNode *program = parser_parse_program(parser);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);
    
    // Create semantic analyzer
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);
    
    // Analyze the program
    bool analysis_result = semantic_analyze_program(analyzer, program);
    
    // Check for errors
    size_t error_count = semantic_get_error_count(analyzer);
    if (error_count > 0) {
        printf("Semantic analysis errors:\n");
        const SemanticError *error = semantic_get_errors(analyzer);
        while (error) {
            printf("  Error: %s\n", error->message);
            error = error->next;
        }
    }
    
    assert(analysis_result == true);
    assert(error_count == 0);
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    printf("✅ Basic struct field access test passed!\n");
}

void test_struct_field_access_with_invalid_field(void) {
    printf("Testing struct field access with invalid field...\n");
    
    const char *source = 
        "package test;\n"
        "priv struct Point {\n"
        "    x: f64,\n"
        "    y: f64\n"
        "}\n"
        "priv fn main(none) -> void {\n"
        "    let p: Point = Point { x: 1.0, y: 2.0 };\n"
        "    let z_val: f64 = p.z;\n"  // Invalid field 'z'
        "    return ();\n"
        "}\n";
    
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    assert(lexer != NULL);
    
    Parser *parser = parser_create(lexer);
    assert(parser != NULL);
    
    ASTNode *program = parser_parse_program(parser);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);
    
    // Create semantic analyzer
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);
    
    // Analyze the program - should fail
    bool analysis_result = semantic_analyze_program(analyzer, program);
    
    // Should have errors
    size_t error_count = semantic_get_error_count(analyzer);
    
    assert(error_count > 0);
    assert(analysis_result == false);
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    printf("✅ Invalid field access test passed!\n");
}

void test_struct_field_access_on_non_struct(void) {
    printf("Testing struct field access on non-struct type...\n");
    
    const char *source = 
        "package test;\n"
        "priv fn main(none) -> void {\n"
        "    let x: i32 = 42;\n"
        "    let val: i32 = x.field;\n"  // Invalid - i32 is not a struct
        "    return ();\n"
        "}\n";
    
    Lexer *lexer = lexer_create(source, strlen(source), "test.asthra");
    assert(lexer != NULL);
    
    Parser *parser = parser_create(lexer);
    assert(parser != NULL);
    
    ASTNode *program = parser_parse_program(parser);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);
    
    // Create semantic analyzer
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);
    
    // Analyze the program - should fail
    bool analysis_result = semantic_analyze_program(analyzer, program);
    
    // Should have errors
    size_t error_count = semantic_get_error_count(analyzer);
    assert(error_count > 0);
    assert(analysis_result == false);
    
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    printf("✅ Non-struct field access test passed!\n");
}

int main(void) {
    printf("Running struct field access tests...\n\n");
    
    test_basic_struct_field_access();
    test_struct_field_access_with_invalid_field();
    test_struct_field_access_on_non_struct();
    
    printf("\n✅ All struct field access tests passed!\n");
    return 0;
} 
