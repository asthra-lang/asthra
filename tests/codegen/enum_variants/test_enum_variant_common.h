/**
 * Common definitions and utilities for enum variant code generation tests
 * Part of Phase 4: Integration and Testing (Enum Variant Construction Implementation Plan)
 */

#ifndef TEST_ENUM_VARIANT_COMMON_H
#define TEST_ENUM_VARIANT_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// Include the necessary headers for code generation testing
#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include "semantic_core.h"
#include "code_generator.h"
#include "code_generator_core.h"
#include "code_generator_types.h"
#include "codegen_test_stubs.h"

// Test helper structure to track both generator and analyzer
typedef struct {
    CodeGenerator *generator;
    SemanticAnalyzer *analyzer;
} TestCodeGenContext;

// Test helper function declarations
Lexer *create_test_lexer(const char *source);
ASTNode *parse_enum_test_expression(const char *source);
TestCodeGenContext *create_test_code_gen_context(void);
void destroy_test_code_gen_context(TestCodeGenContext *ctx);
void analyze_expression_for_test(TestCodeGenContext *ctx, ASTNode *expr);

#endif // TEST_ENUM_VARIANT_COMMON_H