#ifndef SEMANTIC_EXPRESSION_UTILS_H
#define SEMANTIC_EXPRESSION_UTILS_H

#include "semantic_analyzer_core.h"
#include "../parser/ast_types.h"

// Struct and enum expression analysis functions
bool analyze_struct_instantiation(SemanticAnalyzer *analyzer, ASTNode *expr);
bool analyze_enum_variant(SemanticAnalyzer *analyzer, ASTNode *expr);

// Expression utility functions
bool semantic_is_lvalue_expression(SemanticAnalyzer *analyzer, ASTNode *expr);
bool semantic_is_constant_expression(SemanticAnalyzer *analyzer, ASTNode *expr);
bool semantic_has_side_effects(SemanticAnalyzer *analyzer, ASTNode *expr);

#endif // SEMANTIC_EXPRESSION_UTILS_H