#ifndef SEMANTIC_LOOPS_H
#define SEMANTIC_LOOPS_H

#include "semantic_analyzer_core.h"
#include "../parser/ast_types.h"

// Loop statement analysis functions
bool analyze_for_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_while_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

// Loop control flow statement analysis functions
bool analyze_break_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_continue_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

#endif // SEMANTIC_LOOPS_H