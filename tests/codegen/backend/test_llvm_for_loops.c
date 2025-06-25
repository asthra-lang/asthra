/**
 * Unit tests for LLVM for loop iteration with various iterable types
 * Tests range functions, arrays, slices, and other iterable types
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Include LLVM headers
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>

// Include project headers
#include "../../../src/codegen/llvm_backend_internal.h"
#include "../../../src/codegen/llvm_stmt_gen.h"
#include "../../../src/codegen/llvm_types.h"
#include "../../../src/parser/ast_node.h"
#include "../../../src/parser/ast_types.h"
#include "../../../src/analysis/type_info.h"

// Simple test framework
static int test_count = 0;
static int tests_passed = 0;

#define TEST_CASE(name) \
    do { \
        test_count++; \
        printf("Running test: %s\n", #name); \
        if (name()) { \
            tests_passed++; \
            printf("  PASS: %s\n", #name); \
        } else { \
            printf("  FAIL: %s\n", #name); \
        } \
    } while(0)

// Helper function to create a minimal LLVM backend data structure
static LLVMBackendData* create_test_backend() {
    LLVMBackendData *data = calloc(1, sizeof(LLVMBackendData));
    if (!data) return NULL;
    
    // Initialize LLVM
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    
    data->context = LLVMContextCreate();
    data->module = LLVMModuleCreateWithNameInContext("test_module", data->context);
    data->builder = LLVMCreateBuilderInContext(data->context);
    
    // Initialize common types
    data->i32_type = LLVMInt32TypeInContext(data->context);
    data->i64_type = LLVMInt64TypeInContext(data->context);
    data->f32_type = LLVMFloatTypeInContext(data->context);
    data->f64_type = LLVMDoubleTypeInContext(data->context);
    data->bool_type = LLVMInt1TypeInContext(data->context);
    data->void_type = LLVMVoidTypeInContext(data->context);
    data->ptr_type = LLVMPointerTypeInContext(data->context, 0);
    
    // Create a test function
    LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, NULL, 0, false);
    data->current_function = LLVMAddFunction(data->module, "test_function", fn_type);
    LLVMBasicBlockRef entry_bb = LLVMAppendBasicBlockInContext(data->context, data->current_function, "entry");
    LLVMPositionBuilderAtEnd(data->builder, entry_bb);
    
    return data;
}

// Helper function to cleanup test backend
static void cleanup_test_backend(LLVMBackendData *data) {
    if (!data) return;
    
    if (data->builder) LLVMDisposeBuilder(data->builder);
    if (data->module) LLVMDisposeModule(data->module);
    if (data->context) LLVMContextDispose(data->context);
    free(data);
}

// Helper function to create an integer literal AST node
static ASTNode* create_integer_literal(int64_t value) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_INTEGER_LITERAL;
    node->data.integer_literal.value = value;
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Helper function to create an identifier AST node
static ASTNode* create_identifier(const char *name) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_IDENTIFIER;
    node->data.identifier.name = strdup(name);
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Helper function to create a function call AST node
static ASTNode* create_function_call(const char *func_name, ASTNode **args, size_t arg_count) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_CALL_EXPR;
    node->data.call_expr.function = create_identifier(func_name);
    
    if (arg_count > 0) {
        ASTNodeList *arg_list = malloc(sizeof(ASTNodeList) + arg_count * sizeof(ASTNode*));
        arg_list->count = arg_count;
        arg_list->capacity = arg_count;
        for (size_t i = 0; i < arg_count; i++) {
            arg_list->nodes[i] = args[i];
        }
        node->data.call_expr.args = arg_list;
    } else {
        node->data.call_expr.args = NULL;
    }
    
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Helper function to create a for statement AST node
static ASTNode* create_for_stmt(const char *variable, ASTNode *iterable, ASTNode *body) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_FOR_STMT;
    node->data.for_stmt.variable = strdup(variable);
    node->data.for_stmt.iterable = iterable;
    node->data.for_stmt.body = body;
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Helper function to create a block statement
static ASTNode* create_block_stmt(ASTNode **statements, size_t count) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_BLOCK;
    
    if (count > 0) {
        ASTNodeList *stmt_list = malloc(sizeof(ASTNodeList) + count * sizeof(ASTNode*));
        stmt_list->count = count;
        stmt_list->capacity = count;
        for (size_t i = 0; i < count; i++) {
            stmt_list->nodes[i] = statements[i];
        }
        node->data.block.statements = stmt_list;
    } else {
        node->data.block.statements = NULL;
    }
    
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Helper function to create a unit literal AST node
static ASTNode* create_unit_literal() {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_UNIT_LITERAL;
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Helper function to create a return statement
static ASTNode* create_return_stmt(ASTNode *expression) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_RETURN_STMT;
    node->data.return_stmt.expression = expression;
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Test 1: For loop with missing iterable should report error
static bool test_for_loop_missing_iterable() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create for statement with NULL iterable
    ASTNode *body = create_block_stmt(NULL, 0);
    ASTNode *for_stmt = create_for_stmt("i", NULL, body);
    
    // Generate for statement (should report error)
    generate_statement(data, for_stmt);
    
    // Check that an error was reported
    bool has_error = llvm_backend_has_errors(data);
    
    // Cleanup
    free(body);
    free(for_stmt);
    cleanup_test_backend(data);
    
    return has_error;
}

// Test 2: For loop with missing variable should report error
static bool test_for_loop_missing_variable() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create for statement with NULL variable
    ASTNode *iterable = create_integer_literal(10);
    ASTNode *body = create_block_stmt(NULL, 0);
    ASTNode *for_stmt = calloc(1, sizeof(ASTNode));
    for_stmt->type = AST_FOR_STMT;
    for_stmt->data.for_stmt.variable = NULL;  // Missing variable
    for_stmt->data.for_stmt.iterable = iterable;
    for_stmt->data.for_stmt.body = body;
    
    // Generate for statement (should report error)
    generate_statement(data, for_stmt);
    
    // Check that an error was reported
    bool has_error = llvm_backend_has_errors(data);
    
    // Cleanup
    free(iterable);
    free(body);
    free(for_stmt);
    cleanup_test_backend(data);
    
    return has_error;
}

// Test 3: Simple range(10) loop
static bool test_simple_range_loop() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create range(10) call
    ASTNode *args[1] = { create_integer_literal(10) };
    ASTNode *range_call = create_function_call("range", args, 1);
    
    // Create simple body with return statement
    ASTNode *body_stmts[1] = { create_return_stmt(create_unit_literal()) };
    ASTNode *body = create_block_stmt(body_stmts, 1);
    
    // Create for statement
    ASTNode *for_stmt = create_for_stmt("i", range_call, body);
    
    // Generate for statement
    generate_statement(data, for_stmt);
    
    // Add a return statement to complete the function
    // (since this is the last statement in a void function)
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
        LLVMBuildRetVoid(data->builder);
    }
    
    // Check that no error was reported
    bool has_error = llvm_backend_has_errors(data);
    if (has_error) {
        printf("    Backend errors detected:\n");
        llvm_backend_print_errors(data);
    }
    
    // Check that the module can be verified
    char *error = NULL;
    bool is_valid = !LLVMVerifyModule(data->module, LLVMReturnStatusAction, &error);
    if (!is_valid) {
        printf("    Module verification failed: %s\n", error ? error : "Unknown error");
    }
    if (error) LLVMDisposeMessage(error);
    
    // Print LLVM module for debugging if test fails
    if (has_error || !is_valid) {
        char *module_str = LLVMPrintModuleToString(data->module);
        printf("    Generated LLVM IR:\n%s\n", module_str);
        LLVMDisposeMessage(module_str);
    }
    
    // Cleanup
    free(args[0]);
    free(range_call);
    free(body_stmts[0]);
    free(body);
    free(for_stmt);
    cleanup_test_backend(data);
    
    return !has_error && is_valid;
}

// Test 4: Range with start and end: range(5, 15)
static bool test_range_with_bounds() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create range(5, 15) call
    ASTNode *args[2] = { create_integer_literal(5), create_integer_literal(15) };
    ASTNode *range_call = create_function_call("range", args, 2);
    
    // Create simple body
    ASTNode *body = create_block_stmt(NULL, 0);
    
    // Create for statement
    ASTNode *for_stmt = create_for_stmt("j", range_call, body);
    
    // Generate for statement
    generate_statement(data, for_stmt);
    
    // Add a return statement to complete the function
    // (since this is the last statement in a void function)
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
        LLVMBuildRetVoid(data->builder);
    }
    
    // Check that no error was reported
    bool has_error = llvm_backend_has_errors(data);
    if (has_error) {
        printf("    Backend errors detected:\n");
        llvm_backend_print_errors(data);
    }
    
    // Check that the module can be verified
    char *error = NULL;
    bool is_valid = !LLVMVerifyModule(data->module, LLVMReturnStatusAction, &error);
    if (!is_valid) {
        printf("    Module verification failed: %s\n", error ? error : "Unknown error");
    }
    if (error) LLVMDisposeMessage(error);
    
    // Print LLVM module for debugging if test fails
    if (has_error || !is_valid) {
        char *module_str = LLVMPrintModuleToString(data->module);
        printf("    Generated LLVM IR:\n%s\n", module_str);
        LLVMDisposeMessage(module_str);
    }
    
    // Cleanup
    free(args[0]);
    free(args[1]);
    free(range_call);
    free(body);
    free(for_stmt);
    cleanup_test_backend(data);
    
    return !has_error && is_valid;
}

// Test 5: Array iteration (simulated)
static bool test_array_iteration() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create an array literal as iterable (simplified)
    ASTNode *iterable = create_identifier("test_array");
    
    // Create simple body
    ASTNode *body = create_block_stmt(NULL, 0);
    
    // Create for statement
    ASTNode *for_stmt = create_for_stmt("elem", iterable, body);
    
    // Generate for statement
    generate_statement(data, for_stmt);
    
    // We expect this to fail gracefully since we don't have a real array
    bool has_error = llvm_backend_has_errors(data);
    
    // Cleanup
    free(iterable);
    free(body);
    free(for_stmt);
    cleanup_test_backend(data);
    
    // This should produce an error since we can't generate the array expression
    return has_error;
}

// Test 6: Invalid range call (wrong number of arguments)
static bool test_invalid_range_call() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create range() call with 3 arguments (invalid)
    ASTNode *args[3] = { 
        create_integer_literal(1), 
        create_integer_literal(10), 
        create_integer_literal(2) 
    };
    ASTNode *range_call = create_function_call("range", args, 3);
    
    // Create simple body
    ASTNode *body = create_block_stmt(NULL, 0);
    
    // Create for statement
    ASTNode *for_stmt = create_for_stmt("i", range_call, body);
    
    // Generate for statement
    generate_statement(data, for_stmt);
    
    // Check that an error was reported
    bool has_error = llvm_backend_has_errors(data);
    
    // Cleanup
    free(args[0]);
    free(args[1]);
    free(args[2]);
    free(range_call);
    free(body);
    free(for_stmt);
    cleanup_test_backend(data);
    
    return has_error;
}

// Test 7: Non-range function call
static bool test_non_range_function() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create some_function() call (not range)
    ASTNode *some_call = create_function_call("some_function", NULL, 0);
    
    // Create simple body
    ASTNode *body = create_block_stmt(NULL, 0);
    
    // Create for statement
    ASTNode *for_stmt = create_for_stmt("x", some_call, body);
    
    // Generate for statement
    generate_statement(data, for_stmt);
    
    // Should try to handle as non-range iterable and likely fail
    bool has_error = llvm_backend_has_errors(data);
    
    // Cleanup
    free(some_call);
    free(body);
    free(for_stmt);
    cleanup_test_backend(data);
    
    // This should produce an error since we can't generate the function expression properly
    return has_error;
}

int main() {
    printf("Running LLVM For Loop Iteration Tests...\n\n");
    
    TEST_CASE(test_for_loop_missing_iterable);
    TEST_CASE(test_for_loop_missing_variable);
    TEST_CASE(test_simple_range_loop);
    TEST_CASE(test_range_with_bounds);
    TEST_CASE(test_array_iteration);
    TEST_CASE(test_invalid_range_call);
    TEST_CASE(test_non_range_function);
    
    printf("\nTest Results: %d/%d tests passed\n", tests_passed, test_count);
    
    return (tests_passed == test_count) ? 0 : 1;
}