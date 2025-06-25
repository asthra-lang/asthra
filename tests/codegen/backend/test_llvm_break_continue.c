/**
 * Unit tests for LLVM break/continue statement generation
 * Tests break and continue statements in loop contexts
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include LLVM headers
#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>

// Include project headers
#include "../../../src/analysis/type_info.h"
#include "../../../src/codegen/llvm_backend_internal.h"
#include "../../../src/codegen/llvm_stmt_gen.h"
#include "../../../src/codegen/llvm_types.h"
#include "../../../src/parser/ast_node.h"
#include "../../../src/parser/ast_types.h"

// Simple test framework
static int test_count = 0;
static int tests_passed = 0;

#define TEST_CASE(name)                                                                            \
    do {                                                                                           \
        test_count++;                                                                              \
        printf("Running test: %s\n", name);                                                        \
        if (name()) {                                                                              \
            tests_passed++;                                                                        \
            printf("  PASS: %s\n", #name);                                                         \
        } else {                                                                                   \
            printf("  FAIL: %s\n", #name);                                                         \
        }                                                                                          \
    } while (0)

// Helper function to create a minimal LLVM backend data structure
static LLVMBackendData *create_test_backend() {
    LLVMBackendData *data = calloc(1, sizeof(LLVMBackendData));
    if (!data)
        return NULL;

    // Initialize LLVM
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();

    data->context = LLVMContextCreate();
    data->module = LLVMModuleCreateWithNameInContext("test_module", data->context);
    data->builder = LLVMCreateBuilderInContext(data->context);

    // Initialize common types
    data->i32_type = LLVMInt32TypeInContext(data->context);
    data->i64_type = LLVMInt64TypeInContext(data->context);
    data->bool_type = LLVMInt1TypeInContext(data->context);
    data->void_type = LLVMVoidTypeInContext(data->context);
    data->ptr_type = LLVMPointerTypeInContext(data->context, 0);

    // Create a test function
    LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, NULL, 0, false);
    data->current_function = LLVMAddFunction(data->module, "test_function", fn_type);
    LLVMBasicBlockRef entry_bb =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "entry");
    LLVMPositionBuilderAtEnd(data->builder, entry_bb);

    return data;
}

// Helper function to cleanup test backend
static void cleanup_test_backend(LLVMBackendData *data) {
    if (!data)
        return;

    if (data->builder)
        LLVMDisposeBuilder(data->builder);
    if (data->module)
        LLVMDisposeModule(data->module);
    if (data->context)
        LLVMContextDispose(data->context);
    free(data);
}

// Helper function to create a break statement AST node
static ASTNode *create_break_stmt() {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_BREAK_STMT;
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Helper function to create a continue statement AST node
static ASTNode *create_continue_stmt() {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_CONTINUE_STMT;
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Test 1: Break statement outside of loop should report error
static bool test_break_outside_loop() {
    LLVMBackendData *data = create_test_backend();
    if (!data)
        return false;

    ASTNode *break_stmt = create_break_stmt();

    // Generate break statement (should report error)
    generate_statement(data, break_stmt);

    // Check that an error was reported
    bool has_error = llvm_backend_has_errors(data);

    free(break_stmt);
    cleanup_test_backend(data);

    return has_error;
}

// Test 2: Continue statement outside of loop should report error
static bool test_continue_outside_loop() {
    LLVMBackendData *data = create_test_backend();
    if (!data)
        return NULL;

    ASTNode *continue_stmt = create_continue_stmt();

    // Generate continue statement (should report error)
    generate_statement(data, continue_stmt);

    // Check that an error was reported
    bool has_error = llvm_backend_has_errors(data);

    free(continue_stmt);
    cleanup_test_backend(data);

    return has_error;
}

// Test 3: Break statement inside loop context should generate branch
static bool test_break_inside_loop() {
    LLVMBackendData *data = create_test_backend();
    if (!data)
        return false;

    // Create loop blocks
    LLVMBasicBlockRef continue_block =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "continue");
    LLVMBasicBlockRef break_block =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "break");

    // Push loop context
    llvm_backend_push_loop_context(data, continue_block, break_block);

    // Create break statement
    ASTNode *break_stmt = create_break_stmt();

    // Generate break statement
    generate_statement(data, break_stmt);

    // Check that no error was reported
    bool has_error = llvm_backend_has_errors(data);

    // Check that current block has a terminator (branch instruction)
    LLVMValueRef terminator = LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder));
    bool has_terminator = (terminator != NULL);

    // Pop loop context
    llvm_backend_pop_loop_context(data);

    free(break_stmt);
    cleanup_test_backend(data);

    return !has_error && has_terminator;
}

// Test 4: Continue statement inside loop context should generate branch
static bool test_continue_inside_loop() {
    LLVMBackendData *data = create_test_backend();
    if (!data)
        return false;

    // Create loop blocks
    LLVMBasicBlockRef continue_block =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "continue");
    LLVMBasicBlockRef break_block =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "break");

    // Push loop context
    llvm_backend_push_loop_context(data, continue_block, break_block);

    // Create continue statement
    ASTNode *continue_stmt = create_continue_stmt();

    // Generate continue statement
    generate_statement(data, continue_stmt);

    // Check that no error was reported
    bool has_error = llvm_backend_has_errors(data);

    // Check that current block has a terminator (branch instruction)
    LLVMValueRef terminator = LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder));
    bool has_terminator = (terminator != NULL);

    // Pop loop context
    llvm_backend_pop_loop_context(data);

    free(continue_stmt);
    cleanup_test_backend(data);

    return !has_error && has_terminator;
}

// Test 5: Nested loops should use correct loop context
static bool test_nested_loops() {
    LLVMBackendData *data = create_test_backend();
    if (!data)
        return false;

    // Create outer loop blocks
    LLVMBasicBlockRef outer_continue =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "outer_continue");
    LLVMBasicBlockRef outer_break =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "outer_break");

    // Create inner loop blocks
    LLVMBasicBlockRef inner_continue =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "inner_continue");
    LLVMBasicBlockRef inner_break =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "inner_break");

    // Push outer loop context
    llvm_backend_push_loop_context(data, outer_continue, outer_break);

    // Push inner loop context
    llvm_backend_push_loop_context(data, inner_continue, inner_break);

    // Check that current loop is the inner loop
    LoopContext *current_loop = llvm_backend_get_current_loop(data);
    bool correct_inner = (current_loop && current_loop->continue_block == inner_continue &&
                          current_loop->break_block == inner_break);

    // Pop inner loop
    llvm_backend_pop_loop_context(data);

    // Check that current loop is now the outer loop
    current_loop = llvm_backend_get_current_loop(data);
    bool correct_outer = (current_loop && current_loop->continue_block == outer_continue &&
                          current_loop->break_block == outer_break);

    // Pop outer loop
    llvm_backend_pop_loop_context(data);

    // Check that no loop context remains
    current_loop = llvm_backend_get_current_loop(data);
    bool no_context = (current_loop == NULL);

    cleanup_test_backend(data);

    return correct_inner && correct_outer && no_context;
}

// Test 6: Loop context management - push/pop operations
static bool test_loop_context_management() {
    LLVMBackendData *data = create_test_backend();
    if (!data)
        return false;

    // Initially no loop context
    LoopContext *initial = llvm_backend_get_current_loop(data);
    if (initial != NULL) {
        cleanup_test_backend(data);
        return false;
    }

    // Create test blocks
    LLVMBasicBlockRef continue_block =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "continue");
    LLVMBasicBlockRef break_block =
        LLVMAppendBasicBlockInContext(data->context, data->current_function, "break");

    // Push context
    llvm_backend_push_loop_context(data, continue_block, break_block);

    // Check context exists
    LoopContext *current = llvm_backend_get_current_loop(data);
    if (!current || current->continue_block != continue_block ||
        current->break_block != break_block) {
        cleanup_test_backend(data);
        return false;
    }

    // Pop context
    llvm_backend_pop_loop_context(data);

    // Check context is gone
    LoopContext *final = llvm_backend_get_current_loop(data);
    if (final != NULL) {
        cleanup_test_backend(data);
        return false;
    }

    cleanup_test_backend(data);
    return true;
}

int main() {
    printf("Running LLVM Break/Continue Statement Tests...\n\n");

    TEST_CASE(test_break_outside_loop);
    TEST_CASE(test_continue_outside_loop);
    TEST_CASE(test_break_inside_loop);
    TEST_CASE(test_continue_inside_loop);
    TEST_CASE(test_nested_loops);
    TEST_CASE(test_loop_context_management);

    printf("\nTest Results: %d/%d tests passed\n", tests_passed, test_count);

    return (tests_passed == test_count) ? 0 : 1;
}