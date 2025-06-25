/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Statement Analysis Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core semantic analysis functions for statements
 */

#ifndef ASTHRA_SEMANTIC_STATEMENT_ANALYSIS_H
#define ASTHRA_SEMANTIC_STATEMENT_ANALYSIS_H

#include "../parser/ast.h"
#include "semantic_core.h"
#include <stdbool.h>

/**
 * Analyze any type of statement
 * @param analyzer The semantic analyzer context
 * @param stmt The statement AST node to analyze
 * @return true if analysis succeeded, false on error
 */
bool semantic_analyze_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

#endif // ASTHRA_SEMANTIC_STATEMENT_ANALYSIS_H