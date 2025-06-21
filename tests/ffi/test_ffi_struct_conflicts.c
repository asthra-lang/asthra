/**
 * Test for FFI Struct Syntax Conflicts
 * This test specifically checks for parsing conflicts between struct declarations
 * and FFI extern declarations that might cause parsing failures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

// Test cases that might cause struct syntax conflicts
static void test_struct_with_ffi_annotations(void);
static void test_extern_with_struct_parameters(void);
static void test_mixed_struct_and_extern_declarations(void);
static void test_struct_field_ffi_annotations(void);

int main(void) {
    printf("Testing FFI struct syntax conflicts...\n");
    
    test_struct_with_ffi_annotations();
    test_extern_with_struct_parameters();
    test_mixed_struct_and_extern_declarations();
    test_struct_field_ffi_annotations();
    
    printf("All FFI struct syntax conflict tests passed!\n");
    return 0;
}

static void test_struct_with_ffi_annotations(void) {
    printf("Testing struct declarations with FFI annotations...\n");
    
    const char *test_code = 
        "package test;\n"
        "#[ownership(c)]\n"
        "priv struct CCompatStruct {\n"
        "    field1: i32,\n"
        "    field2: *const u8\n"
        "}\n"
        "\n"
        "#[ownership(pinned)]\n"
        "priv struct PinnedStruct {\n"
        "    buffer: []u8\n"
        "}\n";
    
    Lexer *lexer = lexer_create(test_code, strlen(test_code), "test.asthra");
    assert(lexer != NULL);
    
    Parser *parser = parser_create(lexer);
    assert(parser != NULL);
    
    ASTNode *program = parser_parse_program(parser);
    if (!program) {
        fprintf(stderr, "FAILED: Could not parse struct with FFI annotations\n");
        if (parser_had_error(parser)) {
            const ParseError *errors = parser_get_errors(parser);
            while (errors) {
                fprintf(stderr, "Error: %s\n", errors->message);
                errors = errors->next;
            }
        }
        parser_destroy(parser);
        lexer_destroy(lexer);
        assert(false);
    }
    
    assert(program->type == AST_PROGRAM);
    
    // Verify we have struct declarations
    ASTNodeList *declarations = program->data.program.declarations;
    assert(declarations != NULL);
    assert(ast_node_list_size(declarations) == 2);
    
    // Check first struct declaration
    ASTNode *struct1 = ast_node_list_get(declarations, 0);
    assert(struct1->type == AST_STRUCT_DECL);
    assert(strcmp(struct1->data.struct_decl.name, "CCompatStruct") == 0);
    
    // Check second struct declaration
    ASTNode *struct2 = ast_node_list_get(declarations, 1);
    assert(struct2->type == AST_STRUCT_DECL);
    assert(strcmp(struct2->data.struct_decl.name, "PinnedStruct") == 0);
    
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    printf("✓ Struct declarations with FFI annotations parsed successfully\n");
}

static void test_extern_with_struct_parameters(void) {
    printf("Testing extern declarations with struct parameters...\n");
    
    const char *test_code = 
        "package test;\n"
        "priv struct Point {\n"
        "    x: f64,\n"
        "    y: f64\n"
        "}\n"
        "\n"
        "pub extern \"graphics\" fn draw_point(\n"
        "    #[transfer_none] point: Point,\n"
        "    #[borrowed] color: *const u8\n"
        ") -> void;\n"
        "\n"
        "pub extern \"math\" fn distance(\n"
        "    #[borrowed] p1: *const Point,\n"
        "    #[borrowed] p2: *const Point\n"
        ") -> f64;\n";
    
    Lexer *lexer = lexer_create(test_code, strlen(test_code), "test.asthra");
    assert(lexer != NULL);
    
    Parser *parser = parser_create(lexer);
    assert(parser != NULL);
    
    ASTNode *program = parser_parse_program(parser);
    if (!program) {
        fprintf(stderr, "FAILED: Could not parse extern with struct parameters\n");
        if (parser_had_error(parser)) {
            const ParseError *errors = parser_get_errors(parser);
            while (errors) {
                fprintf(stderr, "Error: %s\n", errors->message);
                errors = errors->next;
            }
        }
        parser_destroy(parser);
        lexer_destroy(lexer);
        assert(false);
    }
    
    assert(program->type == AST_PROGRAM);
    
    // Verify we have struct and extern declarations
    ASTNodeList *declarations = program->data.program.declarations;
    assert(declarations != NULL);
    assert(ast_node_list_size(declarations) == 3);
    
    // Check struct declaration
    ASTNode *struct_decl = ast_node_list_get(declarations, 0);
    assert(struct_decl->type == AST_STRUCT_DECL);
    assert(strcmp(struct_decl->data.struct_decl.name, "Point") == 0);
    
    // Check first extern declaration
    ASTNode *extern1 = ast_node_list_get(declarations, 1);
    assert(extern1->type == AST_EXTERN_DECL);
    assert(strcmp(extern1->data.extern_decl.name, "draw_point") == 0);
    assert(strcmp(extern1->data.extern_decl.extern_name, "graphics") == 0);
    
    // Check second extern declaration
    ASTNode *extern2 = ast_node_list_get(declarations, 2);
    assert(extern2->type == AST_EXTERN_DECL);
    assert(strcmp(extern2->data.extern_decl.name, "distance") == 0);
    assert(strcmp(extern2->data.extern_decl.extern_name, "math") == 0);
    
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    printf("✓ Extern declarations with struct parameters parsed successfully\n");
}

static void test_mixed_struct_and_extern_declarations(void) {
    printf("Testing mixed struct and extern declarations...\n");
    
    // Simplified test to isolate the parsing issue
    const char *test_code = 
        "package test;\n"
        "priv struct Buffer {\n"
        "    data: *mut u8\n"
        "}\n"
        "pub extern \"libc\" fn malloc(size: usize) -> void;\n";
    
    Lexer *lexer = lexer_create(test_code, strlen(test_code), "test.asthra");
    assert(lexer != NULL);
    
    Parser *parser = parser_create(lexer);
    assert(parser != NULL);
    
    ASTNode *program = parser_parse_program(parser);
    if (!program) {
        fprintf(stderr, "FAILED: Could not parse simplified mixed declarations\n");
        if (parser_had_error(parser)) {
            const ParseError *errors = parser_get_errors(parser);
            while (errors) {
                fprintf(stderr, "Error at %s:%d:%d: %s\n", 
                       "test.asthra", errors->location.line, errors->location.column, errors->message);
                errors = errors->next;
            }
        }
        parser_destroy(parser);
        lexer_destroy(lexer);
        assert(false);
    }
    
    assert(program->type == AST_PROGRAM);
    
    // Verify we have both declarations
    ASTNodeList *declarations = program->data.program.declarations;
    assert(declarations != NULL);
    assert(ast_node_list_size(declarations) == 2);
    
    // Check declaration types
    assert(ast_node_list_get(declarations, 0)->type == AST_STRUCT_DECL);  // Buffer
    assert(ast_node_list_get(declarations, 1)->type == AST_EXTERN_DECL);  // malloc
    
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    // Now test with FFI annotations
    printf("Testing with FFI annotations...\n");
    
    const char *test_code2 = 
        "package test;\n"
        "priv struct Buffer {\n"
        "    data: *mut u8\n"
        "}\n"
        "pub extern \"libc\" fn malloc(size: usize) -> #[transfer_full] *mut void;\n"
        "#[ownership(c)]\n"
        "priv struct CBuffer {\n"
        "    ptr: *mut u8\n"
        "}\n";
    
    Lexer *lexer2 = lexer_create(test_code2, strlen(test_code2), "test.asthra");
    assert(lexer2 != NULL);
    
    Parser *parser2 = parser_create(lexer2);
    assert(parser2 != NULL);
    
    ASTNode *program2 = parser_parse_program(parser2);
    if (!program2) {
        fprintf(stderr, "FAILED: Could not parse mixed struct and extern declarations with annotations\n");
        if (parser_had_error(parser2)) {
            const ParseError *errors = parser_get_errors(parser2);
            while (errors) {
                fprintf(stderr, "Error at %s:%d:%d: %s\n", 
                       "test.asthra", errors->location.line, errors->location.column, errors->message);
                errors = errors->next;
            }
        }
        parser_destroy(parser2);
        lexer_destroy(lexer2);
        assert(false);
    }
    
    assert(program2->type == AST_PROGRAM);
    
    // Verify we have all declarations
    ASTNodeList *declarations2 = program2->data.program.declarations;
    assert(declarations2 != NULL);
    assert(ast_node_list_size(declarations2) == 3);
    
    // Check declaration types in order
    assert(ast_node_list_get(declarations2, 0)->type == AST_STRUCT_DECL);  // Buffer
    assert(ast_node_list_get(declarations2, 1)->type == AST_EXTERN_DECL);  // malloc
    assert(ast_node_list_get(declarations2, 2)->type == AST_STRUCT_DECL);  // CBuffer
    
    ast_free_node(program2);
    parser_destroy(parser2);
    lexer_destroy(lexer2);
    printf("✓ Mixed struct and extern declarations parsed successfully\n");
}

static void test_struct_field_ffi_annotations(void) {
    printf("Testing struct fields with potential FFI annotation conflicts...\n");
    
    // This test checks for potential conflicts between struct field syntax
    // and FFI parameter annotation syntax
    const char *test_code = 
        "package test;\n"
        "priv struct FFIStruct {\n"
        "    field1: i32,\n"
        "    field2: *const u8,\n"
        "    field3: *mut void\n"
        "}\n"
        "\n"
        "pub extern \"test\" fn process_struct(\n"
        "    #[transfer_none] input: FFIStruct,\n"
        "    #[borrowed] output: *mut FFIStruct\n"
        ") -> i32;\n"
        "\n"
        "priv struct NestedStruct {\n"
        "    inner: FFIStruct,\n"
        "    count: usize\n"
        "}\n";
    
    Lexer *lexer = lexer_create(test_code, strlen(test_code), "test.asthra");
    assert(lexer != NULL);
    
    Parser *parser = parser_create(lexer);
    assert(parser != NULL);
    
    ASTNode *program = parser_parse_program(parser);
    if (!program) {
        fprintf(stderr, "FAILED: Could not parse struct fields with FFI annotation conflicts\n");
        if (parser_had_error(parser)) {
            const ParseError *errors = parser_get_errors(parser);
            while (errors) {
                fprintf(stderr, "Error: %s\n", errors->message);
                errors = errors->next;
            }
        }
        parser_destroy(parser);
        lexer_destroy(lexer);
        assert(false);
    }
    
    assert(program->type == AST_PROGRAM);
    
    // Verify we have all declarations
    ASTNodeList *declarations = program->data.program.declarations;
    assert(declarations != NULL);
    assert(ast_node_list_size(declarations) == 3);
    
    // Check first struct (FFIStruct)
    ASTNode *struct1 = ast_node_list_get(declarations, 0);
    assert(struct1->type == AST_STRUCT_DECL);
    assert(strcmp(struct1->data.struct_decl.name, "FFIStruct") == 0);
    
    // Check extern declaration
    ASTNode *extern_decl = ast_node_list_get(declarations, 1);
    assert(extern_decl->type == AST_EXTERN_DECL);
    assert(strcmp(extern_decl->data.extern_decl.name, "process_struct") == 0);
    
    // Check second struct (NestedStruct)
    ASTNode *struct2 = ast_node_list_get(declarations, 2);
    assert(struct2->type == AST_STRUCT_DECL);
    assert(strcmp(struct2->data.struct_decl.name, "NestedStruct") == 0);
    
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    printf("✓ Struct fields with FFI annotation conflicts parsed successfully\n");
} 
