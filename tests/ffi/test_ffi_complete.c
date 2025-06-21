/**
 * Comprehensive FFI Implementation Test
 * Tests the complete FFI functionality including:
 * - Extern declaration parsing and semantic analysis
 * - FFI marshaling and code generation
 * - Runtime integration
 * - Variadic function support
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parser.h"
#include "semantic_analyzer.h"
#include "ffi_assembly_generator.h"
#include "../runtime/asthra_runtime.h"

// Test helper functions
static void test_extern_declaration_parsing(void);
static void test_ffi_semantic_analysis(void);
static void test_ffi_marshaling(void);
static void test_variadic_function_support(void);
static void test_runtime_integration(void);

int main(void) {
    printf("Running comprehensive FFI implementation tests...\n");
    
    // Initialize runtime for testing
    if (asthra_runtime_init(NULL) != 0) {
        fprintf(stderr, "Failed to initialize Asthra runtime\n");
        return 1;
    }
    
    test_extern_declaration_parsing();
    test_ffi_semantic_analysis();
    test_ffi_marshaling();
    test_variadic_function_support();
    test_runtime_integration();
    
    // Cleanup runtime
    asthra_runtime_cleanup();
    
    printf("All FFI tests passed!\n");
    return 0;
}

static void test_extern_declaration_parsing(void) {
    printf("Testing extern declaration parsing...\n");
    
    const char *test_code = 
        "extern \"libc\" fn malloc(size: usize) -> #[transfer_full] *mut void;\n"
        "extern \"libc\" fn printf(#[borrowed] format: *const u8, ...) -> i32;\n"
        "extern \"my_lib\" fn process_data(\n"
        "    #[transfer_full] data: *mut u8,\n"
        "    #[transfer_none] config: *const u8\n"
        ") -> #[transfer_full] *mut u8;\n";
    
    Parser *parser = parser_create();
    assert(parser != NULL);
    
    if (!parser_parse_string(parser, test_code)) {
        fprintf(stderr, "Failed to parse extern declarations\n");
        parser_destroy(parser);
        assert(false);
    }
    
    ASTNode *program = parser_get_ast(parser);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);
    
    // Verify we have extern declarations
    ASTNodeList *declarations = program->data.program.declarations;
    assert(declarations != NULL);
    assert(ast_node_list_size(declarations) == 3);
    
    // Check first extern declaration (malloc)
    ASTNode *malloc_decl = ast_node_list_get(declarations, 0);
    assert(malloc_decl->type == AST_EXTERN_DECL);
    assert(strcmp(malloc_decl->data.extern_decl.name, "malloc") == 0);
    assert(strcmp(malloc_decl->data.extern_decl.extern_name, "libc") == 0);
    
    // Check second extern declaration (printf - variadic)
    ASTNode *printf_decl = ast_node_list_get(declarations, 1);
    assert(printf_decl->type == AST_EXTERN_DECL);
    assert(strcmp(printf_decl->data.extern_decl.name, "printf") == 0);
    
    // Check third extern declaration (process_data - with annotations)
    ASTNode *process_decl = ast_node_list_get(declarations, 2);
    assert(process_decl->type == AST_EXTERN_DECL);
    assert(strcmp(process_decl->data.extern_decl.name, "process_data") == 0);
    assert(strcmp(process_decl->data.extern_decl.extern_name, "my_lib") == 0);
    
    parser_destroy(parser);
    printf("✓ Extern declaration parsing tests passed\n");
}

static void test_ffi_semantic_analysis(void) {
    printf("Testing FFI semantic analysis...\n");
    
    const char *test_code = 
        "extern \"libc\" fn strlen(s: *const u8) -> usize;\n"
        "extern \"libc\" fn memcpy(dest: *mut void, src: *const void, n: usize) -> *mut void;\n";
    
    Parser *parser = parser_create();
    assert(parser != NULL);
    
    if (!parser_parse_string(parser, test_code)) {
        fprintf(stderr, "Failed to parse FFI test code\n");
        parser_destroy(parser);
        assert(false);
    }
    
    ASTNode *program = parser_get_ast(parser);
    assert(program != NULL);
    
    // Create semantic analyzer
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);
    
    // Analyze the program
    bool analysis_result = semantic_analyze_program(analyzer, program);
    if (!analysis_result) {
        fprintf(stderr, "Semantic analysis failed\n");
        const SemanticError *errors = semantic_get_errors(analyzer);
        while (errors) {
            fprintf(stderr, "Error: %s\n", errors->message);
            errors = errors->next;
        }
        assert(false);
    }
    
    // Verify symbols were registered
    SymbolEntry *strlen_symbol = semantic_resolve_identifier(analyzer, "strlen");
    assert(strlen_symbol != NULL);
    assert(strlen_symbol->kind == SYMBOL_FUNCTION);
    
    SymbolEntry *memcpy_symbol = semantic_resolve_identifier(analyzer, "memcpy");
    assert(memcpy_symbol != NULL);
    assert(memcpy_symbol->kind == SYMBOL_FUNCTION);
    
    semantic_analyzer_destroy(analyzer);
    parser_destroy(parser);
    printf("✓ FFI semantic analysis tests passed\n");
}

static void test_ffi_marshaling(void) {
    printf("Testing FFI marshaling...\n");
    
    // Create FFI assembly generator
    FFIAssemblyGenerator *generator = ffi_assembly_generator_create(ARCH_X86_64, CONV_SYSV_AMD64);
    assert(generator != NULL);
    
    // Test marshaling type determination
    // This would require creating mock AST nodes with type information
    // For now, we'll test the basic functionality
    
    // Create a simple call expression for testing
    ASTNode *call_expr = ast_create_node(AST_CALL_EXPR, (SourceLocation){0});
    assert(call_expr != NULL);
    
    // Create function identifier
    ASTNode *function = ast_create_node(AST_IDENTIFIER, (SourceLocation){0});
    function->data.identifier.name = strdup("test_function");
    call_expr->data.call_expr.function = function;
    
    // Create argument list
    ASTNodeList *args = ast_node_list_create(2);
    
    // Add string argument
    ASTNode *string_arg = ast_create_node(AST_STRING_LITERAL, (SourceLocation){0});
    string_arg->data.string_literal.value = strdup("test string");
    ast_node_list_add(&args, string_arg);
    
    // Add integer argument
    ASTNode *int_arg = ast_create_node(AST_INTEGER_LITERAL, (SourceLocation){0});
    int_arg->data.integer_literal.value = 42;
    ast_node_list_add(&args, int_arg);
    
    call_expr->data.call_expr.args = args;
    
    // Test FFI call generation
    bool result = ffi_generate_extern_call(generator, call_expr);
    assert(result == true);
    
    // Cleanup
    ast_free_node(call_expr);
    ffi_assembly_generator_destroy(generator);
    
    printf("✓ FFI marshaling tests passed\n");
}

static void test_variadic_function_support(void) {
    printf("Testing variadic function support...\n");
    
    // Create FFI assembly generator
    FFIAssemblyGenerator *generator = ffi_assembly_generator_create(ARCH_X86_64, CONV_SYSV_AMD64);
    assert(generator != NULL);
    
    // Create a variadic call expression (printf-like)
    ASTNode *call_expr = ast_create_node(AST_CALL_EXPR, (SourceLocation){0});
    assert(call_expr != NULL);
    
    // Create function identifier
    ASTNode *function = ast_create_node(AST_IDENTIFIER, (SourceLocation){0});
    function->data.identifier.name = strdup("printf");
    call_expr->data.call_expr.function = function;
    
    // Create argument list with format string + variadic args
    ASTNodeList *args = ast_node_list_create(3);
    
    // Format string (fixed argument)
    ASTNode *format_arg = ast_create_node(AST_STRING_LITERAL, (SourceLocation){0});
    format_arg->data.string_literal.value = strdup("Hello %s, number: %d");
    ast_node_list_add(&args, format_arg);
    
    // Variadic argument 1 (string)
    ASTNode *string_arg = ast_create_node(AST_STRING_LITERAL, (SourceLocation){0});
    string_arg->data.string_literal.value = strdup("World");
    ast_node_list_add(&args, string_arg);
    
    // Variadic argument 2 (integer)
    ASTNode *int_arg = ast_create_node(AST_INTEGER_LITERAL, (SourceLocation){0});
    int_arg->data.integer_literal.value = 42;
    ast_node_list_add(&args, int_arg);
    
    call_expr->data.call_expr.args = args;
    
    // Test variadic call generation (1 fixed argument, 2 variadic)
    bool result = ffi_generate_variadic_call(generator, call_expr, 1);
    assert(result == true);
    
    // Cleanup
    ast_free_node(call_expr);
    ffi_assembly_generator_destroy(generator);
    
    printf("✓ Variadic function support tests passed\n");
}

static void test_runtime_integration(void) {
    printf("Testing runtime integration...\n");
    
    // Test string conversion functions
    const char *test_cstr = "Hello, FFI World!";
    AsthraString asthra_str = asthra_string_from_cstr(test_cstr);
    
    assert(asthra_str.data != NULL);
    assert(asthra_str.len == strlen(test_cstr));
    assert(strcmp(asthra_str.data, test_cstr) == 0);
    
    // Test conversion back to C string
    char *converted_cstr = asthra_string_to_cstr(asthra_str);
    assert(converted_cstr != NULL);
    assert(strcmp(converted_cstr, test_cstr) == 0);
    
    // Test string utility functions
    size_t len = asthra_string_len(asthra_str);
    assert(len == strlen(test_cstr));
    
    // Test substring
    AsthraString substr = asthra_string_substring(asthra_str, 7, 10);
    assert(substr.data != NULL);
    assert(substr.len == 3);
    assert(strncmp(substr.data, "FFI", 3) == 0);
    
    // Test character finding
    int64_t pos = asthra_string_find_char(asthra_str, 'F');
    assert(pos == 7); // First 'F' in "FFI"
    
    // Test case conversion
    AsthraString lower_str = asthra_string_to_lowercase(asthra_str);
    assert(lower_str.data != NULL);
    assert(lower_str.data[0] == 'h'); // 'H' -> 'h'
    
    // Test number to string conversions
    AsthraString int_str = asthra_int_to_string(42);
    assert(int_str.data != NULL);
    assert(strcmp(int_str.data, "42") == 0);
    
    AsthraString uint_str = asthra_uint_to_string(123456);
    assert(uint_str.data != NULL);
    assert(strcmp(uint_str.data, "123456") == 0);
    
    AsthraString float_str = asthra_float_to_string(3.14159);
    assert(float_str.data != NULL);
    assert(strstr(float_str.data, "3.14") != NULL);
    
    // Cleanup
    free(converted_cstr);
    asthra_string_free(asthra_str);
    asthra_string_free(substr);
    asthra_string_free(lower_str);
    asthra_string_free(int_str);
    asthra_string_free(uint_str);
    asthra_string_free(float_str);
    
    printf("✓ Runtime integration tests passed\n");
} 
