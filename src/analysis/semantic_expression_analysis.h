/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Expression Analysis Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core semantic analysis functions for expressions
 */

#ifndef ASTHRA_SEMANTIC_EXPRESSION_ANALYSIS_H
#define ASTHRA_SEMANTIC_EXPRESSION_ANALYSIS_H

#include "../parser/ast.h"
#include "semantic_core.h"
#include <stdbool.h>

/**
 * Analyze any type of expression
 * @param analyzer The semantic analyzer context
 * @param expr The expression AST node to analyze
 * @return true if analysis succeeded, false on error
 */
bool semantic_analyze_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

#endif // ASTHRA_SEMANTIC_EXPRESSION_ANALYSIS_H