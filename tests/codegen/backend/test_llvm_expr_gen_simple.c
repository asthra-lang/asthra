/**
 * LLVM Expression Generation Module Tests (Simplified Debug Version)
 * Minimal tests to identify segfault issues
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <llvm-c/Core.h>
#include "../../../src/codegen/llvm_backend_internal.h"
#include "../../../src/codegen/llvm_expr_gen.h"
#include "../../../src/codegen/llvm_types.h"
#include "../../../src/parser/ast_types.h"

// Simple test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAILED: %s\n", message); \
            return 1; \
        } \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr, message) \
    do { \
        if ((ptr) == NULL) { \
            printf("FAILED: %s (pointer is NULL)\n", message); \
            return 1; \
        } \
    } while (0)

// Global test fixture
static LLVMBackendData *g_backend_data = NULL;

// Setup function
static int setup_test_environment(void) {
    printf("Setting up test environment...\n");
    
    g_backend_data = calloc(1, sizeof(LLVMBackendData));
    TEST_ASSERT_NOT_NULL(g_backend_data, "Failed to allocate backend data");
    
    g_backend_data->context = LLVMContextCreate();
    TEST_ASSERT_NOT_NULL(g_backend_data->context, "Failed to create LLVM context");
    
    g_backend_data->module = LLVMModuleCreateWithNameInContext("test_module", g_backend_data->context);
    TEST_ASSERT_NOT_NULL(g_backend_data->module, "Failed to create LLVM module");
    
    g_backend_data->builder = LLVMCreateBuilderInContext(g_backend_data->context);
    TEST_ASSERT_NOT_NULL(g_backend_data->builder, "Failed to create LLVM builder");
    
    // Initialize type cache
    printf("Initializing type cache...\n");
    llvm_types_cache_init(g_backend_data);
    
    // Create a test function to work within
    printf("Creating test function...\n");
    LLVMTypeRef func_type = LLVMFunctionType(g_backend_data->i32_type, NULL, 0, false);
    g_backend_data->current_function = LLVMAddFunction(g_backend_data->module, "test_function", func_type);
    
    // Create entry block and position builder
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(g_backend_data->context, g_backend_data->current_function, "entry");
    LLVMPositionBuilderAtEnd(g_backend_data->builder, entry);
    
    printf("Setup complete.\n");
    return 0;
}

// Teardown function
static void teardown_test_environment(void) {
    printf("Cleaning up test environment...\n");
    if (g_backend_data) {
        if (g_backend_data->builder) {
            LLVMDisposeBuilder(g_backend_data->builder);
        }
        if (g_backend_data->module) {
            LLVMDisposeModule(g_backend_data->module);
        }
        if (g_backend_data->context) {
            LLVMContextDispose(g_backend_data->context);
        }
        free(g_backend_data);
        g_backend_data = NULL;
    }
    printf("Cleanup complete.\n");
}

// Test basic setup
static int test_basic_setup(void) {
    printf("Testing basic setup...\n");
    TEST_ASSERT_NOT_NULL(g_backend_data, "Backend data should be set up");
    TEST_ASSERT_NOT_NULL(g_backend_data->context, "LLVM context should be set up");
    TEST_ASSERT_NOT_NULL(g_backend_data->module, "LLVM module should be set up");
    TEST_ASSERT_NOT_NULL(g_backend_data->builder, "LLVM builder should be set up");
    TEST_ASSERT_NOT_NULL(g_backend_data->current_function, "Test function should be set up");
    printf("Basic setup test passed.\n");
    return 0;
}

// Test NULL input handling
static int test_null_input(void) {
    printf("Testing NULL input handling...\n");
    LLVMValueRef value = generate_expression(g_backend_data, NULL);
    TEST_ASSERT(value == NULL, "NULL node should return NULL");
    printf("NULL input test passed.\n");
    return 0;
}

// Test simple integer literal (minimal)
static int test_simple_integer_literal(void) {
    printf("Testing simple integer literal...\n");
    
    // Create a simple integer literal node
    ASTNode *node = calloc(1, sizeof(ASTNode));
    TEST_ASSERT_NOT_NULL(node, "Failed to allocate AST node");
    
    node->type = AST_INTEGER_LITERAL;
    node->data.integer_literal.value = 42;
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    node->ref_count = 1;
    node->type_info = NULL; // Use default type
    
    printf("About to call generate_expression...\n");
    LLVMValueRef value = generate_expression(g_backend_data, node);
    printf("generate_expression returned.\n");
    
    TEST_ASSERT_NOT_NULL(value, "Integer literal should generate LLVM value");
    
    printf("Checking if value is constant...\n");
    TEST_ASSERT(LLVMIsConstant(value), "Integer literal should be constant");
    
    printf("Cleanup AST node...\n");
    free(node);
    
    printf("Simple integer literal test passed.\n");
    return 0;
}

// Main test runner
int main(int argc, char **argv) {
    printf("Running LLVM Expression Generation Module Tests (Debug)...\n\n");
    
    // Setup test environment
    if (setup_test_environment() != 0) {
        printf("FAILED: Could not setup test environment\n");
        return 1;
    }
    
    // Run basic tests
    if (test_basic_setup() != 0) {
        printf("Basic setup test failed\n");
        teardown_test_environment();
        return 1;
    }
    
    if (test_null_input() != 0) {
        printf("NULL input test failed\n");
        teardown_test_environment();
        return 1;
    }
    
    if (test_simple_integer_literal() != 0) {
        printf("Simple integer literal test failed\n");
        teardown_test_environment();
        return 1;
    }
    
    // Cleanup
    teardown_test_environment();
    
    printf("\nAll tests passed!\n");
    return 0;
}