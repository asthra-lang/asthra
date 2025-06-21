#ifndef TYPE_INFERENCE_H
#define TYPE_INFERENCE_H

#include "semantic_symbols.h"
#include "semantic_types.h"
#include "../parser/ast_types.h"

// Type inference function declarations
TypeDescriptor *semantic_get_expression_type(SemanticAnalyzer *analyzer, ASTNode *expr);

#endif // TYPE_INFERENCE_H