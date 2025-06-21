#ifndef SEMANTIC_BASIC_STATEMENTS_H
#define SEMANTIC_BASIC_STATEMENTS_H

#include "semantic_analyzer_core.h"
#include "../parser/ast_types.h"

// Basic statement analysis functions
bool analyze_block_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_expression_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

#endif // SEMANTIC_BASIC_STATEMENTS_H