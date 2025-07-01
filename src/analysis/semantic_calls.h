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

// Predeclared function validation
bool validate_len_function_call(SemanticAnalyzer *analyzer, ASTNode *call_expr);
bool validate_range_function_call(SemanticAnalyzer *analyzer, ASTNode *call_expr);

// Associated function call analysis
bool analyze_associated_function_call(SemanticAnalyzer *analyzer, ASTNode *expr);

#endif // SEMANTIC_CALLS_H