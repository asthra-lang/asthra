/**
 * LLVM Statement Generation Module Tests
 * Tests for the Asthra LLVM statement code generation functions
 */

#include "../../../src/analysis/type_info.h"
#include "../../../src/codegen/llvm_backend_internal.h"
#include "../../../src/codegen/llvm_expr_gen.h"
#include "../../../src/codegen/llvm_locals.h"
#include "../../../src/codegen/llvm_stmt_gen.h"
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
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    node->ref_count = 1;
    return node;
}

// Helper function to create identifier AST node
static ASTNode *create_identifier(const char *name) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    node->type = AST_IDENTIFIER;
    node->data.identifier.name = strdup(name);
    node->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    node->ref_count = 1;
    return node;
}

// Helper function to free AST node
static void free_ast_node(ASTNode *node) {
    if (!node)
        return;

    switch (node->type) {
    case AST_IDENTIFIER:
        free((char *)node->data.identifier.name);
        break;
    case AST_LET_STMT:
        if (node->data.let_stmt.initializer) {
            free_ast_node(node->data.let_stmt.initializer);
        }
        break;
    case AST_RETURN_STMT:
        if (node->data.return_stmt.expression) {
            free_ast_node(node->data.return_stmt.expression);
        }
        break;
    case AST_EXPR_STMT:
        if (node->data.expr_stmt.expression) {
            free_ast_node(node->data.expr_stmt.expression);
        }
        break;
    default:
        break;
    }
    free(node);
}

// Test NULL input handling
static int test_null_input_handling(void) {
    // Should handle NULL gracefully without crashing
    generate_statement(g_backend_data, NULL);
    return 0;
}

// Test return statement generation
static int test_return_statement_generation(void) {
    // Test return with expression
    ASTNode *return_stmt = calloc(1, sizeof(ASTNode));
    return_stmt->type = AST_RETURN_STMT;
    return_stmt->data.return_stmt.expression = create_integer_literal(42);
    return_stmt->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return_stmt->ref_count = 1;

    generate_statement(g_backend_data, return_stmt);

    // Check that the current block has a terminator (return instruction)
    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(g_backend_data->builder);
    LLVMValueRef terminator = LLVMGetBasicBlockTerminator(current_block);
    TEST_ASSERT_NOT_NULL(terminator, "Return statement should create terminator instruction");
    TEST_ASSERT(LLVMGetInstructionOpcode(terminator) == LLVMRet, "Should be a return instruction");

    free_ast_node(return_stmt);

    // Need to create a new block since we just added a terminator
    LLVMBasicBlockRef new_block = LLVMAppendBasicBlockInContext(
        g_backend_data->context, g_backend_data->current_function, "after_return");
    LLVMPositionBuilderAtEnd(g_backend_data->builder, new_block);

    return 0;
}

// Test void return statement
static int test_void_return_statement(void) {
    // Test return without expression (void return)
    ASTNode *return_stmt = calloc(1, sizeof(ASTNode));
    return_stmt->type = AST_RETURN_STMT;
    return_stmt->data.return_stmt.expression = NULL;
    return_stmt->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    return_stmt->ref_count = 1;

    generate_statement(g_backend_data, return_stmt);

    // Check that the current block has a terminator
    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(g_backend_data->builder);
    LLVMValueRef terminator = LLVMGetBasicBlockTerminator(current_block);
    TEST_ASSERT_NOT_NULL(terminator, "Void return should create terminator instruction");
    TEST_ASSERT(LLVMGetInstructionOpcode(terminator) == LLVMRet, "Should be a return instruction");

    free_ast_node(return_stmt);

    // Create a new block for subsequent tests
    LLVMBasicBlockRef new_block = LLVMAppendBasicBlockInContext(
        g_backend_data->context, g_backend_data->current_function, "after_void_return");
    LLVMPositionBuilderAtEnd(g_backend_data->builder, new_block);

    return 0;
}

// Test variable declaration (let statement)
static int test_variable_declaration(void) {
    // Test let x = 42;
    ASTNode *let_stmt = calloc(1, sizeof(ASTNode));
    let_stmt->type = AST_LET_STMT;
    let_stmt->data.let_stmt.name = "test_var";
    let_stmt->data.let_stmt.type = NULL; // Use default type
    let_stmt->data.let_stmt.initializer = create_integer_literal(42);
    let_stmt->data.let_stmt.is_mutable = false;
    let_stmt->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    let_stmt->ref_count = 1;

    // Set type info for the variable
    TypeInfo i32_type_info = {0};
    i32_type_info.category = TYPE_INFO_PRIMITIVE;
    i32_type_info.data.primitive.kind = PRIMITIVE_INFO_I32;
    let_stmt->type_info = &i32_type_info;

    generate_statement(g_backend_data, let_stmt);

    // Verify that the variable was registered
    LLVMValueRef var_alloca = lookup_local_var(g_backend_data, "test_var");
    TEST_ASSERT_NOT_NULL(var_alloca, "Variable should be registered in local variables");
    TEST_ASSERT(LLVMGetInstructionOpcode(var_alloca) == LLVMAlloca,
                "Should be an alloca instruction");

    free_ast_node(let_stmt);
    return 0;
}

// Test expression statement generation
static int test_expression_statement(void) {
    // Test expression statement (standalone expression)
    ASTNode *expr_stmt = calloc(1, sizeof(ASTNode));
    expr_stmt->type = AST_EXPR_STMT;
    expr_stmt->data.expr_stmt.expression = create_integer_literal(123);
    expr_stmt->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    expr_stmt->ref_count = 1;

    // Should not crash or cause issues
    generate_statement(g_backend_data, expr_stmt);

    free_ast_node(expr_stmt);
    return 0;
}

