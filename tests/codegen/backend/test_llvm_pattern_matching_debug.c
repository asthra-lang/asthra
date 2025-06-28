/**
 * Debug version of pattern matching tests to isolate the issue
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
    data->f32_type = LLVMFloatTypeInContext(data->context);
    data->f64_type = LLVMDoubleTypeInContext(data->context);
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

// Test match statement without arms first
static bool test_match_without_arms() {
    printf("Testing match statement without arms...\n");

    LLVMBackendData *data = create_test_backend();
    if (!data) {
        printf("Failed to create backend\n");
        return false;
    }

    // Create a simple match statement node with null arms
    ASTNode *match_stmt = calloc(1, sizeof(ASTNode));
    match_stmt->type = AST_MATCH_STMT;
    match_stmt->data.match_stmt.expression = NULL; // This should trigger error
    match_stmt->data.match_stmt.arms = NULL;
    match_stmt->location = (SourceLocation){.filename = NULL, .line = 1, .column = 1, .offset = 0};

    printf("Calling generate_statement with match statement...\n");
    generate_statement(data, match_stmt);

    // Check for errors
    bool has_error = llvm_backend_has_errors(data);
    printf("Has errors: %s\n", has_error ? "YES" : "NO");

    if (has_error) {
        printf("Errors reported (as expected):\n");
        llvm_backend_print_errors(data);
    }

    // Cleanup
    free(match_stmt);
    if (data->builder)
        LLVMDisposeBuilder(data->builder);
    if (data->module)
        LLVMDisposeModule(data->module);
    if (data->context)
        LLVMContextDispose(data->context);
    free(data);

    return has_error; // Should have error for null expression
}

int main() {
    printf("Running LLVM Pattern Matching Debug Tests...\n\n");

    bool result = test_match_without_arms();

    printf("\nDebug test result: %s\n", result ? "PASS" : "FAIL");

    return result ? 0 : 1;
}