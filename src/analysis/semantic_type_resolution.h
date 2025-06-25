#ifndef SEMANTIC_TYPE_RESOLUTION_H
#define SEMANTIC_TYPE_RESOLUTION_H

#include "../parser/ast_types.h"
#include "semantic_types.h"

// Forward declaration
typedef struct SemanticAnalyzer SemanticAnalyzer;

// Type resolution and analysis functions
TypeDescriptor *analyze_type_node(SemanticAnalyzer *analyzer, ASTNode *type_node);

#endif // SEMANTIC_TYPE_RESOLUTION_H