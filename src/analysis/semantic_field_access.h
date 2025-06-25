#ifndef SEMANTIC_FIELD_ACCESS_H
#define SEMANTIC_FIELD_ACCESS_H

#include "../parser/ast_types.h"
#include "semantic_analyzer_core.h"

// Field and module access analysis functions
bool analyze_field_access(SemanticAnalyzer *analyzer, ASTNode *node);
ASTNode *analyze_regular_field_access(SemanticAnalyzer *analyzer, ASTNode *node);
ASTNode *analyze_module_access(SemanticAnalyzer *analyzer, ASTNode *node);

#endif // SEMANTIC_FIELD_ACCESS_H