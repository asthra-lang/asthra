/**
 * Unit tests for LLVM pattern matching (match statements) generation
 * Tests basic pattern matching functionality in match statements
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

// Helper function to create a match statement AST node
static ASTNode* create_match_stmt(ASTNode *expression, ASTNodeList *arms) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_MATCH_STMT;
    node->data.match_stmt.expression = expression;
    node->data.match_stmt.arms = arms;
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Helper function to create a match arm AST node
static ASTNode* create_match_arm(ASTNode *pattern, ASTNode *body, ASTNode *guard) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_MATCH_ARM;
    node->data.match_arm.pattern = pattern;
    node->data.match_arm.body = body;
    node->data.match_arm.guard = guard;
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
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

// Helper function to create a return statement
static ASTNode* create_return_stmt(ASTNode *expression) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_RETURN_STMT;
    node->data.return_stmt.expression = expression;
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return node;
}

// Helper function to create an ASTNodeList
static ASTNodeList* create_node_list(ASTNode **nodes, size_t count) {
    ASTNodeList *list = malloc(sizeof(ASTNodeList) + count * sizeof(ASTNode*));
    list->count = count;
    list->capacity = count;
    for (size_t i = 0; i < count; i++) {
        list->nodes[i] = nodes[i];
    }
    return list;
}

// Test 1: Match statement with no expression should report error
static bool test_match_no_expression() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create match arms (empty list)
    ASTNode **arms = malloc(sizeof(ASTNode*) * 1);
    arms[0] = create_match_arm(create_integer_literal(42), create_return_stmt(create_integer_literal(1)), NULL);
    ASTNodeList *arm_list = create_node_list(arms, 1);
    
    // Create match statement with NULL expression
    ASTNode *match_stmt = create_match_stmt(NULL, arm_list);
    
    // Generate match statement (should report error)
    generate_statement(data, match_stmt);
    
    // Check that an error was reported
    bool has_error = llvm_backend_has_errors(data);
    
    // Cleanup
    free(arms);
    free(arm_list);
    free(match_stmt);
    cleanup_test_backend(data);
    
    return has_error;
}

// Test 2: Match statement with no arms should report error
static bool test_match_no_arms() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create match statement with NULL arms
    ASTNode *match_stmt = create_match_stmt(create_integer_literal(42), NULL);
    
    // Generate match statement (should report error)
    generate_statement(data, match_stmt);
    
    // Check that an error was reported
    bool has_error = llvm_backend_has_errors(data);
    
    // Cleanup
    free(match_stmt);
    cleanup_test_backend(data);
    
    return has_error;
}

// Test 3: Simple match statement with integer literal patterns
static bool test_simple_match_statement() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create match arms
    ASTNode **arms = malloc(sizeof(ASTNode*) * 2);
    arms[0] = create_match_arm(create_integer_literal(1), create_return_stmt(create_integer_literal(10)), NULL);
    arms[1] = create_match_arm(create_integer_literal(2), create_return_stmt(create_integer_literal(20)), NULL);
    ASTNodeList *arm_list = create_node_list(arms, 2);
    
    // Create match statement
    ASTNode *match_stmt = create_match_stmt(create_integer_literal(1), arm_list);
    
    // Generate match statement
    generate_statement(data, match_stmt);
    
    // Check that no error was reported
    bool has_error = llvm_backend_has_errors(data);
    
    // Check that the module can be verified
    char *error = NULL;
    bool is_valid = !LLVMVerifyModule(data->module, LLVMReturnStatusAction, &error);
    if (error) LLVMDisposeMessage(error);
    
    // Cleanup
    free(arms);
    free(arm_list);
    free(match_stmt);
    cleanup_test_backend(data);
    
    return !has_error && is_valid;
}

// Test 4: Match statement with identifier pattern (binding)
static bool test_identifier_pattern() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create match arm with identifier pattern
    ASTNode **arms = malloc(sizeof(ASTNode*) * 1);
    arms[0] = create_match_arm(create_identifier("x"), create_return_stmt(create_identifier("x")), NULL);
    ASTNodeList *arm_list = create_node_list(arms, 1);
    
    // Create match statement
    ASTNode *match_stmt = create_match_stmt(create_integer_literal(42), arm_list);
    
    // Generate match statement
    generate_statement(data, match_stmt);
    
    // Check that no error was reported
    bool has_error = llvm_backend_has_errors(data);
    
    // Check that the module can be verified
    char *error = NULL;
    bool is_valid = !LLVMVerifyModule(data->module, LLVMReturnStatusAction, &error);
    if (error) LLVMDisposeMessage(error);
    
    // Cleanup
    free(arms);
    free(arm_list);
    free(match_stmt);
    cleanup_test_backend(data);
    
    return !has_error && is_valid;
}

// Test 5: Match statement with guard condition
static bool test_match_with_guard() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create match arm with guard
    ASTNode *guard = create_integer_literal(1);  // Simple guard (always true)
    ASTNode **arms = malloc(sizeof(ASTNode*) * 1);
    arms[0] = create_match_arm(create_identifier("x"), create_return_stmt(create_identifier("x")), guard);
    ASTNodeList *arm_list = create_node_list(arms, 1);
    
    // Create match statement
    ASTNode *match_stmt = create_match_stmt(create_integer_literal(42), arm_list);
    
    // Generate match statement
    generate_statement(data, match_stmt);
    
    // Check that no error was reported
    bool has_error = llvm_backend_has_errors(data);
    
    // Check that the module can be verified
    char *error = NULL;
    bool is_valid = !LLVMVerifyModule(data->module, LLVMReturnStatusAction, &error);
    if (error) LLVMDisposeMessage(error);
    
    // Cleanup
    free(arms);
    free(arm_list);
    free(match_stmt);
    cleanup_test_backend(data);
    
    return !has_error && is_valid;
}

// Test 6: Multiple match arms
static bool test_multiple_match_arms() {
    LLVMBackendData *data = create_test_backend();
    if (!data) return false;
    
    // Create multiple match arms
    ASTNode **arms = malloc(sizeof(ASTNode*) * 3);
    arms[0] = create_match_arm(create_integer_literal(1), create_return_stmt(create_integer_literal(10)), NULL);
    arms[1] = create_match_arm(create_integer_literal(2), create_return_stmt(create_integer_literal(20)), NULL);
    arms[2] = create_match_arm(create_identifier("_"), create_return_stmt(create_integer_literal(0)), NULL);
    ASTNodeList *arm_list = create_node_list(arms, 3);
    
    // Create match statement
    ASTNode *match_stmt = create_match_stmt(create_integer_literal(1), arm_list);
    
    // Generate match statement
    generate_statement(data, match_stmt);
    
    // Check that no error was reported
    bool has_error = llvm_backend_has_errors(data);
    
    // Check that the module can be verified
    char *error = NULL;
    bool is_valid = !LLVMVerifyModule(data->module, LLVMReturnStatusAction, &error);
    if (error) LLVMDisposeMessage(error);
    
    // Cleanup
    free(arms);
    free(arm_list);
    free(match_stmt);
    cleanup_test_backend(data);
    
    return !has_error && is_valid;
}

int main() {
    printf("Running LLVM Pattern Matching (Match Statements) Tests...\n\n");
    
    TEST_CASE(test_match_no_expression);
    TEST_CASE(test_match_no_arms);
    TEST_CASE(test_simple_match_statement);
    TEST_CASE(test_identifier_pattern);
    TEST_CASE(test_match_with_guard);
    TEST_CASE(test_multiple_match_arms);
    
    printf("\nTest Results: %d/%d tests passed\n", tests_passed, test_count);
    
    return (tests_passed == test_count) ? 0 : 1;
}