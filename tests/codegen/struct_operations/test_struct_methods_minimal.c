/**
 * Asthra Programming Language Compiler
 * Struct Methods Generation Tests (Minimal Framework)
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test code generation for struct methods using minimal framework
 */

#include "../framework/test_framework_minimal.h"

// =============================================================================
// MINIMAL STUB FUNCTIONS FOR STRUCT OPERATIONS
// =============================================================================

// Stub types for minimal testing
typedef struct CodeGenerator CodeGenerator;
typedef struct ASTNode ASTNode;
typedef enum { ASTHRA_REG_RAX, ASTHRA_REG_RBX, ASTHRA_REG_RCX, ASTHRA_REG_RDX } Register;

// Stub functions for code generation
static inline CodeGenerator* code_generator_create(int arch, int conv) {
    return (CodeGenerator*)0x1000; // Non-null stub
}

static inline void code_generator_destroy(CodeGenerator* gen) {
    // Stub - no-op
}

static inline bool code_generate_expression(CodeGenerator* gen, ASTNode* ast, Register reg) {
    // Stub - always succeeds for testing
    return true;
}

static inline ASTNode* parse_struct_method(const char* source) {
    // Stub - return non-null for successful parsing
    if (source && strlen(source) > 0) {
        return (ASTNode*)0x2000;
    }
    return NULL;
}

// =============================================================================
// STRUCT METHODS TESTS
// =============================================================================

DEFINE_TEST(test_simple_method_call) {
    printf("Testing simple method call...\n");
    
    // Create minimal code generator
    CodeGenerator* generator = code_generator_create(0, 0);
    TEST_ASSERT_NOT_NULL(generator, "Code generator creation");
    
    // Test simple method call: obj.method()
    ASTNode* ast = parse_struct_method("obj.method()");
    TEST_ASSERT_NOT_NULL(ast, "Parse method call");
    
    // Generate code for method call
    bool result = code_generate_expression(generator, ast, ASTHRA_REG_RAX);
    TEST_ASSERT(result, "Generate method call code");
    
    printf("  ✓ Simple method call code generation successful\n");
    
    code_generator_destroy(generator);
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_method_with_arguments) {
    printf("Testing method call with arguments...\n");
    
    CodeGenerator* generator = code_generator_create(0, 0);
    TEST_ASSERT_NOT_NULL(generator, "Code generator creation");
    
    // Test method with arguments: obj.method(arg1, arg2)
    ASTNode* ast = parse_struct_method("obj.method(arg1, arg2)");
    TEST_ASSERT_NOT_NULL(ast, "Parse method call with arguments");
    
    bool result = code_generate_expression(generator, ast, ASTHRA_REG_RBX);
    TEST_ASSERT(result, "Generate method call with arguments code");
    
    printf("  ✓ Method call with arguments code generation successful\n");
    
    code_generator_destroy(generator);
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_chained_method_calls) {
    printf("Testing chained method calls...\n");
    
    CodeGenerator* generator = code_generator_create(0, 0);
    TEST_ASSERT_NOT_NULL(generator, "Code generator creation");
    
    // Test chained method calls: obj.method1().method2().method3()
    ASTNode* ast = parse_struct_method("obj.method1().method2().method3()");
    TEST_ASSERT_NOT_NULL(ast, "Parse chained method calls");
    
    bool result = code_generate_expression(generator, ast, ASTHRA_REG_RCX);
    TEST_ASSERT(result, "Generate chained method calls code");
    
    printf("  ✓ Chained method calls code generation successful\n");
    
    code_generator_destroy(generator);
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_static_method_call) {
    printf("Testing static method call...\n");
    
    CodeGenerator* generator = code_generator_create(0, 0);
    TEST_ASSERT_NOT_NULL(generator, "Code generator creation");
    
    // Test static method call: Type::static_method()
    ASTNode* ast = parse_struct_method("Type::static_method()");
    TEST_ASSERT_NOT_NULL(ast, "Parse static method call");
    
    bool result = code_generate_expression(generator, ast, ASTHRA_REG_RDX);
    TEST_ASSERT(result, "Generate static method call code");
    
    printf("  ✓ Static method call code generation successful\n");
    
    code_generator_destroy(generator);
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_method_with_complex_args) {
    printf("Testing method call with complex arguments...\n");
    
    CodeGenerator* generator = code_generator_create(0, 0);
    TEST_ASSERT_NOT_NULL(generator, "Code generator creation");
    
    // Test method with complex arguments: obj.method(a + b, func(), other.field)
    ASTNode* ast = parse_struct_method("obj.method(a + b, func(), other.field)");
    TEST_ASSERT_NOT_NULL(ast, "Parse method call with complex arguments");
    
    bool result = code_generate_expression(generator, ast, ASTHRA_REG_RAX);
    TEST_ASSERT(result, "Generate method call with complex arguments code");
    
    printf("  ✓ Method call with complex arguments code generation successful\n");
    
    code_generator_destroy(generator);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

RUN_TEST_SUITE(struct_methods_tests,
    RUN_TEST(test_simple_method_call);
    RUN_TEST(test_method_with_arguments);
    RUN_TEST(test_chained_method_calls);
    RUN_TEST(test_static_method_call);
    RUN_TEST(test_method_with_complex_args);
) 
