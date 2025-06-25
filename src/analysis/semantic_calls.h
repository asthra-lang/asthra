#ifndef SEMANTIC_CALLS_H
#define SEMANTIC_CALLS_H

#include "../parser/ast_types.h"
#include "semantic_analyzer_core.h"

// Call expression analysis functions
bool analyze_call_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

// Helper functions for argument validation
bool validate_function_arguments(SemanticAnalyzer *analyzer, ASTNode *call_expr,
                                 SymbolEntry *func_symbol);
bool validate_method_arguments(SemanticAnalyzer *analyzer, ASTNode *call_expr,
                               SymbolEntry *method_symbol);

#endif // SEMANTIC_CALLS_H