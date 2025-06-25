#ifndef SEMANTIC_ARRAYS_H
#define SEMANTIC_ARRAYS_H

#include "../parser/ast_types.h"
#include "semantic_analyzer_core.h"

// Array, slice, and index access analysis functions
bool analyze_array_literal(SemanticAnalyzer *analyzer, ASTNode *expr);
bool analyze_slice_expression(SemanticAnalyzer *analyzer, ASTNode *expr);
bool analyze_index_access(SemanticAnalyzer *analyzer, ASTNode *expr);

#endif // SEMANTIC_ARRAYS_H