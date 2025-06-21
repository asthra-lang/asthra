#ifndef SEMANTIC_VARIABLES_H
#define SEMANTIC_VARIABLES_H

#include "semantic_analyzer_core.h"
#include "../parser/ast_types.h"

// Variable declaration and assignment analysis functions
bool analyze_let_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_assignment_validation(SemanticAnalyzer *analyzer, ASTNode *assignment);

#endif // SEMANTIC_VARIABLES_H