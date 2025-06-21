#ifndef SEMANTIC_CONTROL_FLOW_H
#define SEMANTIC_CONTROL_FLOW_H

#include "semantic_analyzer_core.h"
#include "../parser/ast_types.h"

// Control flow statement analysis functions
bool analyze_return_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_if_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_if_let_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_match_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Check if a block statement returns Never type
 * Returns true if the block definitely never returns (contains Never-returning statement)
 */
bool block_returns_never(SemanticAnalyzer *analyzer, ASTNode *block);

#endif // SEMANTIC_CONTROL_FLOW_H