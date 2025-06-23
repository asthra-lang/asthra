/**
 * Asthra Programming Language Compiler
 * Const Expression Evaluator - Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 2: Semantic Analysis Implementation
 * Interface for compile-time evaluation of constant expressions
 */

#ifndef ASTHRA_CONST_EVALUATOR_H
#define ASTHRA_CONST_EVALUATOR_H

#include "semantic_analyzer_core.h"
#include "../parser/ast.h"
#include "../parser/ast_types.h"
#include "../codegen/global_symbols.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONST VALUE TYPES
// =============================================================================

// ConstValueType is defined in ../codegen/global_symbols.h

typedef struct ConstValue {
    ConstValueType type;
    union {
        int64_t integer_value;
        double float_value;
        char *string_value;
        bool boolean_value;
    } data;
} ConstValue;

// =============================================================================
// CONST EXPRESSION TYPES (imported from ast_types.h)
// =============================================================================

// Note: ConstExprType is defined in ast_types.h to avoid duplication

// =============================================================================
// CONST VALUE CREATION AND DESTRUCTION
// =============================================================================

/**
 * Create a new integer const value
 */
ConstValue *const_value_create_integer(int64_t value);

/**
 * Create a new float const value
 */
ConstValue *const_value_create_float(double value);

/**
 * Create a new string const value
 */
ConstValue *const_value_create_string(const char *value);

/**
 * Create a new boolean const value
 */
ConstValue *const_value_create_boolean(bool value);

/**
 * Destroy a const value and free its memory
 */
void const_value_destroy(ConstValue *value);

// =============================================================================
// CONST EXPRESSION EVALUATION
// =============================================================================

/**
 * Evaluate a const expression at compile time
 * @param analyzer The semantic analyzer instance
 * @param const_expr The const expression AST node to evaluate
 * @return The evaluated const value, or NULL if evaluation failed
 */
ConstValue *evaluate_const_expression(SemanticAnalyzer *analyzer, ASTNode *const_expr);

/**
 * Evaluate a literal node as a constant value
 * @param analyzer The semantic analyzer instance
 * @param literal The literal AST node (e.g., AST_INTEGER_LITERAL) to evaluate
 * @return The evaluated const value, or NULL if evaluation failed
 */
ConstValue *evaluate_literal_as_const(SemanticAnalyzer *analyzer, ASTNode *literal);

/**
 * Check for dependency cycles in const declarations
 * @param analyzer The semantic analyzer instance
 * @param const_name The name of the const being declared
 * @param const_expr The const expression to check for cycles
 * @return true if no cycles detected, false if cycle found
 */
bool check_const_dependency_cycle(SemanticAnalyzer *analyzer, const char *const_name, ASTNode *const_expr);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONST_EVALUATOR_H
