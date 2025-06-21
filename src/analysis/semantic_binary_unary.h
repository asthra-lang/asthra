#ifndef SEMANTIC_BINARY_UNARY_H
#define SEMANTIC_BINARY_UNARY_H

#include "semantic_analyzer_core.h"
#include "../parser/ast_types.h"

// Binary and unary expression analysis functions
bool analyze_binary_expression(SemanticAnalyzer *analyzer, ASTNode *expr);
bool analyze_unary_expression(SemanticAnalyzer *analyzer, ASTNode *expr);
bool analyze_assignment_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

#endif // SEMANTIC_BINARY_UNARY_H