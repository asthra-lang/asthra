/**
 * Common definitions and utilities for enum variant code generation tests
 * Part of Phase 4: Integration and Testing (Enum Variant Construction Implementation Plan)
 */

#ifndef TEST_ENUM_VARIANT_COMMON_H
#define TEST_ENUM_VARIANT_COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the necessary headers for code generation testing
#include "ast.h"
// Backend interface removed - using LLVM directly
#include "../../framework/backend_stubs.h"
#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_core.h"

// Test helper structure to track both backend and analyzer
typedef struct {
    void *backend; // Backend abstraction removed
    SemanticAnalyzer *analyzer;
} TestCodeGenContext;

// Test helper function declarations
Lexer *create_test_lexer(const char *source);
ASTNode *parse_enum_test_expression(const char *source);
TestCodeGenContext *create_test_code_gen_context(void);
void destroy_test_code_gen_context(TestCodeGenContext *ctx);
void analyze_expression_for_test(TestCodeGenContext *ctx, ASTNode *expr);

// Functions from codegen_test_stubs.c
char *test_code_generate_expression(void *backend, ASTNode *expr);
char *test_code_generate_enum_variant_construction(void *backend, const char *enum_name,
                                                   const char *variant_name, ASTNode *payload);

#endif // TEST_ENUM_VARIANT_COMMON_H