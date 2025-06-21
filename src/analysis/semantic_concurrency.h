#ifndef SEMANTIC_CONCURRENCY_H
#define SEMANTIC_CONCURRENCY_H

#include "semantic_analyzer_core.h"
#include "../parser/ast_types.h"

// Concurrency statement analysis functions
bool analyze_spawn_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_spawn_with_handle_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool analyze_await_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

#endif // SEMANTIC_CONCURRENCY_H