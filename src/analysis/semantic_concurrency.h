#ifndef SEMANTIC_CONCURRENCY_H
#define SEMANTIC_CONCURRENCY_H

#include "../parser/ast_types.h"
#include "semantic_analyzer_core.h"

// Concurrency statement analysis functions
bool analyze_spawn_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_spawn_with_handle_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_await_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

// Concurrency expression analysis functions
bool analyze_await_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

#endif // SEMANTIC_CONCURRENCY_H