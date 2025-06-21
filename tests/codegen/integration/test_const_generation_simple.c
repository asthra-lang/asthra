/**
 * Asthra Programming Language Compiler
 * Simple Const Generation Test
 * 
 * Tests the basic functionality of const declaration code generation
 * without complex semantic analysis dependencies
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Include the const generation headers
#include "ffi_assembly_const.h"
#include "global_symbols.h"
#include "ast.h"

// Mock FFI generator for testing
typedef struct {
    int dummy;
} MockFFIGenerator;

// Mock emit_comment function for testing
static void emit_comment(void* generator, const char* comment) {
    printf("COMMENT: %s\n", comment);
}

// Test basic integer constant generation
void test_integer_const_generation(void) {
    printf("Testing integer constant generation...\n");
    
    char* result = ffi_generate_integer_const(42);
    assert(result != NULL);
    assert(strcmp(result, "42") == 0);
    free(result);
    
    result = ffi_generate_integer_const(-123);
    assert(result != NULL);
    assert(strcmp(result, "-123") == 0);
    free(result);
    
    printf("✓ Integer constant generation tests passed\n");
}

// Test basic float constant generation
void test_float_const_generation(void) {
    printf("Testing float constant generation...\n");
    
    char* result = ffi_generate_float_const(3.14159);
    assert(result != NULL);
    printf("Generated float: '%s'\n", result);  // Debug output
    assert(strstr(result, "3.14") != NULL);  // More lenient check
    free(result);
    
    result = ffi_generate_float_const(-2.5);
    assert(result != NULL);
    printf("Generated float: '%s'\n", result);  // Debug output
    assert(strstr(result, "-2.5") != NULL);
    free(result);
    
    printf("✓ Float constant generation tests passed\n");
}

// Test basic string constant generation
void test_string_const_generation(void) {
    printf("Testing string constant generation...\n");
    
    char* result = ffi_generate_string_const("hello");
    assert(result != NULL);
    assert(strcmp(result, "\"hello\"") == 0);
    free(result);
    
    result = ffi_generate_string_const("hello \"world\"");
    assert(result != NULL);
    assert(strstr(result, "\\\"") != NULL);  // Should contain escaped quotes
    free(result);
    
    printf("✓ String constant generation tests passed\n");
}

// Test boolean constant generation
void test_boolean_const_generation(void) {
    printf("Testing boolean constant generation...\n");
    
    char* result = ffi_generate_boolean_const(true);
    assert(result != NULL);
    assert(strcmp(result, "true") == 0);
    free(result);
    
    result = ffi_generate_boolean_const(false);
    assert(result != NULL);
    assert(strcmp(result, "false") == 0);
    free(result);
    
    printf("✓ Boolean constant generation tests passed\n");
}

// Test C type mapping
void test_c_type_mapping(void) {
    printf("Testing C type mapping...\n");
    
    // Create mock AST node for testing
    ASTNode type_node;
    type_node.type = AST_BASE_TYPE;
    type_node.data.base_type.name = "i32";
    
    char* result = ffi_get_c_type_from_ast(&type_node);
    assert(result != NULL);
    assert(strcmp(result, "int32_t") == 0);
    free(result);
    
    type_node.data.base_type.name = "string";
    result = ffi_get_c_type_from_ast(&type_node);
    assert(result != NULL);
    assert(strcmp(result, "const char*") == 0);
    free(result);
    
    printf("✓ C type mapping tests passed\n");
}

// Test global symbol table basic operations
void test_global_symbol_table(void) {
    printf("Testing global symbol table...\n");
    
    // Clean up any previous global state
    global_symbols_cleanup();
    
    MockFFIGenerator mock_gen;
    
    // Add a symbol using the global function
    bool success = ffi_add_global_const_symbol((FFIAssemblyGenerator*)&mock_gen, "TEST_CONST", "int32_t", true);
    assert(success);
    
    // Test that symbol exists
    bool exists = ffi_const_symbol_exists((FFIAssemblyGenerator*)&mock_gen, "TEST_CONST");
    assert(exists);
    
    bool is_public = ffi_const_symbol_is_public((FFIAssemblyGenerator*)&mock_gen, "TEST_CONST");
    assert(is_public);
    
    // Add a private symbol
    success = ffi_add_global_const_symbol((FFIAssemblyGenerator*)&mock_gen, "PRIVATE_CONST", "float", false);
    assert(success);
    
    exists = ffi_const_symbol_exists((FFIAssemblyGenerator*)&mock_gen, "PRIVATE_CONST");
    assert(exists);
    
    is_public = ffi_const_symbol_is_public((FFIAssemblyGenerator*)&mock_gen, "PRIVATE_CONST");
    assert(!is_public);  // Should be private
    
    global_symbols_cleanup();
    
    printf("✓ Global symbol table tests passed\n");
}

// Test binary operator mapping
void test_binary_operator_mapping(void) {
    printf("Testing binary operator mapping...\n");
    
    const char* op = ffi_get_c_binary_operator(BINOP_ADD);
    assert(op != NULL);
    assert(strcmp(op, "+") == 0);
    
    op = ffi_get_c_binary_operator(BINOP_MUL);
    assert(op != NULL);
    assert(strcmp(op, "*") == 0);
    
    op = ffi_get_c_binary_operator(BINOP_EQ);
    assert(op != NULL);
    assert(strcmp(op, "==") == 0);
    
    printf("✓ Binary operator mapping tests passed\n");
}

// Test unary operator mapping
void test_unary_operator_mapping(void) {
    printf("Testing unary operator mapping...\n");
    
    const char* op = ffi_get_c_unary_operator(UNOP_MINUS);
    assert(op != NULL);
    assert(strcmp(op, "-") == 0);
    
    op = ffi_get_c_unary_operator(UNOP_NOT);
    assert(op != NULL);
    assert(strcmp(op, "!") == 0);
    
    printf("✓ Unary operator mapping tests passed\n");
}

int main(void) {
    printf("=== Const Generation Simple Tests ===\n\n");
    
    test_integer_const_generation();
    test_float_const_generation();
    test_string_const_generation();
    test_boolean_const_generation();
    test_c_type_mapping();
    test_global_symbol_table();
    test_binary_operator_mapping();
    test_unary_operator_mapping();
    
    printf("\n=== All tests passed! ===\n");
    return 0;
} 
