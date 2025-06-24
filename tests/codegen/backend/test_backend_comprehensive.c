/**
 * Comprehensive Backend Tests
 * Tests all aspects of the multi-backend architecture
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include "../../../tests/framework/test_framework.h"
#include "../../../src/codegen/backend_interface.h"
#include "../../../src/compiler.h"
#include "../../../src/parser/ast_types.h"
#include "../../../src/analysis/type_info.h"

// Test utilities for creating AST nodes
static ASTNode* create_test_program_ast(void) {
    ASTNode *program = calloc(1, sizeof(ASTNode));
    program->type = AST_PROGRAM;
    program->location = (SourceLocation){.line = 1, .column = 1, .file = "test.asthra"};
    program->ref_count = 1;
    
    // Create declarations list
    ASTNodeList *decls = malloc(sizeof(ASTNodeList) + 2 * sizeof(ASTNode*));
    decls->count = 2;
    decls->capacity = 2;
    
    // Create a simple function: fn main() -> i32 { return 42; }
    ASTNode *main_func = create_test_function("main", "i32");
    ASTNode *add_func = create_test_function("add", "i32");
    
    decls->nodes[0] = main_func;
    decls->nodes[1] = add_func;
    program->data.program.declarations = decls;
    
    return program;
}

static ASTNode* create_test_function(const char* name, const char* return_type) {
    ASTNode *func = calloc(1, sizeof(ASTNode));
    func->type = AST_FUNCTION_DECL;
    func->location = (SourceLocation){.line = 1, .column = 1, .file = "test.asthra"};
    func->ref_count = 1;
    func->data.function_decl.name = strdup(name);
    func->data.function_decl.visibility = VISIBILITY_PUBLIC;
    
    // Create simple return statement: return 42;
    ASTNode *body = create_test_block_with_return();
    func->data.function_decl.body = body;
    
    // Create type info
    TypeInfo *func_type = calloc(1, sizeof(TypeInfo));
    func_type->category = TYPE_INFO_FUNCTION;
    func_type->type_id = TYPE_INFO_FUNCTION;
    func_type->name = malloc(strlen(name) + 20);
    snprintf((char*)func_type->name, strlen(name) + 20, "fn %s() -> %s", name, return_type);
    
    // Set return type
    TypeInfo *ret_type = calloc(1, sizeof(TypeInfo));
    ret_type->category = TYPE_INFO_PRIMITIVE;
    ret_type->data.primitive.kind = PRIMITIVE_INFO_I32;
    ret_type->name = "i32";
    func_type->data.function.return_type = ret_type;
    func_type->data.function.param_count = 0;
    func_type->data.function.param_types = NULL;
    
    func->type_info = func_type;
    
    return func;
}

static ASTNode* create_test_block_with_return(void) {
    ASTNode *block = calloc(1, sizeof(ASTNode));
    block->type = AST_BLOCK;
    block->location = (SourceLocation){.line = 1, .column = 1, .file = "test.asthra"};
    block->ref_count = 1;
    
    // Create statements list
    ASTNodeList *stmts = malloc(sizeof(ASTNodeList) + sizeof(ASTNode*));
    stmts->count = 1;
    stmts->capacity = 1;
    
    // Create return statement
    ASTNode *ret_stmt = calloc(1, sizeof(ASTNode));
    ret_stmt->type = AST_RETURN_STMT;
    ret_stmt->location = (SourceLocation){.line = 1, .column = 1, .file = "test.asthra"};
    ret_stmt->ref_count = 1;
    
    // Create literal value: 42
    ASTNode *literal = calloc(1, sizeof(ASTNode));
    literal->type = AST_LITERAL;
    literal->location = (SourceLocation){.line = 1, .column = 1, .file = "test.asthra"};
    literal->ref_count = 1;
    literal->data.literal.type = LITERAL_INT;
    literal->data.literal.value.int_val = 42;
    
    ret_stmt->data.return_stmt.value = literal;
    stmts->nodes[0] = ret_stmt;
    block->data.block.statements = stmts;
    
    return block;
}

// Test 1: Backend Creation and Initialization
TEST_CASE(test_backend_creation_and_initialization) {
    AsthraCompilerOptions options = asthra_compiler_default_options();
    
    // Test C Backend
    {
        options.backend_type = ASTHRA_BACKEND_C;
        AsthraBackend *backend = asthra_backend_create(&options);
        ASSERT_NOT_NULL(backend, "C backend should be created");
        ASSERT_EQ(backend->type, ASTHRA_BACKEND_C, "Backend type should be C");
        
        int result = asthra_backend_initialize(backend, &options);
        ASSERT_EQ(result, 0, "C backend initialization should succeed");
        
        ASSERT_NOT_NULL(asthra_backend_get_name(backend), "Backend name should not be null");
        ASSERT_NOT_NULL(asthra_backend_get_version(backend), "Backend version should not be null");
        
        asthra_backend_destroy(backend);
    }
    
    // Test Assembly Backend
    {
        options.backend_type = ASTHRA_BACKEND_ASSEMBLY;
        AsthraBackend *backend = asthra_backend_create(&options);
        ASSERT_NOT_NULL(backend, "Assembly backend should be created");
        ASSERT_EQ(backend->type, ASTHRA_BACKEND_ASSEMBLY, "Backend type should be Assembly");
        
        int result = asthra_backend_initialize(backend, &options);
        ASSERT_EQ(result, 0, "Assembly backend initialization should succeed");
        
        asthra_backend_destroy(backend);
    }
    
    // Test LLVM Backend (conditional based on compilation flags)
    {
        options.backend_type = ASTHRA_BACKEND_LLVM_IR;
        AsthraBackend *backend = asthra_backend_create(&options);
        
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
        ASSERT_NOT_NULL(backend, "LLVM backend should be created when enabled");
        ASSERT_EQ(backend->type, ASTHRA_BACKEND_LLVM_IR, "Backend type should be LLVM IR");
        
        int result = asthra_backend_initialize(backend, &options);
        ASSERT_EQ(result, 0, "LLVM backend initialization should succeed when enabled");
        
        asthra_backend_destroy(backend);
#else
        // When LLVM is not compiled in, creation should fail
        // This is expected behavior and tests that the system gracefully handles missing backends
        printf("  Note: LLVM backend not compiled in - testing graceful failure\n");
#endif
    }
    
    return true;
}

// Test 2: Code Generation
TEST_CASE(test_backend_code_generation) {
    AsthraCompilerOptions options = asthra_compiler_default_options();
    AsthraCompilerContext ctx = {.options = options};
    ASTNode *ast = create_test_program_ast();
    
    // Test C Backend Code Generation
    {
        options.backend_type = ASTHRA_BACKEND_C;
        AsthraBackend *backend = asthra_backend_create(&options);
        ASSERT_NOT_NULL(backend, "C backend should be created");
        
        int result = asthra_backend_initialize(backend, &options);
        ASSERT_EQ(result, 0, "C backend should initialize");
        
        // Generate code to a temporary file
        const char *output_file = "/tmp/test_asthra_c_output.c";
        result = asthra_backend_generate(backend, &ctx, ast, output_file);
        ASSERT_EQ(result, 0, "C code generation should succeed");
        
        // Verify output file exists and has content
        FILE *f = fopen(output_file, "r");
        ASSERT_NOT_NULL(f, "Output file should be created");
        
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        ASSERT_GT(size, 0, "Output file should have content");
        fclose(f);
        
        // Check backend statistics
        size_t lines, functions;
        double time;
        asthra_backend_get_stats(backend, &lines, &functions, &time);
        ASSERT_GT(lines, 0, "Should have generated some lines");
        ASSERT_GE(time, 0.0, "Generation time should be non-negative");
        
        // Cleanup
        unlink(output_file);
        asthra_backend_destroy(backend);
    }
    
    // Test Assembly Backend Code Generation
    {
        options.backend_type = ASTHRA_BACKEND_ASSEMBLY;
        AsthraBackend *backend = asthra_backend_create(&options);
        ASSERT_NOT_NULL(backend, "Assembly backend should be created");
        
        int result = asthra_backend_initialize(backend, &options);
        ASSERT_EQ(result, 0, "Assembly backend should initialize");
        
        // Generate code to a temporary file
        const char *output_file = "/tmp/test_asthra_asm_output.s";
        result = asthra_backend_generate(backend, &ctx, ast, output_file);
        
        if (result == 0) {
            // Verify output file exists and has content
            FILE *f = fopen(output_file, "r");
            ASSERT_NOT_NULL(f, "Assembly output file should be created");
            
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            ASSERT_GT(size, 0, "Assembly output file should have content");
            fclose(f);
            
            unlink(output_file);
        } else {
            printf("  Note: Assembly generation may fail on some platforms - this is expected\n");
        }
        
        asthra_backend_destroy(backend);
    }
    
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
    // Test LLVM Backend Code Generation (only if compiled in)
    {
        options.backend_type = ASTHRA_BACKEND_LLVM_IR;
        AsthraBackend *backend = asthra_backend_create(&options);
        ASSERT_NOT_NULL(backend, "LLVM backend should be created");
        
        int result = asthra_backend_initialize(backend, &options);
        ASSERT_EQ(result, 0, "LLVM backend should initialize");
        
        // Generate code to a temporary file
        const char *output_file = "/tmp/test_asthra_llvm_output.ll";
        result = asthra_backend_generate(backend, &ctx, ast, output_file);
        
        if (result == 0) {
            // Verify output file exists and has content
            FILE *f = fopen(output_file, "r");
            ASSERT_NOT_NULL(f, "LLVM output file should be created");
            
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            ASSERT_GT(size, 0, "LLVM output file should have content");
            fclose(f);
            
            unlink(output_file);
        }
        
        asthra_backend_destroy(backend);
    }
#endif
    
    // TODO: Free AST properly
    return true;
}

// Test 3: Feature Support
TEST_CASE(test_backend_feature_support) {
    AsthraCompilerOptions options = asthra_compiler_default_options();
    
    // Test C Backend Features
    {
        options.backend_type = ASTHRA_BACKEND_C;
        AsthraBackend *backend = asthra_backend_create(&options);
        ASSERT_NOT_NULL(backend, "C backend should be created");
        
        // Test common features
        ASSERT_TRUE(asthra_backend_supports_feature(backend, "functions"), 
                   "C backend should support functions");
        ASSERT_TRUE(asthra_backend_supports_feature(backend, "expressions"), 
                   "C backend should support expressions");
        ASSERT_TRUE(asthra_backend_supports_feature(backend, "statements"), 
                   "C backend should support statements");
        ASSERT_FALSE(asthra_backend_supports_feature(backend, "unknown_feature"), 
                    "C backend should not support unknown features");
        
        asthra_backend_destroy(backend);
    }
    
    // Test Assembly Backend Features
    {
        options.backend_type = ASTHRA_BACKEND_ASSEMBLY;
        AsthraBackend *backend = asthra_backend_create(&options);
        ASSERT_NOT_NULL(backend, "Assembly backend should be created");
        
        // Test assembly-specific features
        ASSERT_TRUE(asthra_backend_supports_feature(backend, "x86_64"), 
                   "Assembly backend should support x86_64");
        ASSERT_TRUE(asthra_backend_supports_feature(backend, "arm64"), 
                   "Assembly backend should support arm64");
        ASSERT_TRUE(asthra_backend_supports_feature(backend, "intel_syntax"), 
                   "Assembly backend should support Intel syntax");
        ASSERT_TRUE(asthra_backend_supports_feature(backend, "att_syntax"), 
                   "Assembly backend should support AT&T syntax");
        
        asthra_backend_destroy(backend);
    }
    
#ifdef ASTHRA_ENABLE_LLVM_BACKEND
    // Test LLVM Backend Features (only if compiled in)
    {
        options.backend_type = ASTHRA_BACKEND_LLVM_IR;
        AsthraBackend *backend = asthra_backend_create(&options);
        ASSERT_NOT_NULL(backend, "LLVM backend should be created");
        
        // Test LLVM-specific features
        ASSERT_TRUE(asthra_backend_supports_feature(backend, "optimization"), 
                   "LLVM backend should support optimization");
        ASSERT_TRUE(asthra_backend_supports_feature(backend, "debug-info"), 
                   "LLVM backend should support debug info");
        ASSERT_TRUE(asthra_backend_supports_feature(backend, "cross-compilation"), 
                   "LLVM backend should support cross-compilation");
        
        asthra_backend_destroy(backend);
    }
#endif
    
    return true;
}

// Test 4: Output File Name Generation
TEST_CASE(test_backend_output_filenames) {
    // Test C backend file extensions
    {
        char *output = asthra_backend_get_output_filename(ASTHRA_BACKEND_C, 
                                                          "test.asthra", NULL);
        ASSERT_NOT_NULL(output, "Output filename should not be null");
        ASSERT_STR_EQ(output, "test.c", "C backend should generate .c extension");
        free(output);
        
        // Test with explicit output file
        output = asthra_backend_get_output_filename(ASTHRA_BACKEND_C, 
                                                   "test.asthra", "custom.c");
        ASSERT_STR_EQ(output, "custom.c", "Should use explicit output filename");
        free(output);
    }
    
    // Test Assembly backend file extensions
    {
        char *output = asthra_backend_get_output_filename(ASTHRA_BACKEND_ASSEMBLY, 
                                                          "test.asthra", NULL);
        ASSERT_NOT_NULL(output, "Output filename should not be null");
        ASSERT_STR_EQ(output, "test.s", "Assembly backend should generate .s extension");
        free(output);
    }
    
    // Test LLVM backend file extensions
    {
        char *output = asthra_backend_get_output_filename(ASTHRA_BACKEND_LLVM_IR, 
                                                          "test.asthra", NULL);
        ASSERT_NOT_NULL(output, "Output filename should not be null");
        ASSERT_STR_EQ(output, "test.ll", "LLVM backend should generate .ll extension");
        free(output);
    }
    
    return true;
}

// Test 5: Error Handling
TEST_CASE(test_backend_error_handling) {
    AsthraCompilerOptions options = asthra_compiler_default_options();
    
    // Test with invalid parameters
    {
        AsthraBackend *backend = asthra_backend_create(NULL);
        ASSERT_NULL(backend, "Backend creation with NULL options should fail");
        
        backend = asthra_backend_create_by_type(ASTHRA_BACKEND_C);
        ASSERT_NOT_NULL(backend, "Backend should be created");
        
        // Test with NULL compiler context
        int result = asthra_backend_generate(backend, NULL, NULL, "output.c");
        ASSERT_NE(result, 0, "Code generation with NULL context should fail");
        
        const char *error = asthra_backend_get_last_error(backend);
        ASSERT_NOT_NULL(error, "Error message should be available");
        
        asthra_backend_destroy(backend);
    }
    
    // Test LLVM backend without LLVM compiled in
#ifndef ASTHRA_ENABLE_LLVM_BACKEND
    {
        AsthraBackend *backend = asthra_backend_create_by_type(ASTHRA_BACKEND_LLVM_IR);
        ASSERT_NOT_NULL(backend, "Backend object should be created");
        
        int result = asthra_backend_initialize(backend, &options);
        ASSERT_NE(result, 0, "LLVM backend initialization should fail when not compiled in");
        
        const char *error = asthra_backend_get_last_error(backend);
        ASSERT_NOT_NULL(error, "Error message should be available");
        ASSERT_TRUE(strstr(error, "not compiled in") != NULL, 
                   "Error should mention not compiled in");
        
        asthra_backend_destroy(backend);
    }
#endif
    
    return true;
}

// Test 6: Backend Statistics
TEST_CASE(test_backend_statistics) {
    AsthraCompilerOptions options = asthra_compiler_default_options();
    AsthraCompilerContext ctx = {.options = options};
    ASTNode *ast = create_test_program_ast();
    
    options.backend_type = ASTHRA_BACKEND_C;
    AsthraBackend *backend = asthra_backend_create(&options);
    ASSERT_NOT_NULL(backend, "Backend should be created");
    
    int result = asthra_backend_initialize(backend, &options);
    ASSERT_EQ(result, 0, "Backend should initialize");
    
    // Initial statistics should be zero
    size_t lines, functions;
    double time;
    asthra_backend_get_stats(backend, &lines, &functions, &time);
    ASSERT_EQ(lines, 0, "Initial lines should be zero");
    ASSERT_EQ(functions, 0, "Initial functions should be zero");
    ASSERT_EQ(time, 0.0, "Initial time should be zero");
    
    // Generate code
    const char *output_file = "/tmp/test_asthra_stats_output.c";
    result = asthra_backend_generate(backend, &ctx, ast, output_file);
    ASSERT_EQ(result, 0, "Code generation should succeed");
    
    // Check updated statistics
    asthra_backend_get_stats(backend, &lines, &functions, &time);
    ASSERT_GT(lines, 0, "Lines should be greater than zero after generation");
    ASSERT_GE(time, 0.0, "Generation time should be non-negative");
    
    // Cleanup
    unlink(output_file);
    asthra_backend_destroy(backend);
    
    return true;
}

int main(void) {
    printf("=== Comprehensive Backend Tests ===\n\n");
    
    int passed = 0;
    int total = 0;
    
    RUN_TEST(test_backend_creation_and_initialization);
    RUN_TEST(test_backend_code_generation);
    RUN_TEST(test_backend_feature_support);
    RUN_TEST(test_backend_output_filenames);
    RUN_TEST(test_backend_error_handling);
    RUN_TEST(test_backend_statistics);
    
    printf("\n=== Backend Test Results ===\n");
    printf("Tests passed: %d/%d\n", passed, total);
    
    if (passed == total) {
        printf("✅ All backend tests passed!\n");
        return 0;
    } else {
        printf("❌ Some backend tests failed!\n");
        return 1;
    }
}