// Test block statement generation
static int test_block_statement(void) {
    // Create a block with multiple statements
    ASTNode *block = calloc(1, sizeof(ASTNode));
    block->type = AST_BLOCK;
    block->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    block->ref_count = 1;

    // Create statements list
    ASTNodeList *statements = calloc(1, sizeof(ASTNodeList) + 2 * sizeof(ASTNode *));
    statements->count = 2;
    statements->capacity = 2;

    // First statement: let y = 10;
    ASTNode *let_stmt = calloc(1, sizeof(ASTNode));
    let_stmt->type = AST_LET_STMT;
    let_stmt->data.let_stmt.name = "block_var";
    let_stmt->data.let_stmt.type = NULL;
    let_stmt->data.let_stmt.initializer = create_integer_literal(10);
    let_stmt->data.let_stmt.is_mutable = false;
    let_stmt->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    let_stmt->ref_count = 1;

    // Second statement: expression statement
    ASTNode *expr_stmt = calloc(1, sizeof(ASTNode));
    expr_stmt->type = AST_EXPR_STMT;
    expr_stmt->data.expr_stmt.expression = create_integer_literal(456);
    expr_stmt->location = (SourceLocation){.filename = NULL, .line = 2, .column = 1, .offset = 0};
    expr_stmt->ref_count = 1;

    statements->nodes[0] = let_stmt;
    statements->nodes[1] = expr_stmt;
    block->data.block.statements = statements;

    generate_statement(g_backend_data, block);

    // Verify that the variable from the block was registered
    LLVMValueRef block_var = lookup_local_var(g_backend_data, "block_var");
    TEST_ASSERT_NOT_NULL(block_var, "Block variable should be registered");

    // Cleanup
    free_ast_node(let_stmt);
    free_ast_node(expr_stmt);
    free(statements);
    free_ast_node(block);

    return 0;
}

// Test if statement generation (basic structure)
static int test_if_statement_basic(void) {
    // Create a simple if statement: if (true) { return 1; }
    ASTNode *if_stmt = calloc(1, sizeof(ASTNode));
    if_stmt->type = AST_IF_STMT;
    if_stmt->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    if_stmt->ref_count = 1;

    // Condition: true literal
    ASTNode *condition = calloc(1, sizeof(ASTNode));
    condition->type = AST_BOOL_LITERAL;
    condition->data.bool_literal.value = true;
    condition->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    condition->ref_count = 1;

    // Then block: return 1;
    ASTNode *then_block = calloc(1, sizeof(ASTNode));
    then_block->type = AST_RETURN_STMT;
    then_block->data.return_stmt.expression = create_integer_literal(1);
    then_block->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    then_block->ref_count = 1;

    if_stmt->data.if_stmt.condition = condition;
    if_stmt->data.if_stmt.then_block = then_block;
    if_stmt->data.if_stmt.else_block = NULL;

    generate_statement(g_backend_data, if_stmt);

    // Should create multiple basic blocks
    // We can't easily verify the exact structure without more complex testing,
    // but at least verify it doesn't crash

    free_ast_node(condition);
    free_ast_node(then_block);
    free_ast_node(if_stmt);

    return 0;
}

// Test unimplemented statements (should not crash)
static int test_unimplemented_statements(void) {
    // Test break statement (not yet implemented)
    ASTNode *break_stmt = calloc(1, sizeof(ASTNode));
    break_stmt->type = AST_BREAK_STMT;
    break_stmt->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    break_stmt->ref_count = 1;

    generate_statement(g_backend_data, break_stmt); // Should not crash

    free_ast_node(break_stmt);

    // Test continue statement (not yet implemented)
    ASTNode *continue_stmt = calloc(1, sizeof(ASTNode));
    continue_stmt->type = AST_CONTINUE_STMT;
    continue_stmt->location =
        (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    continue_stmt->ref_count = 1;

    generate_statement(g_backend_data, continue_stmt); // Should not crash

    free_ast_node(continue_stmt);

    // Test match statement (not yet implemented)
    ASTNode *match_stmt = calloc(1, sizeof(ASTNode));
    match_stmt->type = AST_MATCH_STMT;
    match_stmt->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    match_stmt->ref_count = 1;

    generate_statement(g_backend_data, match_stmt); // Should not crash

    free_ast_node(match_stmt);

    return 0;
}

// Test unknown statement type handling
static int test_unknown_statement_type(void) {
    ASTNode *unknown_stmt = calloc(1, sizeof(ASTNode));
    unknown_stmt->type = 999; // Unknown/invalid type
    unknown_stmt->location =
        (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};
    unknown_stmt->ref_count = 1;

    generate_statement(g_backend_data, unknown_stmt); // Should not crash

    free_ast_node(unknown_stmt);
    return 0;
}

// Main test runner
int main(int argc, char **argv) {
    int total = 0, passed = 0, failed = 0;

    printf("Running LLVM Statement Generation Module Tests...\n\n");

    // Setup test environment
    if (setup_test_environment() != 0) {
        printf("FAILED: Could not setup test environment\n");
        return 1;
    }

    // Run tests
    RUN_TEST(test_null_input_handling);
    RUN_TEST(test_return_statement_generation);
    RUN_TEST(test_void_return_statement);
    RUN_TEST(test_variable_declaration);
    RUN_TEST(test_expression_statement);
    RUN_TEST(test_block_statement);
    RUN_TEST(test_if_statement_basic);
    RUN_TEST(test_unimplemented_statements);
    RUN_TEST(test_unknown_statement_type);

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