/**
 * LLVM Cast Expression Tests
 * Tests for the Asthra LLVM cast expression code generation functions
 */

#include "../../../src/analysis/type_info.h"
#include "../../../src/codegen/llvm_backend_internal.h"
#include "../../../src/codegen/llvm_expr_gen.h"
#include "../../../src/codegen/llvm_types.h"
#include "../../../src/parser/ast_types.h"
#include <llvm-c/Core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple test framework macros
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            printf("FAILED: %s\n", message);                                                       \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_NOT_NULL(ptr, message)                                                         \
    do {                                                                                           \
        if ((ptr) == NULL) {                                                                       \
            printf("FAILED: %s (pointer is NULL)\n", message);                                     \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

#define RUN_TEST(test_name)                                                                        \
    do {                                                                                           \
        printf("Running %s... ", #test_name);                                                      \
        fflush(stdout);                                                                            \
        if (test_name() == 0) {                                                                    \
            printf("PASSED\n");                                                                    \
            passed++;                                                                              \
        } else {                                                                                   \
            printf("FAILED\n");                                                                    \
            failed++;                                                                              \
        }                                                                                          \
        total++;                                                                                   \
    } while (0)

// Global test fixture
static LLVMBackendData *g_backend_data = NULL;

// Setup function
static int setup_test_environment(void) {
    g_backend_data = calloc(1, sizeof(LLVMBackendData));
    TEST_ASSERT_NOT_NULL(g_backend_data, "Failed to allocate backend data");

    g_backend_data->context = LLVMContextCreate();
    TEST_ASSERT_NOT_NULL(g_backend_data->context, "Failed to create LLVM context");

    g_backend_data->module =
        LLVMModuleCreateWithNameInContext("test_module", g_backend_data->context);
    TEST_ASSERT_NOT_NULL(g_backend_data->module, "Failed to create LLVM module");

    g_backend_data->builder = LLVMCreateBuilderInContext(g_backend_data->context);
    TEST_ASSERT_NOT_NULL(g_backend_data->builder, "Failed to create LLVM builder");

    // Initialize type cache
    llvm_types_cache_init(g_backend_data);

    // Create a test function to work within
    LLVMTypeRef func_type = LLVMFunctionType(g_backend_data->i32_type, NULL, 0, false);
    g_backend_data->current_function =
        LLVMAddFunction(g_backend_data->module, "test_function", func_type);

    // Create entry block and position builder
    LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(
        g_backend_data->context, g_backend_data->current_function, "entry");
    LLVMPositionBuilderAtEnd(g_backend_data->builder, entry);

    return 0;
}

// Teardown function
static void teardown_test_environment(void) {
    if (g_backend_data) {
        llvm_backend_clear_errors(g_backend_data);
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
}

// Helper function to create integer literal AST node
static ASTNode *create_integer_literal(int64_t value) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_INTEGER_LITERAL;
    node->data.integer_literal.value = value;
    node->location =
        (SourceLocation){.filename = "test.asthra", .line = 1, .column = 1, .offset = 0};
    node->ref_count = 1;
    return node;
}

// Helper function to create cast expression
static ASTNode *create_cast_expr(ASTNode *expression, TypeInfo *target_type_info) {
    ASTNode *cast_node = calloc(1, sizeof(ASTNode));
    cast_node->type = AST_CAST_EXPR;
    cast_node->data.cast_expr.expression = expression;
    cast_node->data.cast_expr.target_type = NULL; // We'll use type_info instead
    cast_node->type_info = target_type_info;
    cast_node->location =
        (SourceLocation){.filename = "test.asthra", .line = 1, .column = 1, .offset = 0};
    cast_node->ref_count = 1;
    return cast_node;
}

// Helper function to create TypeInfo for primitive types
static TypeInfo *create_primitive_type_info(enum PrimitiveInfoKind kind) {
    TypeInfo *type_info = calloc(1, sizeof(TypeInfo));
    type_info->category = TYPE_INFO_PRIMITIVE;
    type_info->data.primitive.kind = kind;
    type_info->ref_count = 1;
    return type_info;
}

// Helper function to free AST node
static void free_ast_node(ASTNode *node) {
    if (!node)
        return;

    if (node->type == AST_CAST_EXPR) {
        if (node->data.cast_expr.expression) {
            free_ast_node(node->data.cast_expr.expression);
        }
        if (node->data.cast_expr.target_type) {
            free_ast_node(node->data.cast_expr.target_type);
        }
    }

    if (node->type_info) {
        free(node->type_info);
    }

    free(node);
}

// Test integer to integer casting (same type)
static int test_integer_same_type_cast(void) {
    // Create i32 literal
    ASTNode *literal = create_integer_literal(42);

    // Create i32 type info
    TypeInfo *i32_type = create_primitive_type_info(PRIMITIVE_INFO_I32);

    // Create cast expression
    ASTNode *cast_expr = create_cast_expr(literal, i32_type);

    // Generate LLVM code
    LLVMValueRef result = generate_expression(g_backend_data, cast_expr);

    if (!result) {
        printf("Cast failed. Errors:\n");
        llvm_backend_print_errors(g_backend_data);
    }

    TEST_ASSERT_NOT_NULL(result, "Cast should generate LLVM value");
    TEST_ASSERT(LLVMIsConstant(result), "Result should be constant");

    // Cleanup
    free_ast_node(cast_expr);

    return 0;
}

// Test integer to integer casting (different sizes)
static int test_integer_size_cast(void) {
    // Create i32 literal
    ASTNode *literal = create_integer_literal(42);

    // Create i64 type info (larger)
    TypeInfo *i64_type = create_primitive_type_info(PRIMITIVE_INFO_I64);

    // Create cast expression
    ASTNode *cast_expr = create_cast_expr(literal, i64_type);

    // Generate LLVM code
    LLVMValueRef result = generate_expression(g_backend_data, cast_expr);

    if (!result) {
        printf("Size cast failed. Errors:\n");
        llvm_backend_print_errors(g_backend_data);
    }

    TEST_ASSERT_NOT_NULL(result, "Cast should generate LLVM value");

    // Check that result has correct type
    LLVMTypeRef result_type = LLVMTypeOf(result);
    TEST_ASSERT(LLVMGetTypeKind(result_type) == LLVMIntegerTypeKind,
                "Result should be integer type");
    TEST_ASSERT(LLVMGetIntTypeWidth(result_type) == 64, "Result should be 64-bit integer");

    // Cleanup
    free_ast_node(cast_expr);

    return 0;
}

// Test integer to float casting
static int test_integer_to_float_cast(void) {
    // Create i32 literal
    ASTNode *literal = create_integer_literal(42);

    // Create f64 type info
    TypeInfo *f64_type = create_primitive_type_info(PRIMITIVE_INFO_F64);

    // Create cast expression
    ASTNode *cast_expr = create_cast_expr(literal, f64_type);

    // Generate LLVM code
    LLVMValueRef result = generate_expression(g_backend_data, cast_expr);

    if (!result) {
        printf("Float cast failed. Errors:\n");
        llvm_backend_print_errors(g_backend_data);
    }

    TEST_ASSERT_NOT_NULL(result, "Cast should generate LLVM value");

    // Check that result has correct type
    LLVMTypeRef result_type = LLVMTypeOf(result);
    TEST_ASSERT(LLVMGetTypeKind(result_type) == LLVMDoubleTypeKind, "Result should be double type");

    // Cleanup
    free_ast_node(cast_expr);

    return 0;
}

// Test cast with missing expression
static int test_cast_missing_expression(void) {
    // Create cast expression with NULL expression
    ASTNode *cast_expr = calloc(1, sizeof(ASTNode));
    cast_expr->type = AST_CAST_EXPR;
    cast_expr->data.cast_expr.expression = NULL; // Missing expression
    cast_expr->data.cast_expr.target_type = NULL;
    cast_expr->location =
        (SourceLocation){.filename = "test.asthra", .line = 1, .column = 1, .offset = 0};
    cast_expr->ref_count = 1;

    // This should report an error
    LLVMValueRef result = generate_expression(g_backend_data, cast_expr);

    TEST_ASSERT(result == NULL, "Cast with missing expression should return NULL");
    TEST_ASSERT(llvm_backend_has_errors(g_backend_data), "Should have reported an error");

    // Clear errors for next test
    llvm_backend_clear_errors(g_backend_data);

    // Cleanup
    free(cast_expr);

    return 0;
}

// Test cast with missing type information
static int test_cast_missing_type_info(void) {
    // Create i32 literal
    ASTNode *literal = create_integer_literal(42);

    // Create cast expression with NULL type info
    ASTNode *cast_expr = calloc(1, sizeof(ASTNode));
    cast_expr->type = AST_CAST_EXPR;
    cast_expr->data.cast_expr.expression = literal;
    cast_expr->data.cast_expr.target_type = NULL;
    cast_expr->type_info = NULL; // Missing type info
    cast_expr->location =
        (SourceLocation){.filename = "test.asthra", .line = 1, .column = 1, .offset = 0};
    cast_expr->ref_count = 1;

    // This should report an error
    LLVMValueRef result = generate_expression(g_backend_data, cast_expr);

    TEST_ASSERT(result == NULL, "Cast with missing type info should return NULL");
    TEST_ASSERT(llvm_backend_has_errors(g_backend_data), "Should have reported an error");

    // Clear errors for next test
    llvm_backend_clear_errors(g_backend_data);

    // Cleanup
    free_ast_node(literal);
    free(cast_expr);

    return 0;
}

// Main test runner
int main(int argc, char **argv) {
    int total = 0, passed = 0, failed = 0;

    printf("Running LLVM Cast Expression Tests...\n\n");

    // Setup test environment
    if (setup_test_environment() != 0) {
        printf("FAILED: Could not setup test environment\n");
        return 1;
    }

    // Run tests
    RUN_TEST(test_integer_same_type_cast);
    RUN_TEST(test_integer_size_cast);
    RUN_TEST(test_integer_to_float_cast);
    RUN_TEST(test_cast_missing_expression);
    RUN_TEST(test_cast_missing_type_info);

    // Cleanup
    teardown_test_environment();

    // Summary
    printf("\n=== Test Summary ===\n");
    printf("Total: %d\n", total);
    printf("Passed: %d\n", passed);
    printf("Failed: %d\n", failed);

    if (failed > 0) {
        printf("Overall result: FAILED\n");
        return 1;
    } else {
        printf("Overall result: PASSED\n");
        return 0;
    }
}